#include "codemodel/dastedmodel.h"
#include "codemodel/dastedmessages.h"

#include "dlangdebughelper.h"
#include "dlangeditorutils.h"

#include <QTcpSocket>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>

#include <msgpack.hpp>

using namespace Dasted;

// Converting utils
DCodeModel::Symbol conv(const Dasted::Symbol &sym)
{
    DCodeModel::Symbol res;
    res.name = QString::fromStdString(sym.name.impl);
    res.location.filename = QString::fromStdString(sym.location.filename.impl);
    res.location.position = sym.location.cursor;
    res.type = fromChar(sym.type);
    res.subType = subTypefromChar(sym.subType);
    res.typeName = QString::fromStdString(sym.typeName.impl);
    for (const auto& p : sym.templateParameters.impl) {
        res.templateParameters += QString::fromStdString(p.impl) + QLatin1String(", ");
    }
    res.templateParameters.remove(res.templateParameters.length() - 2, 2);
    return res;
}

DastedServer::DastedServer(const QString &processName, int port, QObject *parent)
    : DlangEditor::Utils::ServerDaemon(parent, processName), m_port(port)
{
    setArguments(QStringList(arguments()) << "-p" << QString::number(port)
                 << "-d");
}

DastedServer::~DastedServer()
{

}

int DastedServer::port() const
{
    return m_port;
}

#define ENFORCE(op, exc) if (!(op)) throw std::runtime_error(exc)

// ClientPrivate
namespace Dasted {
namespace Internal {
class ClientPrivate
{
public:
    ClientPrivate(int port);
    void setPort(int port);
    int port() const;

    void complete(const QString &projectName, const DCodeModel::Sources &sources,
                  int position, DCodeModel::CompletionList &result);
    void appendIncludePaths(const QString &projectName,
                            const QStringList &includePaths);
    void getDocumentationComments(const QString &projectName,
                                  const DCodeModel::Sources &sources,
                                  int position, QStringList &result);
    void findSymbolLocation(const QString &projectName,
                            const DCodeModel::Sources &sources,
                            int position, DCodeModel::Symbol &result);
    void getSymbolsByName(const QString &projectName,
                          const DCodeModel::Sources &sources,
                          const QString &name, DCodeModel::SymbolList &result);
    void getCurrentDocumentSymbols(const QString &projectName,
                                   const DCodeModel::Sources &sources,
                                   DCodeModel::Scope &result);

    template <MessageType T>
    void reqRep(const Request<T> &req, Reply<T> &rep, int timeout = 1000) {
        ConnectionGuard g(this);
        send(req);
        recv(rep, timeout);
    }

    template <MessageType T>
    void send(const Request<T> &req) {
        m_buff.clear();
        msgpack::pack(&m_buff, req);
        quint32 s = static_cast<quint32>(m_buff.size());
        quint8 type = static_cast<quint8>(T);
        s += sizeof(type) + sizeof(PROTOCOL_VERSION);
        ENFORCE(m_tcp.write(reinterpret_cast<const char*>(&s), sizeof(s)) == sizeof(s),
                "failed to write message size");
        ENFORCE(m_tcp.write(reinterpret_cast<const char*>(&PROTOCOL_VERSION), sizeof(PROTOCOL_VERSION)) == sizeof(PROTOCOL_VERSION),
                "failed to write protocol version");
        ENFORCE(m_tcp.write(reinterpret_cast<const char*>(&type), sizeof(type)) == sizeof(type),
                "failed to write message type");
        ENFORCE(m_tcp.write(m_buff.data(), m_buff.size()) == static_cast<qint64>(m_buff.size()),
                "failed to write message");
        ENFORCE(m_tcp.waitForBytesWritten(1000),
                "unfinished socket write");
    }

