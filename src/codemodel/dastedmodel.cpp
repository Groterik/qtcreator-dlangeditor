#include "codemodel/dastedmodel.h"
#include "codemodel/dastedmessages.h"

#include <QTcpSocket>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>

#include <msgpack.hpp>

using namespace Dasted;

Server::Server(const QString &processName, int port, QObject *parent)
    : DlangEditor::Utils::ServerDaemon(parent, processName), m_port(port)
{
    setArguments(QStringList(arguments()) << "-p" << QString::number(port));
}

Server::~Server()
{

}

int Server::port() const
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

    void complete(const QString &source, int position, DCodeModel::CompletionList &result);
    void appendIncludePaths(const QStringList &includePaths);
    void getDocumentationComments(const QString &sources, int position, QStringList &result);
    void findSymbolLocation(const QString &sources, int position, DCodeModel::Symbol &result);
    void getSymbolsByName(const QString &sources, const QString &name, DCodeModel::SymbolList &result);
    void getCurrentDocumentSymbols(const QString &sources, DCodeModel::SymbolList &result);

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
        ENFORCE(m_tcp.write(m_buff.data(), m_buff.size()) == static_cast<quint64>(m_buff.size()),
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

void Internal::ClientPrivate::complete(const QString &source, int position, DCodeModel::CompletionList &result)
{
    result.type = DCodeModel::COMPLETION_BAD_TYPE;
    result.list.clear();
    Request<COMPLETE> req;
    req.src.impl = source.toStdString();
    req.cursor = position;
    Reply<COMPLETE> rep;
    reqRep(req, rep);
    for (auto &s: rep.symbols.impl) {
        DCodeModel::Symbol symbol;
        symbol.data = QString::fromStdString(s.name.impl);
        symbol.type = fromChar(s.type);
        result.list.push_back(symbol);
    }
    result.type = rep.calltips ? DCodeModel::COMPLETION_CALLTIP : DCodeModel::COMPLETION_IDENTIFIER;
}

void Internal::ClientPrivate::appendIncludePaths(const QStringList &includePaths)
{
    Request<ADD_IMPORT_PATHS> req;
    foreach (auto &p, includePaths) {
        DString s;
        s.impl = p.toStdString();
        req.paths.impl.push_back(s);
    }
    Reply<ADD_IMPORT_PATHS> rep;
    reqRep(req, rep, 5000);
}

void Internal::ClientPrivate::getDocumentationComments(const QString &sources, int position, QStringList &result)
{
    result.clear();
    Request<GET_DOC> req;
    req.src.impl = sources.toStdString();
    req.cursor = position;
    Reply<GET_DOC> rep;
    reqRep(req, rep);
    for (auto &s: rep.symbols.impl) {
        result.push_back(QString::fromStdString(s.doc.impl));
    }
}

void Internal::ClientPrivate::findSymbolLocation(const QString &sources, int position, DCodeModel::Symbol &result)
{
    Request<FIND_DECLARATION> req;
    req.src.impl = sources.toStdString();
    req.cursor = position;
    Reply<FIND_DECLARATION> rep;
    reqRep(req, rep);
    result = DCodeModel::Symbol();
    result.data = QString::fromStdString(rep.symbol.name.impl);
    result.location.filename = QString::fromStdString(rep.symbol.location.filename.impl);
    result.location.position = rep.symbol.location.cursor;
    result.type = fromChar(rep.symbol.type);
}

void Internal::ClientPrivate::getSymbolsByName(const QString &sources, const QString &name, DCodeModel::SymbolList &result)
{
    Q_UNUSED(sources)
    Q_UNUSED(name)
    Q_UNUSED(result)
    throw std::runtime_error("not implemented yet");
}

void Internal::ClientPrivate::getCurrentDocumentSymbols(const QString &sources, DCodeModel::SymbolList &result)
{
    Q_UNUSED(sources)
    Q_UNUSED(result)
    throw std::runtime_error("not implemented yet");
}

Client::Client(int port)
    : d(new Internal::ClientPrivate(port))
{

}

void Client::setPort(int port)
{
    return d->setPort(port);
}

int Client::port() const
{
    return d->port();
}

Client::~Client()
{
    delete d;
}

DCodeModel::ModelId Client::id() const
{
    return DASTED_CODEMODEL_ID;
}

Client *Client::copy() const
{
    return new Client(port());
}

void Client::complete(const QString &source, int position, DCodeModel::CompletionList &result)
{
    return d->complete(source, position, result);
}

void Client::appendIncludePaths(const QStringList &includePaths)
{
    return d->appendIncludePaths(includePaths);
}

void Client::getDocumentationComments(const QString &sources, int position, QStringList &result)
{
    return d->getDocumentationComments(sources, position, result);
}

void Client::findSymbolLocation(const QString &sources, int position, DCodeModel::Symbol &result)
{
    return d->findSymbolLocation(sources, position, result);
}

void Client::getSymbolsByName(const QString &sources, const QString &name, DCodeModel::SymbolList &result)
{
    return d->getSymbolsByName(sources, name, result);
}

void Client::getCurrentDocumentSymbols(const QString &sources, DCodeModel::SymbolList &result)
{
    return d->getCurrentDocumentSymbols(sources, result);
}

QMutex serverMutex;

DCodeModel::IModelSharedPtr Factory::createClient(bool serverAutoStart)
{
    try {
        if (!m_server) {
            QMutexLocker lock(&serverMutex);
            if (!m_server) {
                m_server = createServer(m_port, serverAutoStart);
            }
        }
    } catch (...) {
        return DCodeModel::IModelSharedPtr();
    }
    return DCodeModel::IModelSharedPtr(new Client(m_port));
}

void Factory::setPort(int r)
{
    m_port = r;
}

int Factory::port() const
{
    return m_port;
}

void Factory::setProcessName(const QString &p)
{
    m_serverProcessName = p;
}

void Factory::setServerLog(const QString &l)
{
    m_serverLog = l;
}

void Factory::restore(int port, int ts)
{
    Q_UNUSED(port)
    Q_UNUSED(ts)
    throw std::runtime_error("not implemented yet");
}

void Factory::setNameGetter(Factory::NameGetter c)
{
    m_nameGetter = c;
}

void Factory::setServerInitializer(Factory::ServerInitializer i)
{
    m_serverInitializer = i;
}

Factory &Factory::instance()
{
    static Factory inst;
    return inst;
}

void Factory::onError(QString error)
{
    qWarning("DcdFactory::onError: %s", error.toStdString().data());
    Server *server = qobject_cast<Server*>(sender());
    server->stop();
}

void Factory::onServerFinished()
{
    Server *server = qobject_cast<Server*>(sender());
    if (server) {
        emit serverFinished(m_port);
        finished = true;
    }
}

Factory::Factory()
    : finished(false)
{

}

Factory::~Factory()
{

}

QSharedPointer<Server> Factory::createServer(int port, bool start)
{
    auto server = QSharedPointer<Server>(new Server(m_serverProcessName, port, this));

    if (start) {
        server->start();
    }

    QTimer *timer = new QTimer;
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, [=]() {
        if (m_serverInitializer) {
            m_serverInitializer(server);
        }
    });
    timer->start(1000);

    return server;
}

DCodeModel::SymbolType Dasted::fromChar(unsigned char c)
{
    using namespace DCodeModel;
    switch (c) {
    case 'c': return SYMBOL_CLASS;
    case 'i': return SYMBOL_INTERFACE;
    case 's': return SYMBOL_STRUCT;
    case 'u': return SYMBOL_UNION;
    case 'v': return SYMBOL_VAR;
    case 'm': return SYMBOL_MEMBER_VAR;
    case 'k': return SYMBOL_KEYWORD;
    case 'f': return SYMBOL_FUNCTION;
    case 'g': return SYMBOL_ENUM_NAME;
    case 'e': return SYMBOL_ENUM_VAR;
    case 'P': return SYMBOL_PACKAGE;
    case 'M': return SYMBOL_MODULE;
    case 'a': return SYMBOL_ARRAY;
    case 'A': return SYMBOL_ASSOC_ARRAY;
    case 'l': return SYMBOL_ALIAS;
    case 't': return SYMBOL_TEMPLATE;
    case 'T': return SYMBOL_MIXIN;
    default: return SYMBOL_NO_TYPE;
    }
    return SYMBOL_NO_TYPE;
}
