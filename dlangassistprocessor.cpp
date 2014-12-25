#include "dlangassistprocessor.h"

#include "dlangcompletionassistprovider.h"
#include "dlangoptionspage.h"
#include "dcdsupport.h"

#include <texteditor/codeassist/assistinterface.h>
#include <texteditor/codeassist/assistproposalitem.h>
#include <texteditor/codeassist/genericproposal.h>
#include <texteditor/codeassist/keywordscompletionassist.h>
#include <texteditor/codeassist/functionhintproposal.h>
#include <coreplugin/messagemanager.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>

#include <QTextDocument>
#include <QIcon>

using namespace DlangEditor;

namespace {
class CompletionIconMap
{
public:
    CompletionIconMap() {
        mapping.resize(Dcd::DcdCompletion::DCD_IDENTIFIER_TYPE_SIZE);
        mapping[Dcd::DcdCompletion::DCD_ALIAS] = QIcon(QLatin1String(":/dlangeditor/images/alias.png"));
        mapping[Dcd::DcdCompletion::DCD_ARRAY] = QIcon(QLatin1String(":/dlangeditor/images/array.png"));
        mapping[Dcd::DcdCompletion::DCD_ASSOC_ARRAY] = QIcon(QLatin1String(":/dlangeditor/images/assoc_array.png"));
        mapping[Dcd::DcdCompletion::DCD_CLASS] = QIcon(QLatin1String(":/codemodel/images/class.png"));
        mapping[Dcd::DcdCompletion::DCD_ENUM_NAME] = QIcon(QLatin1String(":/codemodel/images/enum.png"));
        mapping[Dcd::DcdCompletion::DCD_ENUM_VAR] = QIcon(QLatin1String(":/codemodel/images/enumerator.png"));
        mapping[Dcd::DcdCompletion::DCD_FUNCTION] = QIcon(QLatin1String(":/codemodel/images/func.png"));
        mapping[Dcd::DcdCompletion::DCD_INTERFACE] = QIcon(QLatin1String(":/dlangeditor/images/interface.png"));
        mapping[Dcd::DcdCompletion::DCD_KEYWORD] = QIcon(QLatin1String(":/codemodel/images/keyword.png"));
        mapping[Dcd::DcdCompletion::DCD_MEMBER_VAR] = QIcon(QLatin1String(":/dlangeditor/images/member_var.png"));
        mapping[Dcd::DcdCompletion::DCD_MIXIN] = QIcon(QLatin1String(":/dlangeditor/images/mixin.png"));
        mapping[Dcd::DcdCompletion::DCD_MODULE] = QIcon(QLatin1String(":/codemodel/images/namespace.png"));
        mapping[Dcd::DcdCompletion::DCD_PACKAGE] = QIcon(QLatin1String(":/core/images/dir.png"));
        mapping[Dcd::DcdCompletion::DCD_STRUCT] = QIcon(QLatin1String(":/dlangeditor/images/struct.png"));
        mapping[Dcd::DcdCompletion::DCD_TEMPLATE] = QIcon(QLatin1String(":/dlangeditor/images/template.png"));
        mapping[Dcd::DcdCompletion::DCD_UNION] = QIcon(QLatin1String(":/dlangeditor/images/union.png"));
        mapping[Dcd::DcdCompletion::DCD_VAR] = QIcon(QLatin1String(":/codemodel/images/var.png"));
    }

    const QIcon& fromType(Dcd::DcdCompletion::IdentifierType type) const {
        return mapping.at(type);
    }

private:
    QVector<QIcon> mapping;
};

static CompletionIconMap staticIcons;
}



QChar characterAt(const TextEditor::AssistInterface *interface, int offset = 0)
{
    return interface->characterAt(interface->position() + offset - 1);
}

bool findFunctionArgumentsBegin(const TextEditor::AssistInterface *interface, int &offset)
{
    QChar c = characterAt(interface);
    int off;
    for (off = 1; !c.isNull() && c != QLatin1Char('(') && c != QLatin1Char(')'); ++off) {
        c = characterAt(interface, -off);
    }
    if (c == QLatin1Char('(')) {
        offset = -off;
        return true;
    }
    return false;
}

int findWordBegin(const TextEditor::AssistInterface *interface)
{
    int pos = interface->position();
    QChar c;
    do {
        c = interface->characterAt(--pos);
    } while (!c.isNull() && (c.isLetterOrNumber() ||  c == QLatin1Char('_')));
    return pos + 1;
}

DlangAssistProcessor::DlangAssistProcessor()
    : m_client(0)
{

}

DlangAssistProcessor::~DlangAssistProcessor()
{

}

TextEditor::IAssistProposal *DlangAssistProcessor::perform(const TextEditor::AssistInterface *interface)
{
    m_interface.reset(static_cast<const TextEditor::AssistInterface *>(interface));
    m_proposalOffset = 0;

    if (interface->reason() != TextEditor::ExplicitlyInvoked && !accepts())
        return 0;

    return proposal();
}

bool DlangAssistProcessor::accepts()
{
    QChar currentChar = characterAt(m_interface.data());
    switch (DlangCompletionAssistProvider::completionTypeOfChar(currentChar)) {
    case DlangCompletionAssistProvider::DLANG_DOT:
        return true;
    case DlangCompletionAssistProvider::DLANG_COMMA:
        return findFunctionArgumentsBegin(m_interface.data(), m_proposalOffset);
    case DlangCompletionAssistProvider::DLANG_FUNCTION:
        return true;
    default:
        return false;
    }
}

TextEditor::IAssistProposal *createAssistProposal(const Dcd::DcdClient::CompletionList& list, int pos)
{
    using namespace TextEditor;
    switch (list.type) {
    case Dcd::DCD_IDENTIFIER:
    {
        QList<TextEditor::AssistProposalItem *> items;
        foreach (const Dcd::DcdCompletion& comp, list.list) {
            AssistProposalItem *item = new AssistProposalItem;
            item->setText(comp.data);
            item->setIcon(staticIcons.fromType(comp.type));
            items.append(item);
        }
        return new GenericProposal(pos, items);
    }
        break;
    case Dcd::DCD_CALLTIP:
    {
        QStringList functionSymbols;
        foreach (const Dcd::DcdCompletion& comp, list.list) {
            functionSymbols.append(comp.data);
        }
        IFunctionHintProposalModel *model =
                new KeywordsFunctionHintModel(functionSymbols);
        return new FunctionHintProposal(pos, model);
    }
        break;
    case Dcd::DCD_BAD_TYPE:
        return 0;
    default:
        return 0;
    }
}

TextEditor::IAssistProposal *DlangAssistProcessor::proposal()
{
    try {
        if (!m_client) {
            QString projectName = ProjectExplorer::ProjectExplorerPlugin::currentProject() ?
                        ProjectExplorer::ProjectExplorerPlugin::currentProject()->displayName() : QString();
            m_client = Dcd::DcdFactory::instance()->client(projectName);
            if (!m_client) {
                return 0;
            }
        }
        Dcd::DcdClient::CompletionList list;
        m_client->completeFromArray(m_interface->textDocument()->toPlainText(), m_interface->position(), list);
        int wordPosition = findWordBegin(m_interface.data());
        return createAssistProposal(list, wordPosition);
    }
    catch (std::exception& ex) {
        qWarning("DlangAssistProcessor::proposal:%s", ex.what());
        m_client.reset();
    }
    return 0;
}
