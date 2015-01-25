#ifndef DCDSUPPORT_H
#define DCDSUPPORT_H

#include <QObject>
#include <QStringList>
#include <QProcess>
#include <QMap>
#include <QSharedPointer>

QT_FORWARD_DECLARE_CLASS(QTextStream)

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

class DcdClient : public QObject
{
    Q_OBJECT
public:
    struct CompletionList
    {
        CompletionType type;
        QList<DcdCompletion> list;
    };

    DcdClient(const QString &projectName, const QString &processName, int port, QObject *parent = 0);

    const QString &projectName() const;

    void setOutputFile(const QString &filePath);

    /**
     * @brief Complete by position in the file
     * @param filePath
     * @param position
     * @param[out] result result of completion (may be empty, of course)
     * @return false on error (errorString() may contain error description)
     */
    void complete(const QString &filePath, int position, CompletionList &result);

    /**
     * @brief Complete by position in byte array passed to dcd-client by input channel
     * @param array
     * @param position
     * @param result result of completion (may be empty, of course)
     * @return false on error (error(QString) signal may contain error description)
     */
    void completeFromArray(const QString &array, int position, CompletionList &result);

    /**
     * @brief Send request to dcd-server to add include path
     * @param includePath
     * @return false on error (error(QString) signal may contain error description)
     */
    void appendIncludePath(const QString &includePath);

    struct Location {
        QString filename;
        int position;
        Location() {}
        Location(const QString& s, int line) : filename(s), position(line) {}
        bool isNull() const {
            return filename.isNull() || filename.isEmpty();
        }
    };

    /**
     * @brief Finds symbol location
     * @param array
     * @param position
     * @param result pair of file path and symbol definition line
     * @return
     */
    void findSymbolLocation(const QString &array, int position, Location& result);

    /**
     * @brief Gets documentation comments
     * @param array
     * @param position
     * @param result string list of documentation comments
     * @return
     */
    void getDocumentationComments(const QString &array, int position, QStringList &result);

    typedef QList<QPair<Location, DcdCompletion::IdentifierType> > DcdSymbolList;

    /**
     * @brief Gets symbols by name
     * @param array
     * @param position
     * @param result string list of documentation comments
     * @return
     */
    void getSymbolsByName(const QString &array, const QString &name, DcdSymbolList &result);

    int port() const {
        return m_port;
    }

signals:
public slots:
private:
    void parseOutput(const QByteArray &output, CompletionList &result);
    void parseIdentifiers(QTextStream &stream, CompletionList &result);
    void parseCalltips(QTextStream &stream, CompletionList &result);
    void parseSymbols(const QByteArray &output, DcdSymbolList &result);

    QString m_projectName;
    int m_port;
    QString m_processName;
    QStringList m_portArguments;
    QString m_filePath;
};

namespace Internal {
class ClientPrivate;
}

class Client : public QObject
{
    Q_OBJECT
public:
    Client(int port);
    void complete(const QString &source, int position, DcdClient::CompletionList &result);
private:
    Internal::ClientPrivate* d;
};

class DcdServer : public QObject
{
    Q_OBJECT
public:
    DcdServer(const QString &projectName, const QString &processName, int port, QObject *parent = 0);
    virtual ~DcdServer();
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

/**
 * @brief The factory that creates the "connected" pair of server-client
 * Creates the pair lazily and once for each project name
 */
class DcdFactory : public QObject
{
    Q_OBJECT
public:

    typedef QSharedPointer<Dcd::DcdClient> ClientPointer;

    /**
     * @brief Creates the pair of server-client with the first free port in portRange() and starts the server.
     *  On the next calling with the same project name if server is running factory won't recreate anything
     *  and will return previous client
     * @param projectName
     * @return client
     */
    ClientPointer client(const QString &projectName);

    void setPortRange(int first, int last);
    QPair<int, int> portRange() const;
    static DcdFactory *instance();
private slots:
    void onError(QString error);
private:
    DcdFactory(QPair<int, int> range);
    virtual ~DcdFactory() {}
    void appendIncludePaths(ClientPointer client);

    typedef QSharedPointer<Dcd::DcdServer> ServerPointer;
    typedef QPair<ClientPointer, ServerPointer> ClientServer;
    typedef QMap<QString, ClientServer> MapString;

    MapString mapChannels;
    int currentPortOffset;
    int m_firstPort;
    int m_lastPort;
};


QPair<int, int> findSymbol(const QString& text, int pos);

} // namespace Dcd

#endif // DCDSUPPORT_H
