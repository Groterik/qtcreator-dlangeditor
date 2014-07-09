#include "dlangassistprocessor.h"

#include "dlangcompletionassistprovider.h"
#include "dlangoptionspage.h"
#include "dcdsupport.h"

#include <texteditor/codeassist/iassistinterface.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>

using namespace DlangEditor;

QChar characterAt(const TextEditor::IAssistInterface* interface, int offset = 0)
{
    return interface->characterAt(interface->position() + offset - 1);
}

bool findFunctionArgumentsBegin(const TextEditor::IAssistInterface* interface, int &offset)
{
    QChar c = characterAt(interface);
    Q_ASSERT(c.isSpace());
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

TextEditor::IAssistProposal *createAssistProposal(const Dcd::DcdClient::CompletionList& list)
{
    return 0;
}

TextEditor::IAssistProposal *DlangAssistProcessor::proposals()
{
    if (!m_client) {
        QString projectName = ProjectExplorer::ProjectExplorerPlugin::currentProject() ? ProjectExplorer::ProjectExplorerPlugin::currentProject()->displayName() : QString();
        m_client = DcdFactory::instance()->createClient(projectName);
        m_client->appendIncludePath(DlangOptionsPage::phobosDir());
    }
    Dcd::DcdClient::CompletionList list;
    m_client->complete(m_interface->fileName(), m_interface->position(), list);
    return createAssistProposal(list);
}




Dcd::DcdClient *DcdFactory::createClient(const QString &projectName)
{
    MapStringInt::iterator it = mapChannels.find(projectName);
    if (it == mapChannels.end()) {
        int port =  m_firstPort + currentPortOffset % (m_lastPort - m_firstPort + 1);
        Dcd::DcdServer *server = new Dcd::DcdServer(DlangOptionsPage::dcdServerExecutable(), port, this);
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