    template <MessageType T>
    void recv(Reply<T> &rep, int timeout = 1000) {
        ENFORCE(m_tcp.waitForDisconnected(timeout), "server operation timeout");
        QByteArray arr = m_tcp.readAll();
        quint32 s = *reinterpret_cast<const quint32*>(arr.data());
        ENFORCE(s == arr.size() - sizeof(s), "message length mismatched");
        ENFORCE(s > sizeof(quint8) + sizeof(quint8), "message is too small");
        quint8 v = *reinterpret_cast<const quint8*>(arr.data() + sizeof(s));
        ENFORCE(v == PROTOCOL_VERSION, "protocol version mismatched");
        quint8 type = *reinterpret_cast<const quint8*>(arr.data() + sizeof(s) + sizeof(v));
        ENFORCE(type == T, "message type mismatched");
        const char *pkg = arr.data() + sizeof(s) + sizeof(v) + sizeof(type);
        const size_t pkgLength = s - sizeof(v) - sizeof(type);
        msgpack::unpack(&m_unp, pkg, pkgLength);
        msgpack::object obj = m_unp.get();
        rep = obj.as<Reply<T> >();
        m_unp.zone()->clear();
    }

private:
    class ConnectionGuard
    {
    public:
        ConnectionGuard(ClientPrivate *parent);
        ~ConnectionGuard();

    private:
        ClientPrivate *m_parent;
    };

    msgpack::sbuffer m_buff;
    msgpack::unpacked m_unp;
    int m_port;
    QTcpSocket m_tcp;
};
} // namespace Internal
} // namespace Dasted


Internal::ClientPrivate::ConnectionGuard::ConnectionGuard(ClientPrivate *parent)
    : m_parent(parent)
{
    ENFORCE(m_parent->m_port > 0, "uninitialized port");
    m_parent->m_tcp.connectToHost("localhost", m_parent->m_port);
    ENFORCE(m_parent->m_tcp.waitForConnected(100),
            "failed to connect to dasted with port = " + std::to_string(m_parent->m_port));
}

Internal::ClientPrivate::ConnectionGuard::~ConnectionGuard()
{
    m_parent->m_tcp.close();
}

Internal::ClientPrivate::ClientPrivate(int port)
    : m_port(port)
{

}

void Internal::ClientPrivate::setPort(int port)
{
    m_port = port;
}

int Internal::ClientPrivate::port() const
{
    return m_port;
}

void Internal::ClientPrivate::complete(const QString &projectName,
                                       const DCodeModel::Sources &sources,
                                       int position,
                                       DCodeModel::CompletionList &result)
{
    DEBUG_GUARD("complete: " + projectName + " " + QString::number(position));
    result.type = DCodeModel::COMPLETION_BAD_TYPE;
    result.list.clear();
    Request<COMPLETE> req;
    req.project.impl = projectName.toStdString();
    req.src.text.impl = sources.txt.toStdString();
    req.src.revision = sources.revision;
    req.src.filename.impl = sources.filename.toStdString();
    req.cursor = position;
    Reply<COMPLETE> rep;
    reqRep(req, rep);
    for (auto &s: rep.symbols.impl) {
        DCodeModel::Symbol symbol;
        symbol.name = QString::fromStdString(s.name.impl);
        symbol.type = fromChar(s.type);
        for (auto &p: s.parameters.impl) {
            symbol.parameters.append(QString::fromStdString(p.impl) + ", ");
        }
        symbol.parameters.chop(2);
        result.list.push_back(symbol);
    }
    result.type = rep.calltips ? DCodeModel::COMPLETION_CALLTIP
                               : DCodeModel::COMPLETION_IDENTIFIER;
}

void Internal::ClientPrivate::appendIncludePaths(
        const QString &projectName, const QStringList &includePaths)
{
    DEBUG_GUARD("add import paths: " + projectName + " " +
                includePaths.join(','));
    Request<ADD_IMPORT_PATHS> req;
    req.project.impl = projectName.toStdString();
    foreach (auto &p, includePaths) {
        DString s;
        s.impl = p.toStdString();
        req.paths.impl.push_back(s);
    }
    Reply<ADD_IMPORT_PATHS> rep;
    reqRep(req, rep, 10000);
}

