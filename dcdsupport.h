#ifndef DCDSUPPORT_H
#define DCDSUPPORT_H

#include <QObject>
#include <QStringList>
#include <QProcess>

QT_FORWARD_DECLARE_CLASS(QTextStream)

namespace Dcd {

enum CompletionType {
    DCD_IDENTIFIER, DCD_CALLTIP,
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

    static IdentifierType fromString(const QString& name);
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

    DcdClient(QString processName, int port, QObject *parent = 0);

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

signals:
public slots:
private:
    void parseOutput(const QByteArray& output, CompletionList& result);
    void parseIdentifiers(QTextStream &stream, CompletionList& result);
    void parseCalltips(QTextStream& stream, CompletionList& result);

    int m_port;
    QString m_processName;
    QStringList m_portArguments;
};

class DcdServer : public QObject
{
    Q_OBJECT
public:
    DcdServer(QString processName, int port, QObject *parent = 0);
    virtual ~DcdServer();
    int port() const;

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
    int m_port;
    QString m_processName;
    QProcess *m_process;
};

} // namespace Dcd

#endif // DCDSUPPORT_H
