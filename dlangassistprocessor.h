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
    virtual TextEditor::IAssistProposal *perform(const TextEditor::IAssistInterface *interface);
    QScopedPointer<const TextEditor::IAssistInterface> m_interface;
private:
    bool accepts();
    TextEditor::IAssistProposal *proposal();
    TextEditor::IAssistProposal *completeAt();

    Dcd::DcdClient *m_client;
    int m_proposalOffset;
};

class DcdFactory : public QObject
{
    Q_OBJECT
public:
    Dcd::DcdClient *client(const QString &projectName);
    void setPortRange(int first, int last);
    static DcdFactory *instance();
private slots:
    void onError(QString);
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