void Internal::ClientPrivate::getDocumentationComments(
        const QString &projectName, const DCodeModel::Sources &sources, int position,
        QStringList &result)
{
    result.clear();
    Request<GET_DOC> req;
    req.project.impl = projectName.toStdString();
    req.src.text.impl = sources.txt.toStdString();
    req.src.revision = sources.revision;
    req.src.filename.impl = sources.filename.toStdString();
    req.cursor = position;
    Reply<GET_DOC> rep;
    reqRep(req, rep);
    for (auto &s: rep.symbols.impl) {
        result.push_back(QString::fromStdString(s.doc.impl));
    }
}

void Internal::ClientPrivate::findSymbolLocation(const QString &projectName,
                                                 const DCodeModel::Sources &sources,
                                                 int position,
                                                 DCodeModel::Symbol &result)
{
    DEBUG_GUARD("findSymbolLocation: " + projectName + " " +
                QString::number(position));
    Request<FIND_DECLARATION> req;
    req.project.impl = projectName.toStdString();
    req.src.text.impl = sources.txt.toStdString();
    req.src.revision = sources.revision;
    req.src.filename.impl = sources.filename.toStdString();
    req.cursor = position;
    Reply<FIND_DECLARATION> rep;
    reqRep(req, rep);
    result = DCodeModel::Symbol();
    result.name = QString::fromStdString(rep.symbol.name.impl);
    result.location.filename = QString::fromStdString(rep.symbol.location.filename.impl);
    result.location.position = rep.symbol.location.cursor;
    result.type = fromChar(rep.symbol.type);
}

void Internal::ClientPrivate::getSymbolsByName(const QString &projectName,
                                               const DCodeModel::Sources &sources,
                                               const QString &name,
                                               DCodeModel::SymbolList &result)
{
    Q_UNUSED(projectName)
    Q_UNUSED(sources)
    Q_UNUSED(name)
    Q_UNUSED(result)
    throw std::runtime_error("not implemented yet");
}

static void convertScope(const Scope& s, DCodeModel::Scope& result)
{
    result.symbol = conv(s.symbol);
    for (const auto& scope : s.children.impl) {
        DCodeModel::Scope resScope;
        convertScope(scope, resScope);
        result.children.push_back(resScope);
    }
}

void Internal::ClientPrivate::getCurrentDocumentSymbols(
        const QString &projectName, const DCodeModel::Sources &sources,
        DCodeModel::Scope &result)
{
    result = DCodeModel::Scope();
    Request<OUTLINE> req;
    req.project.impl = projectName.toStdString();
    req.src.text.impl = sources.txt.toStdString();
    req.src.revision = sources.revision;
    req.src.filename.impl = sources.filename.toStdString();
    Reply<OUTLINE> rep;
    reqRep(req, rep);

    convertScope(rep.global, result);
    result.fixParents();
}

DastedModel::DastedModel(int port, bool autoStartServer,
                         const QString &processName)
    : d(new Internal::ClientPrivate(port))
{
    if (autoStartServer) {
        startServer(processName, port);
    }
}

void DastedModel::setPort(int port)
{
    return d->setPort(port);
}

int DastedModel::port() const
{
    return d->port();
}

DastedModel::~DastedModel()
{
    delete d;
}

DCodeModel::ModelId DastedModel::id() const
{
    return DASTED_CODEMODEL_ID;
}

#define CATCHALL(op) try {\
        return op; \
    } \
    catch (const std::exception& ex) { \
        qWarning() << "Dasted client: " << ex.what(); \
    } \
    catch (...) { \
        qWarning() << "Dasted client: unknown exception"; \
    }


void DastedModel::complete(const QString &projectName,
                           const DCodeModel::Sources &sources,
                           int position,
                           DCodeModel::CompletionList &result)
{
    CATCHALL(d->complete(projectName, sources, position, result))
}

