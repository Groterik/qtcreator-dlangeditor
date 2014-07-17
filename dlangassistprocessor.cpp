#include "dlangassistprocessor.h"

#include "dlangcompletionassistprovider.h"
#include "dlangoptionspage.h"
#include "dcdsupport.h"

#include <texteditor/codeassist/iassistinterface.h>
#include <texteditor/codeassist/genericproposal.h>
#include <texteditor/codeassist/basicproposalitemlistmodel.h>
#include <texteditor/codeassist/keywordscompletionassist.h>
#include <texteditor/codeassist/functionhintproposal.h>
#include <coreplugin/messagemanager.h>
#include <extensionsystem/pluginmanager.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>
#include <cpptools/cppmodelmanagerinterface.h>

#include <QTextDocument>
#include <QIcon>

using namespace DlangEditor;

namespace {
class CompletionIconMap
{
public:
    CompletionIconMap() {
        mapping.resize(Dcd::DcdCompletion::DCD_IDENTIFIER_TYPE_SIZE);
        mapping[Dcd::DcdCompletion::DCD_ALIAS] = QIcon(QLatin1String(":/dlang/alias.png"));
        mapping[Dcd::DcdCompletion::DCD_ARRAY] = QIcon(QLatin1String(":/dlang/array.png"));
        mapping[Dcd::DcdCompletion::DCD_ASSOC_ARRAY] = QIcon(QLatin1String(":/dlang/assoc_array.png"));
        mapping[Dcd::DcdCompletion::DCD_CLASS] = QIcon(QLatin1String(":/codemodel/images/class.png"));
        mapping[Dcd::DcdCompletion::DCD_ENUM_NAME] = QIcon(QLatin1String(":/codemodel/images/enum.png"));
        mapping[Dcd::DcdCompletion::DCD_ENUM_VAR] = QIcon(QLatin1String(":/codemodel/images/enumerator.png"));
        mapping[Dcd::DcdCompletion::DCD_FUNCTION] = QIcon(QLatin1String(":/codemodel/images/func.png"));
        mapping[Dcd::DcdCompletion::DCD_INTERFACE] = QIcon(QLatin1String(":/dlang/interface.png"));
        mapping[Dcd::DcdCompletion::DCD_KEYWORD] = QIcon(QLatin1String(":/kodemodel/keyword.png"));
        mapping[Dcd::DcdCompletion::DCD_MEMBER_VAR] = QIcon(QLatin1String(":/dlang/member_var.png"));
        mapping[Dcd::DcdCompletion::DCD_MIXIN] = QIcon(QLatin1String(":/dlang/mixin.png"));
        mapping[Dcd::DcdCompletion::DCD_MODULE] = QIcon(QLatin1String(":/codemodel/images/namespace.png"));
        mapping[Dcd::DcdCompletion::DCD_PACKAGE] = QIcon(QLatin1String(":/dlang/package.png"));
        mapping[Dcd::DcdCompletion::DCD_STRUCT] = QIcon(QLatin1String(":/dlang/struct.png"));
        mapping[Dcd::DcdCompletion::DCD_TEMPLATE] = QIcon(QLatin1String(":/dlang/template.png"));
        mapping[Dcd::DcdCompletion::DCD_UNION] = QIcon(QLatin1String(":/dlang/union.png"));
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



QChar characterAt(const TextEditor::IAssistInterface* interface, int offset = 0)
{
    return interface->characterAt(interface->position() + offset - 1);
}

bool findFunctionArgumentsBegin(const TextEditor::IAssistInterface* interface, int &offset)
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

int findWordBegin(const TextEditor::IAssistInterface* interface)
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

TextEditor::IAssistProposal *DlangAssistProcessor::perform(const TextEditor::IAssistInterface *interface)
{
    m_interface.reset(static_cast<const TextEditor::IAssistInterface *>(interface));
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
        QList<TextEditor::BasicProposalItem *> items;
        foreach (const Dcd::DcdCompletion& comp, list.list) {
            BasicProposalItem *item = new BasicProposalItem;
            item->setText(comp.data);
            item->setIcon(staticIcons.fromType(comp.type));
            items.append(item);
        }
        return new GenericProposal(pos, new BasicProposalItemListModel(items));
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
    default:
        return 0;
    }
}

TextEditor::IAssistProposal *DlangAssistProcessor::proposal()
{
    try {
        if (!m_client) {
            QString projectName = ProjectExplorer::ProjectExplorerPlugin::currentProject() ? ProjectExplorer::ProjectExplorerPlugin::currentProject()->displayName() : QString();
            m_client = DcdFactory::instance()->client(projectName);
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
        m_client = 0;
    }
    return 0;
}

Dcd::DcdClient *DcdFactory::client(const QString &projectName)
{
    MapString::iterator it = mapChannels.find(projectName);
    if (it == mapChannels.end()) {
        int port =  m_firstPort + currentPortOffset % (m_lastPort - m_firstPort + 1);
        QScopedPointer<Dcd::DcdServer> server(new Dcd::DcdServer(DlangOptionsPage::dcdServerExecutable(), port, this));
        QStringList pluginPaths = ExtensionSystem::PluginManager::pluginPaths();
        if (!pluginPaths.isEmpty()) {
            foreach (const QString& s, pluginPaths) {
                if (s.startsWith("/home/")) {
                    server->setOutputFile(s + "/server.log");
                    break;
                }
            }
        }
        server->start();
        connect(server.data(), SIGNAL(error(QString)), this, SLOT(onError(QString)));
        QScopedPointer<Dcd::DcdClient> client(new Dcd::DcdClient(DlangOptionsPage::dcdClientExecutable(), port, this));
        appendIncludePaths(client.data());

        it = mapChannels.insert(projectName, qMakePair(client.take(), server.take()));
        mapPorts[port] = projectName;
        ++currentPortOffset;
    } else {
        if (!it.value().second->isRunning()) {
            int port = it.value().second->port();
            it.value().second->stop();
            mapChannels.erase(it);
            mapPorts.remove(port);
            return 0;
        }
    }
    return it.value().first;
}

void DcdFactory::appendIncludePaths(Dcd::DcdClient *client)
{
    // append default include paths from options page
    QStringList list = DlangOptionsPage::includePaths();

    // append include paths from project settings
    CppTools::CppModelManagerInterface *modelmanager =
            CppTools::CppModelManagerInterface::instance();
    if (modelmanager) {
        ProjectExplorer::Project *currentProject = ProjectExplorer::ProjectExplorerPlugin::currentProject();
        if (currentProject) {
            CppTools::CppModelManagerInterface::ProjectInfo pinfo = modelmanager->projectInfo(currentProject);
            if (pinfo.isValid()) {
                list += pinfo.includePaths();
            }
        }
    }
    list.removeDuplicates();

    foreach (const QString& l, list) {
        client->appendIncludePath(l);
    }
}

void DcdFactory::setPortRange(int first, int last)
{
    m_firstPort = first;
    m_lastPort = std::max(last, first);
}

QPair<int, int> DcdFactory::portRange() const
{
    return qMakePair(m_firstPort, m_lastPort);
}

DcdFactory *DcdFactory::instance()
{
    static DcdFactory inst(DlangOptionsPage::portsRange());
    return &inst;
}

void DcdFactory::onError(QString error)
{
    qDebug("DcdFactory::onError: %s", error.toStdString().data());
    qWarning("DcdFactory::onError: %s", error.toStdString().data());
    Dcd::DcdServer *server = qobject_cast<Dcd::DcdServer*>(sender());
    QString projectName = mapPorts[server->port()];
    server->stop();
    mapPorts.remove(server->port());
    mapChannels.remove(projectName);
}

DcdFactory::DcdFactory(QPair<int, int> range)
    : currentPortOffset(0)
{
    setPortRange(range.first, range.second);
}
