#include "dlangassistprocessor.h"

#include "dlangcompletionassistprovider.h"
#include "dlangoptionspage.h"
#include "codemodel/dmodel.h"
#include "dlangimagecache.h"

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

TextEditor::IAssistProposal *createAssistProposal(const DCodeModel::CompletionList &list, int pos)
{
    using namespace TextEditor;
    switch (list.type) {
    case DCodeModel::COMPLETION_IDENTIFIER:
    {
        QList<TextEditor::AssistProposalItem *> items;
        foreach (const auto& comp, list.list) {
            AssistProposalItem *item = new AssistProposalItem;
            item->setText(comp.data);
            item->setIcon(DlangIconCache::instance().fromType(comp.type));
            items.append(item);
        }
        return new GenericProposal(pos, items);
    }
        break;
    case DCodeModel::COMPLETION_CALLTIP:
    {
        QStringList functionSymbols;
        foreach (const auto& comp, list.list) {
            functionSymbols.append(comp.data);
        }
        IFunctionHintProposalModel *model =
                new KeywordsFunctionHintModel(functionSymbols);
        return new FunctionHintProposal(pos, model);
    }
        break;
    case DCodeModel::COMPLETION_BAD_TYPE:
        return 0;
    default:
        return 0;
    }
}

TextEditor::IAssistProposal *DlangAssistProcessor::proposal()
{
    try {
        DCodeModel::CompletionList list;
        list.type = DCodeModel::COMPLETION_BAD_TYPE;
        DCodeModel::IModelSharedPtr model = DCodeModel::Factory::instance().getModel();
        model->complete(m_interface->textDocument()->toPlainText(), m_interface->position(), list);
        int wordPosition = findWordBegin(m_interface.data());
        return createAssistProposal(list, wordPosition);
    }
    catch (std::exception& ex) {
        qWarning("DlangAssistProcessor::proposal:%s", ex.what());
    }
    return 0;
}