void DastedModel::appendIncludePaths(const QString &projectName,
                                     const QStringList &includePaths)
{
    CATCHALL(d->appendIncludePaths(projectName, includePaths))
}

void DastedModel::getDocumentationComments(const QString &projectName,
                                           const DCodeModel::Sources &sources,
                                           int position,
                                           QStringList &result)
{
    CATCHALL(d->getDocumentationComments(projectName, sources, position, result))
}

void DastedModel::findSymbolLocation(const QString &projectName,
                                     const DCodeModel::Sources &sources,
                                     int position,
                                     DCodeModel::Symbol &result)
{
    CATCHALL(d->findSymbolLocation(projectName, sources, position, result))
}

void DastedModel::getSymbolsByName(const QString &projectName,
                                   const DCodeModel::Sources &sources,
                                   const QString &name,
                                   DCodeModel::SymbolList &result)
{
    CATCHALL(d->getSymbolsByName(projectName, sources, name, result))
}

void DastedModel::getCurrentDocumentSymbols(const QString &projectName,
                                            const DCodeModel::Sources &sources,
                                            DCodeModel::Scope &result)
{
    CATCHALL(d->getCurrentDocumentSymbols(projectName, sources, result))
}

void DastedModel::onImportPathsUpdate(QString projectName, QStringList imports)
{
    CATCHALL(appendIncludePaths(projectName, imports))
}

void DastedModel::onServerError(QString error)
{
    qWarning() << "Dasted server error: " << error;
    m_server->stop();
    QString processName = m_server->processName();
    int port = m_server->port();
    startServer(processName, port);
}

void DastedModel::startServer(const QString &processName, int port)
{
    try {
        m_server.create(processName, port);
        m_server->start();
        connect(m_server.data(), SIGNAL(error(QString)),
                this, SLOT(onServerError(QString)));
    }
    catch (const std::exception& ex) {
        qWarning() << "Dasted server: " << ex.what();
    }
    catch (...) {
        qWarning() << "Dasted server: unknown exception";
    }
}

DCodeModel::SymbolType Dasted::fromChar(unsigned char c)
{
    using namespace DCodeModel;
    switch (c) {
    case CLASS: return SYMBOL_CLASS;
    case INTERFACE: return SYMBOL_INTERFACE;
    case STRUCT: return SYMBOL_STRUCT;
    case UNION: return SYMBOL_UNION;
    case VARIABLE: return SYMBOL_VAR;
    case MEMBER: return SYMBOL_MEMBER_VAR;
    case KEYWORD: return SYMBOL_KEYWORD;
    case FUNCTION: return SYMBOL_FUNCTION;
    case ENUM: return SYMBOL_ENUM_NAME;
    case ENUM_VARIABLE: return SYMBOL_ENUM_VAR;
    case PACKAGE: return SYMBOL_PACKAGE;
    case MODULE: return SYMBOL_MODULE;
    case ARRAY: return SYMBOL_ARRAY;
    case ASSOCIATIVE_ARRAY: return SYMBOL_ASSOC_ARRAY;
    case ALIAS: return SYMBOL_ALIAS;
    case TEMPLATE: return SYMBOL_TEMPLATE;
    case MIXIN_TEMPLATE: return SYMBOL_MIXIN;
    case BLOCK: return SYMBOL_BLOCK;
    default: return SYMBOL_NO_TYPE;
    }
    return SYMBOL_NO_TYPE;
}

DCodeModel::SymbolSubType Dasted::subTypefromChar(unsigned char c)
{
    using namespace DCodeModel;
    switch (c) {
    case SUBTYPE_IN: return SYMBOL_IN;
    case SUBTYPE_OUT: return SYMBOL_OUT;
    case SUBTYPE_SCOPE: return SYMBOL_SCOPE;
    case SUBTYPE_UNITTEST: return SYMBOL_UNITTEST;
    default: return SYMBOL_NO_SUB_TYPE;
    }
    return SYMBOL_NO_SUB_TYPE;
}
