#ifndef DCDSUPPORT_H
#define DCDSUPPORT_H

#include <QObject>
#include <QStringList>
#include <QProcess>
#include <QMap>
#include <QVector>
#include <QSharedPointer>

#include <functional>

namespace Dcd {

enum CompletionType {
    DCD_BAD_TYPE, DCD_IDENTIFIER, DCD_CALLTIP,
    DCD_COMPLETION_TYPE_SIZE
};

/**
 * @brief The DcdCompletion struct returned from client as result of completion
 */
struct DcdCompletion
{
    enum IdentifierType {
        DCD_NO_TYPE, DCD_ENUM_VAR, DCD_VAR, DCD_CLASS, DCD_INTERFACE,
        DCD_STRUCT, DCD_UNION, DCD_MEMBER_VAR, DCD_KEYWORD, DCD_FUNCTION,
        DCD_ENUM_NAME, DCD_PACKAGE, DCD_MODULE, DCD_ARRAY, DCD_ASSOC_ARRAY,
        DCD_ALIAS, DCD_TEMPLATE, DCD_MIXIN,
        DCD_IDENTIFIER_TYPE_SIZE
    };

    IdentifierType type;
    QString data;

    static IdentifierType fromString(QChar c);
};

class Server : public QObject
{
    Q_OBJECT
public:
    Server(const QString &projectName, const QString &processName, int port, QObject *parent = 0);
    virtual ~Server();
    int port() const;

    const QString& projectName() const;

    void setOutputFile(const QString& filePath);

    void start();
    void stop();
    bool isRunning() const;
signals:
    /**
     * @brief The signal is emitted when an error occurs with the dcd-server or
     * dcd-server exits
     */
    void error(QString);
    void finished();
private slots:
    void onFinished(int errorCode);
    void onError(QProcess::ProcessError error);
private:
    QString m_projectName;
    int m_port;
    QString m_processName;
    QProcess *m_process;
    QString m_filePath;
};

namespace Internal {
class ClientPrivate;
}

class Client : public QObject
{
    Q_OBJECT
public:
    struct Location {
        QString filename;
        int position;
        Location() {}
        Location(const QString& s, int line) : filename(s), position(line) {}
        bool isNull() const {
            return filename.isNull() || filename.isEmpty();
        }
    };

    struct CompletionList
    {
        CompletionType type;
        QList<DcdCompletion> list;
    };

    typedef QList<QPair<Location, DcdCompletion::IdentifierType> > SymbolList;

    Client(int port = -1);

    void setPort(int port);
    int port() const;

    virtual ~Client();
    /**
     * @brief Complete by position in the file
     * @param source
     * @param position
     * @param[out] result result of completion (may be empty, of course)
     * @return throws on error
     */
    void complete(const QString &source, int position, CompletionList &result);
    /**
     * @brief Send request to dcd-server to add include path
     * @param includePath
     * @return throws on error
     */
    void appendIncludePaths(const QStringList &includePaths);
    /**
     * @brief Gets documentation comments
     * @param sources
     * @param position
     * @param[out] result string list of documentation comments
     * @return throws on error
     */
    void getDocumentationComments(const QString &sources, int position, QStringList &result);
    /**
     * @brief Gets symbols by name
     * @param sources
     * @param position
     * @param[out] result string list of documentation comments
     * @return throws on error
     */
    void findSymbolLocation(const QString &sources, int position, Client::Location &result);

    /**
     * @brief Gets symbols by name
     * @param sources
     * @param name
     * @param[out] result string list of documentation comments
     * @return throws on error
     */
    void getSymbolsByName(const QString &sources, const QString &name, SymbolList &result);
private:
    Internal::ClientPrivate* d;
};

class Factory : public QObject
{
    Q_OBJECT
public:
    typedef std::function<QString()> NameGetter;
    typedef std::function<void(QSharedPointer<Server>)> ServerInitializer;

    void setPortRange(QPair<int, int> r);

    void setProcessName(const QString& p);

    void setServerLog(const QString& l);

    /**
     * @brief Gets port number for client (starts dcd-server instance if needed)
     * @return port
     */
    int getPort();

    /**
     * @brief Restores dcd-server
     * @param port
     * @param ts timestamp
     */
    void restore(int port, int ts = 0);

    void setNameGetter(NameGetter c);

    void setServerInitializer(ServerInitializer i);

    static Factory &instance();

signals:
    void serverFinished(int port);

private slots:
    void onError(QString error);
    void onServerFinished();

private:
    Factory();
    QSharedPointer<Server> createServer(const QString& name, int port);
    QString m_serverProcessName;
    QString m_serverLog;
    NameGetter m_nameGetter;
    ServerInitializer m_serverInitializer;
    QPair<int, int> m_portRange;
    int m_currentPort;
    QMap<int, QSharedPointer<Server> > m_byPort;
    QMap<QString, QSharedPointer<Server> > m_byName;
    QVector<QSharedPointer<Server> > m_forDeletion;
};

QPair<int, int> findSymbol(const QString& text, int pos);

} // namespace Dcd

#endif // DCDSUPPORT_H
