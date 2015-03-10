#ifndef DCDSUPPORT_H
#define DCDSUPPORT_H

#include "codemodel/dmodel.h"

#include <QObject>
#include <QStringList>
#include <QProcess>
#include <QMap>
#include <QVector>
#include <QSharedPointer>

#include <functional>

namespace Dcd {

const char DCD_CODEMODEL_ID[] = "DCD Code model";

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

class Client : public QObject, public DCodeModel::IModel
{
    Q_OBJECT
public:

    Client(int port = -1);

    void setPort(int port);
    int port() const;

    virtual ~Client();
    virtual DCodeModel::ModelId id() const Q_DECL_OVERRIDE;
    virtual Client* copy() const Q_DECL_OVERRIDE;
    void complete(const QString &source, int position, DCodeModel::CompletionList &result) Q_DECL_OVERRIDE;
    void appendIncludePaths(const QStringList &includePaths);
    void getDocumentationComments(const QString &sources, int position, QStringList &result) Q_DECL_OVERRIDE;
    void findSymbolLocation(const QString &sources, int position, DCodeModel::Symbol &result) Q_DECL_OVERRIDE;
    void getSymbolsByName(const QString &sources, const QString &name, DCodeModel::SymbolList &result) Q_DECL_OVERRIDE;
    void getCurrentDocumentSymbols(const QString &sources, DCodeModel::SymbolList &result) Q_DECL_OVERRIDE;
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
    ~Factory();
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

DCodeModel::SymbolType fromString(QChar c);

} // namespace Dcd

#endif // DCDSUPPORT_H
