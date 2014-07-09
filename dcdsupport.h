#ifndef DCDSUPPORT_H
#define DCDSUPPORT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>

QT_FORWARD_DECLARE_CLASS(QProcess)
QT_FORWARD_DECLARE_CLASS(QTextStream)

namespace Dcd {

struct DcdCompletion
{
    enum CompletionType {
        DCD_IDENTIFIER, DCD_CALLTIP,
        DCD_COMPLETION_TYPE_SIZE
    };

    enum IdentifierType {
        DCD_NO_TYPE, DCD_ENUM_VAR, DCD_VAR, DCD_CLASS, DCD_INTERFACE,
        DCD_STRUCT, DCD_UNION, DCD_MEMBER_VAR, DCD_KEYWORD, DCD_FUNCTION,
        DCD_ENUM_NAME, DCD_PACKAGE, DCD_MODULE, DCD_ARRAY, DCD_ASSOC_ARRAY,
        DCD_ALIAS, DCD_TEMPLATE, DCD_MIXIN,
        DCD_IDENTIFIER_TYPE_SIZE
    };
    CompletionType type;
    IdentifierType identType;
    QString data;

    static IdentifierType fromString(const QString& name);
};

class DcdClient : public QObject
{
    Q_OBJECT
public:
    typedef QList<DcdCompletion> CompletionList;
    DcdClient(QString processName, int port, QObject *parent = 0);

    bool complete(const QString &filePath, int position, CompletionList &result);
    bool appendIncludePath(const QString &includePath);

    const QString &errorString();

signals:

public slots:
private:
    bool parseOutput(const QByteArray& output, CompletionList& result);
    bool parseIdentifiers(QTextStream &stream, CompletionList& result);
    bool parseCalltips(QTextStream& stream, CompletionList& result);

    int m_port;
    QString m_processName;
    QProcess *m_process;
    QStringList m_portArguments;
    QString m_errorString;
};

class DcdServer : public QObject
{
    Q_OBJECT
public:
    DcdServer(QString processName, int port, QObject *parent = 0);
    void setProgram(QString program);
    void setPort(int port);
    int port() const;

    bool start();
    void stop();
signals:
    void error(QString);
private slots:
    void onFinished(int errorCode);
private:
    int m_port;
    QString m_processName;
    QProcess *m_process;
};

} // namespace Dcd

#endif // DCDSUPPORT_H
