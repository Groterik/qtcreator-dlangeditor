#ifndef DASTEDMODEL_H
#define DASTEDMODEL_H

#include "codemodel/dmodel.h"
#include "serverprocess.h"

#include <QObject>
#include <QStringList>
#include <QProcess>
#include <QMap>
#include <QVector>
#include <QSharedPointer>

#include <functional>

namespace Dasted {

const char DASTED_CODEMODEL_ID[] = "Dasted Code model";

class Server : public DlangEditor::Utils::ServerDaemon
{
    Q_OBJECT
public:
    Server(const QString &processName, int port, QObject *parent = 0);
    virtual ~Server();
    int port() const;
public slots:
    void onImportPathsUpdate(QString projectName, QStringList imports);
private:
    int m_port;
    QStringList m_importPaths;
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
    void getCurrentDocumentSymbols(const QString &sources, DCodeModel::Scope &result) Q_DECL_OVERRIDE;
private:
    Internal::ClientPrivate* d;
};

class Factory : public QObject
{
    Q_OBJECT
public:
    typedef std::function<QString()> NameGetter;
    typedef std::function<void(QSharedPointer<Server>)> ServerInitializer;

    DCodeModel::IModelSharedPtr createClient(bool serverAutoStart);

    void setPort(int r);
    int port() const;

    void setProcessName(const QString& p);

    void setServerLog(const QString& l);

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
    QSharedPointer<Server> createServer(int port, bool start = true);
    QSharedPointer<Server> m_server;
    QString m_serverProcessName;
    QString m_serverLog;
    NameGetter m_nameGetter;
    ServerInitializer m_serverInitializer;
    bool finished;
    int m_port;
};

DCodeModel::SymbolType fromChar(unsigned char c);

} // namespace Dasted

#endif // DASTEDMODEL_H
