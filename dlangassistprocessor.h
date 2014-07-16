#ifndef DLANGASSISTPROCESSOR_H
#define DLANGASSISTPROCESSOR_H

#include <QMap>

#include <texteditor/codeassist/iassistprocessor.h>

namespace Dcd {
class DcdClient;
class DcdServer;
}

namespace DlangEditor {

class DlangAssistProcessor : public TextEditor::IAssistProcessor
{
public:
    DlangAssistProcessor();
    virtual ~DlangAssistProcessor();

    // pure TextEditor::IAssistProcessor

    /**
     * @brief The method is called everytime when completion is needed
     * @param interface has usefull information about document and position of completion
     * @return proposal for completion
     */
    virtual TextEditor::IAssistProposal *perform(const TextEditor::IAssistInterface *interface);
private:
    bool accepts();
    TextEditor::IAssistProposal *proposal();
    TextEditor::IAssistProposal *completeAt();

    QScopedPointer<const TextEditor::IAssistInterface> m_interface;
    Dcd::DcdClient *m_client;
    int m_proposalOffset;
};

/**
 * @brief The factory that creates the "connected" pair of server-client
 * Creates the pair lazily and once for each project name
 */
class DcdFactory : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Creates the pair of server-client with the first free port in portRange() and starts the server.
     *  On the next calling with the same project name if server is running factory won't recreate anything
     *  and will return previous client
     * @param projectName
     * @return client
     */
    Dcd::DcdClient *client(const QString &projectName);
    void appendIncludePaths(Dcd::DcdClient *client);
    void setPortRange(int first, int last);
    QPair<int, int> portRange() const;
    static DcdFactory *instance();
private slots:
    void onError(QString error);
private:
    DcdFactory(QPair<int, int> range);
    virtual ~DcdFactory() {}
    typedef QMap<QString, QPair<Dcd::DcdClient*, Dcd::DcdServer*> > MapString;
    typedef QMap<int, QString> MapPort;
    MapString mapChannels;
    MapPort mapPorts;
    int currentPortOffset;
    int m_firstPort;
    int m_lastPort;
};

} // namespace DlangEditor

#endif // DLANGASSISTPROCESSOR_H
