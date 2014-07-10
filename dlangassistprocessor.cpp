#include "dlangassistprocessor.h"

#include "dlangcompletionassistprovider.h"
#include "dlangoptionspage.h"
#include "dcdsupport.h"

#include <texteditor/codeassist/iassistinterface.h>
#include <texteditor/codeassist/genericproposal.h>
#include <texteditor/codeassist/basicproposalitemlistmodel.h>
#include <texteditor/codeassist/keywordscompletionassist.h>
#include <coreplugin/messagemanager.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>

#include <QTextDocument>

using namespace DlangEditor;

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

    return proposals();
}

bool DlangAssistProcessor::accepts()
{
    QChar currentChar = characterAt(m_interface.data());
    switch (DlangCompletionAssistProvider::completionTypeOfChar(currentChar)) {
    case DlangCompletionAssistProvider::DLANG_DOT:
        return true;
    case DlangCompletionAssistProvider::DLANG_COMMA:
        return findFunctionArgumentsBegin(m_interface.data(), m_proposalOffset);
    default:
        return false;
    }
}

TextEditor::IAssistProposal *createAssistProposal(const Dcd::DcdClient::CompletionList& list, int pos)
{
    using namespace TextEditor;
    QList<TextEditor::BasicProposalItem *> items;
    foreach (const Dcd::DcdCompletion& comp, list) {
        BasicProposalItem *item = new BasicProposalItem;
        item->setText(comp.data);
        items.append(item);
    }
    return new GenericProposal(pos, new BasicProposalItemListModel(items));
}

TextEditor::IAssistProposal *DlangAssistProcessor::proposals()
{
    if (!m_client) {
        QString projectName = ProjectExplorer::ProjectExplorerPlugin::currentProject() ? ProjectExplorer::ProjectExplorerPlugin::currentProject()->displayName() : QString();
        m_client = DcdFactory::instance()->createClient(projectName);
        if (!m_client) {
            return 0;
        }
        if (!m_client->appendIncludePath(DlangOptionsPage::phobosDir())) {
            Core::MessageManager::write(m_client->errorString(), Core::MessageManager::Flash);
            return 0;
        }
    }
    Dcd::DcdClient::CompletionList list;
    if (!m_client->completeFromArray(m_interface->textDocument()->toPlainText(), m_interface->position(), list)) {
        Core::MessageManager::write(m_client->errorString(), Core::MessageManager::Flash);
        return 0;
    }
    int wordPosition = findWordBegin(m_interface.data());
    return createAssistProposal(list, wordPosition);
}




Dcd::DcdClient *DcdFactory::createClient(const QString &projectName)
{
    MapStringInt::iterator it = mapChannels.find(projectName);
    if (it == mapChannels.end()) {
        int port =  m_firstPort + currentPortOffset % (m_lastPort - m_firstPort + 1);
        Dcd::DcdServer *server = new Dcd::DcdServer(DlangOptionsPage::dcdServerExecutable(), port, this);
        if (!server->start()) {
            Core::MessageManager::write(QLatin1String("Failed to start DCD server"), Core::MessageManager::Flash);
            return 0;
        }
        connect(server, SIGNAL(error(QString)), this, SLOT(onError(QString)));
        it = mapChannels.insert(projectName, port);
        mapPorts[port] = projectName;
        ++currentPortOffset;
    }
    return new Dcd::DcdClient(DlangOptionsPage::dcdClientExecutable(), it.value(), this);
}

void DcdFactory::setPortRange(int first, int last)
{
    m_firstPort = first;
    m_lastPort = std::max(last, first);
}

DcdFactory *DcdFactory::instance()
{
    static DcdFactory inst(DlangOptionsPage::portsRange());
    return &inst;
}

void DcdFactory::onError(QString)
{
    Dcd::DcdServer *server = qobject_cast<Dcd::DcdServer*>(sender());
    QString projectName = mapPorts[server->port()];
    mapPorts.remove(server->port());
    mapChannels.remove(projectName);
    server->stop();
}

DcdFactory::DcdFactory(QPair<int, int> range)
    : currentPortOffset(0)
{
    setPortRange(range.first, range.second);
}
