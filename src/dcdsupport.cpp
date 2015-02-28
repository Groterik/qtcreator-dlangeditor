#include "dcdsupport.h"

#include "dlangdebughelper.h"

#include <stdexcept>
#include <bitset>
#include <sstream>

#include <QProcess>
#include <QTextStream>
#include <QDebug>
#include <QTcpSocket>
#include <QMutexLocker>
#include <QTime>
#include <QCoreApplication>

#include <msgpack.hpp>

#define NIY throw std::runtime_error("not implemented yet")

void delay(int millisecondsToWait)
{
    QTime dieTime = QTime::currentTime().addMSecs(millisecondsToWait);
    while (QTime::currentTime() < dieTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

using namespace Dcd;

namespace Dcd {
typedef unsigned char RequestKind;
typedef unsigned char ubyte;

/// Invalid completion kind. This is used internally and will never
/// be returned in a completion response.
const char dummy = '?';

/// Import symbol. This is used internally and will never
/// be returned in a completion response.
const char importSymbol = '*';

/// With symbol. This is used internally and will never
/// be returned in a completion response.
const char withSymbol = 'w';

/// class names
const char className = 'c';

/// interface names
const char interfaceName = 'i';

/// structure names
const char structName = 's';

/// union name
const char unionName = 'u';

/// variable name
const char variableName = 'v';

/// member variable
const char memberVariableName = 'm';

/// keyword, built-in version, scope statement
const char keyword = 'k';

/// function or method
const char functionName = 'f';

/// enum name
const char enumName = 'g';

/// enum member
const char enumMember = 'e';

/// package name
const char packageName = 'P';

/// module name
const char moduleName = 'M';

/// array
const char array = 'a';

/// associative array
const char assocArray = 'A';

/// alias name
const char aliasName = 'l';

/// template name
const char templateName = 't';

/// mixin template name
const char mixinTemplateName = 'T';


/**
* The completion list contains a listing of identifier/kind pairs.
*/
const std::string identifiers = "identifiers";

/**
* The auto-completion list consists of a listing of functions and their
* parameters.
*/
const std::string calltips = "calltips";

/**
* The response contains the location of a symbol declaration.
*/
const std::string location = "location";

/**
* The response contains documentation comments for the symbol.
*/
const std::string ddoc = "ddoc";


/**
 * Request kind
 */
enum RequestKindBits
{
        uninitialized = -1,
        /// Autocompletion
        autocomplete = 0,
        /// Clear the completion cache
        clearCache,
        /// Add import directory to server
        addImport,
        /// Shut down the server
        shutdown,
        /// Get declaration location of given symbol
        symbolLocation,
        /// Get the doc comments for the symbol
        doc,
        /// Query server status
        query,
        /// Search for symbol
        search,
};

typedef std::bitset<std::numeric_limits<RequestKind>::digits> RequestKindFlag;

/**
 * Autocompletion request message
 */
struct AutocompleteRequest
{
        /**
         * File name used for error reporting
         */
        std::string fileName;

        /**
         * Command coming from the client
         */
        RequestKind kind;

        /**
         * Paths to be searched for import files
         */
        std::vector<std::string> importPaths;

        /**
         * The source code to auto complete
         */
        std::string sourceCode;

        /**
         * The cursor position
         */
        size_t cursorPosition;

        /**
         * Name of symbol searched for
         */
        std::string searchName;

        AutocompleteRequest() : cursorPosition(0) {}

        MSGPACK_DEFINE(fileName, kind, importPaths, sourceCode, cursorPosition, searchName)
};

/**
 * Autocompletion response message
 */
struct AutocompleteResponse
{
        /**
         * The autocompletion type. (Parameters or identifier)
         */
        std::string completionType;

        /**
         * The path to the file that contains the symbol.
         */
        std::string symbolFilePath;

        /**
         * The byte offset at which the symbol is located.
         */
        size_t symbolLocation;

        /**
         * The documentation comment
         */
        std::vector<std::string> docComments;

        /**
         * The completions
         */
        std::vector<std::string> completions;

        /**
         * The kinds of the items in the completions array. Will be empty if the
         * completion type is a function argument list.
         */
        std::string completionKinds;

        /**
         * Symbol locations for symbol searches.
         */
        std::vector<size_t> locations;

        AutocompleteResponse() : symbolLocation(0) {}

        template <class ... Args>
        void unpackFields(const msgpack::object &o, Args& ... a) {
            static const size_t count = sizeof...(Args);
            if (o.via.array.size != count) {
                throw std::runtime_error("bad unserialized fields count");
            }
            unpackImpl<0>(o, a...);
        }

        template <int N, class T, class ... Tail>
        void unpackImpl(const msgpack::object &o, T& f, Tail& ... tail) {
            auto& field_obj = o.via.array.ptr[N];
            if (field_obj.type != msgpack::type::NIL) {
                field_obj >> f;
            }
            unpackImpl<N+1>(o, tail...);
        }

        template <int N>
        void unpackImpl(const msgpack::object &/*o*/) {
            return;
        }

        void msgpack_unpack(const msgpack::object &o)
        {
            if (o.type != msgpack::type::ARRAY) {
                throw msgpack::type_error();
            }
            unpackFields(o, completionType, symbolFilePath, symbolLocation, docComments, completions, completionKinds, locations);
        }
};

namespace Internal
{
class ClientPrivate
{
public:
    ClientPrivate(int m_port);

    void setPort(int m_port);
    int port() const;

    void complete(const QString &sources, int position, Client::CompletionList &result);
    void appendIncludePath(const QStringList &includePaths);
    void getDocumentationComments(const QString &sources, int position, QStringList &result);
    void findSymbolLocation(const QString &sources, int position, Client::Location &result);
    void getSymbolsByName(const QString &sources, const QString &name, Client::SymbolList &result);
    void getCurrentDocumentSymbols(const QString &sources, Client::SymbolList &result);

private:

    void send(const AutocompleteRequest &req);
    void recv(AutocompleteResponse &rep);

    void req(const AutocompleteRequest &req);
    void reqRep(const AutocompleteRequest &req, AutocompleteResponse &rep);

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
} // namespace Dcd::Internal
} // namespace Dcd

Client::Client(int port)
    : d(new Internal::ClientPrivate(port))
{

}

void Client::setPort(int port)
{
    d->setPort(port);
}

int Client::port() const
{
    return d->port();
}

Client::~Client()
{
    delete d;
}

void Client::complete(const QString &source, int position, CompletionList &result)
{
    return d->complete(source, position, result);
}

void Client::appendIncludePaths(const QStringList &includePaths)
{
    return d->appendIncludePath(includePaths);
}

void Client::getDocumentationComments(const QString &sources, int position, QStringList &result)
{
    return d->getDocumentationComments(sources, position, result);
}

void Client::findSymbolLocation(const QString &sources, int position, Client::Location &result)
{
    return d->findSymbolLocation(sources, position, result);
}

void Client::getSymbolsByName(const QString &sources, const QString &name, Client::SymbolList &result)
{
    return d->getSymbolsByName(sources, name, result);
}

void Client::getCurrentDocumentSymbols(const QString &sources, Client::SymbolList &result)
{
    return d->getCurrentDocumentSymbols(sources, result);
}

void Factory::setPortRange(QPair<int, int> r)
{
    m_portRange = qMakePair(r.first, std::max(r.first, r.second));
    m_currentPort = m_portRange.first;
}

void Factory::setProcessName(const QString &p)
{
    m_serverProcessName = p;
}

void Factory::setServerLog(const QString &l)
{
    m_serverLog = l;
}

QMutex createMutex;

int Factory::getPort() {
    QMutexLocker locker(&createMutex);
    m_forDeletion.clear();
    QString name = m_nameGetter ? m_nameGetter() : QLatin1String("default");
    auto it = m_byName.find(name);
    if (it == m_byName.end()) {
        // create new Server instance
        if (m_currentPort == m_portRange.second) {
            throw std::runtime_error("dcd-servers port range has been exceeded");
        }
        QSharedPointer<Server> server = createServer(name, m_currentPort++);
        return server->port();
    }
    return it.value()->port();
}

QMutex restoreMutex;

void Factory::restore(int port, int /*ts*/)
{
    QMutexLocker locker(&restoreMutex);
    m_forDeletion.clear();
    auto it = m_byPort.find(port);
    if (it == m_byPort.end()) {
        throw std::runtime_error("failed to restore dcd-server on " + std::to_string(port));
    }
    QString name = it.value()->projectName();
    it.value() = createServer(name, port);
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
    qDebug("DcdFactory::onError: %s", error.toStdString().data());
    qWarning("DcdFactory::onError: %s", error.toStdString().data());
    Dcd::Server *server = qobject_cast<Dcd::Server*>(sender());
    server->stop();
}

void Factory::onServerFinished()
{
    Dcd::Server *server = qobject_cast<Dcd::Server*>(sender());
    if (server) {
        DEBUG_GUARD(QLatin1String("port=") + QString::number(server->port()));
        auto name = server->projectName();
        auto port = server->port();
        QSharedPointer<Server> server;
        auto itN = m_byName.find(name);
        if (itN != m_byName.end()) {
            server = itN.value();
            m_byName.erase(itN);
        }
        auto itP = m_byPort.find(port);
        if (itP != m_byPort.end()) {
            server = itP.value();
            m_byPort.erase(itP);
        }
        if (server) {
            m_forDeletion.push_back(server);
        }
        emit serverFinished(port);
    }
}

Factory::Factory()
    : m_portRange(qMakePair(0, 0)), m_currentPort(0)
{

}

Factory::~Factory()
{
    DEBUG_GUARD(QLatin1String("map_size=") + QString::number(m_byName.size()));
    m_byName.clear();
    m_byPort.clear();
}

QSharedPointer<Server> Factory::createServer(const QString &name, int port)
{
    QSharedPointer<Server> server(new Server(name, m_serverProcessName, port));
    server->setOutputFile(m_serverLog);
    server->start();

    delay(1000);

    if (m_serverInitializer) {
        m_serverInitializer(server);
    }

    m_byName.insert(name, server);
    m_byPort.insert(port, server);

    connect(server.data(), SIGNAL(finished()), this, SLOT(onServerFinished()));

    return server;
}

void startProcess(QProcess &p, const QString &processName, const QStringList &args,
                  const QString &filePath, QIODevice::OpenMode mode = QIODevice::ReadWrite)
{
    DEBUG_GUARD(QLatin1String("process=") + processName);
    if (p.state() != QProcess::NotRunning) {
        throw std::runtime_error("process is already running");
    }
    if (!filePath.isEmpty()) {
        p.setStandardOutputFile(filePath+ QLatin1String(".out"), QIODevice::Truncate | QIODevice::Unbuffered);
        p.setStandardErrorFile(filePath + QLatin1String(".err"), QIODevice::Truncate | QIODevice::Unbuffered);
    }
    qDebug() << processName << " process " << args;
    p.start(processName, args, mode);
    if (!p.waitForStarted(1000)) {
        throw std::runtime_error("process start timeout");
    }
}

void waitForFinished(QProcess &p)
{
    if (!p.waitForFinished(1000)) {
        throw std::runtime_error("process finish timeout");
    }
    if (p.exitStatus() != QProcess::NormalExit || p.exitCode() != 0) {
        throw std::runtime_error(p.readAllStandardError().data());
    }
}


DcdCompletion::IdentifierType DcdCompletion::fromString(QChar c)
{
    switch (c.toLatin1()) {
    case 'c': return DCD_CLASS;
    case 'i': return DCD_INTERFACE;
    case 's': return DCD_STRUCT;
    case 'u': return DCD_UNION;
    case 'v': return DCD_VAR;
    case 'm': return DCD_MEMBER_VAR;
    case 'k': return DCD_KEYWORD;
    case 'f': return DCD_FUNCTION;
    case 'g': return DCD_ENUM_NAME;
    case 'e': return DCD_ENUM_VAR;
    case 'P': return DCD_PACKAGE;
    case 'M': return DCD_MODULE;
    case 'a': return DCD_ARRAY;
    case 'A': return DCD_ASSOC_ARRAY;
    case 'l': return DCD_ALIAS;
    case 't': return DCD_TEMPLATE;
    case 'T': return DCD_MIXIN;
    default: return DCD_NO_TYPE;
    }
    return DCD_NO_TYPE;
}


Server::Server(const QString& projectName, const QString &processName, int port, QObject *parent)
    : QObject(parent), m_projectName(projectName), m_port(port), m_processName(processName)
{
    m_process = new QProcess(this);
    connect(m_process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onError(QProcess::ProcessError)));
}

Server::~Server()
{
    DEBUG_GUARD(QLatin1String("port=") + QString::number(m_port));
    stop();
    m_process->waitForFinished(10000);
}

int Server::port() const
{
    return m_port;
}

const QString &Server::projectName() const
{
    return m_projectName;
}

void Server::setOutputFile(const QString &filePath)
{
    m_filePath = filePath;
}

void Server::start()
{
    startProcess(*m_process, m_processName, QStringList() << QLatin1String("--port") << QString::number(m_port), m_filePath);
}

void Server::stop()
{
    m_process->kill();
}

bool Server::isRunning() const
{
    return (m_process && m_process->state() == QProcess::Running);
}

void Server::onFinished(int errorCode)
{
    DEBUG_GUARD(QLatin1String("port=") + QString::number(m_port));
    if (errorCode != 0) {
        emit error(tr("DCD server process has been terminated with exit code %1").arg(errorCode));
    }
    emit finished();
}

void Server::onError(QProcess::ProcessError error)
{
    DEBUG_GUARD(QLatin1String("port=") + QString::number(m_port));
    switch (error) {
    case QProcess::FailedToStart:
        emit this->error(tr("dcd-server failed to start"));
        break;
    case QProcess::Crashed:
        emit this->error(tr("dcd-server crashed"));
        break;
    case QProcess::Timedout:
        emit this->error(tr("dcd-server starting timeout"));
        break;
    default:
        emit this->error(tr("dcd-server unknown error"));
        break;
    }
    stop();
}

inline bool isSymbolChar(QChar c)
{
    return !c.isNull() && (c.isLetterOrNumber() ||  c == QLatin1Char('_'));
}

QPair<int, int> Dcd::findSymbol(const QString &text, int pos)
{
    int bpos = pos - 1;
    for (; bpos >= 0 && isSymbolChar(text.at(bpos)); --bpos) {}
    int epos = pos;
    const int len = text.length();
    for (; epos < len && isSymbolChar(text.at(epos)); ++epos) {}
    return qMakePair(bpos + 1, epos);
}


Internal::ClientPrivate::ClientPrivate(int port)
    : m_port(-1)
{
    setPort(port);
}

void Internal::ClientPrivate::setPort(int port)
{
    DEBUG_GUARD(QLatin1String("port=") + QString::number(port));
    this->m_port = port;
}

int Internal::ClientPrivate::port() const
{
    return this->m_port;
}

void Internal::ClientPrivate::complete(const QString &sources, int position, Client::CompletionList &result)
{
    DEBUG_GUARD(QLatin1String("position=") + QString::number(position));

    AutocompleteRequest req;
    RequestKindFlag kind;
    kind.set(autocomplete);
    req.cursorPosition = position;
    req.fileName = "stdin";
    req.kind = static_cast<RequestKind>(kind.to_ulong());
    req.sourceCode = sources.toStdString();
    AutocompleteResponse rep;
    reqRep(req, rep);
    result.list.clear();
    if (rep.completionType == identifiers) {
        result.type = DCD_IDENTIFIER;
        for (size_t i = 0; i < rep.completionKinds.size(); ++i) {
            DcdCompletion c;
            c.type = DcdCompletion::fromString(rep.completionKinds[i]);
            c.data = QString::fromStdString(rep.completions[i]);
            result.list.append(c);
        }
    } else if (rep.completionType == calltips) {
        result.type = DCD_CALLTIP;
        for (size_t i = 0; i < rep.completions.size(); ++i) {
            DcdCompletion c;
            c.data = QString::fromStdString(rep.completions[i]);
            result.list.append(c);
        }
    } else {
        throw std::runtime_error("bad completion type");
    }
}

void Internal::ClientPrivate::appendIncludePath(const QStringList &includePaths)
{
    DEBUG_GUARD(QLatin1String("count=") + QString::number(includePaths.length()));
    AutocompleteRequest req;
    RequestKindFlag kind;
    kind.set(addImport);
    foreach (const QString& path, includePaths) {
        req.importPaths.push_back(path.toStdString());
    }
    req.kind = static_cast<RequestKind>(kind.to_ullong());
    this->req(req);
}

void Internal::ClientPrivate::getDocumentationComments(const QString &sources, int position, QStringList &result)
{
    DEBUG_GUARD(QLatin1String("position=") + QString::number(position));
    result.clear();
    AutocompleteRequest req;
    RequestKindFlag kind;
    kind.set(doc);
    req.cursorPosition = position;
    req.fileName = "stdin";
    req.kind = static_cast<RequestKind>(kind.to_ulong());
    req.sourceCode = sources.toStdString();
    AutocompleteResponse rep;
    reqRep(req, rep);
    foreach (const auto& s, rep.docComments) {
        result.push_back(QString::fromStdString(s));
    }
    return;
}

void Internal::ClientPrivate::findSymbolLocation(const QString &sources, int position, Client::Location &result)
{
    DEBUG_GUARD(QLatin1String("position=") + QString::number(position));
    AutocompleteRequest req;
    RequestKindFlag kind;
    kind.set(symbolLocation);
    req.cursorPosition = position;
    req.fileName = "stdin";
    req.kind = static_cast<RequestKind>(kind.to_ulong());
    req.sourceCode = sources.toStdString();
    AutocompleteResponse rep;
    reqRep(req, rep);
    result.filename = QString::fromStdString(rep.symbolFilePath);
    result.position = static_cast<int>(rep.symbolLocation);
    return;
}

void Internal::ClientPrivate::getSymbolsByName(const QString &sources, const QString &name, Client::SymbolList &result)
{
    DEBUG_GUARD(QLatin1String("name=") + name);
    Q_UNUSED(sources)
    Q_UNUSED(name)
    Q_UNUSED(result)
    // TODO
    NIY;
}

void Internal::ClientPrivate::getCurrentDocumentSymbols(const QString &sources, Client::SymbolList &result)
{
    DEBUG_GUARD("");
    Q_UNUSED(sources)
    Q_UNUSED(result)
    // TODO
    NIY;
}

#define CHECK_RETURN(op, result) if (!(op)) return result
#define CHECK_THROW(op, exc) if (!(op)) throw exc

void Internal::ClientPrivate::recv(AutocompleteResponse &rep)
{
    CHECK_THROW(m_tcp.waitForDisconnected(1000), std::runtime_error("server operation timeout"));
    QByteArray arr = m_tcp.readAll();
    msgpack::unpack(&m_unp, arr.data(), arr.size());
    msgpack::object obj = m_unp.get();
    rep = obj.as<AutocompleteResponse>();
    m_unp.zone()->clear();
}

void Internal::ClientPrivate::req(const AutocompleteRequest &req)
{
    ConnectionGuard g(this);
    send(req);
}

void Internal::ClientPrivate::reqRep(const AutocompleteRequest &req, AutocompleteResponse &rep)
{
    ConnectionGuard g(this);
    send(req);
    recv(rep);
}

void Internal::ClientPrivate::send(const AutocompleteRequest &req)
{
    m_buff.clear();
    msgpack::pack(&m_buff, req);
    size_t s = m_buff.size();
    if (m_buff.size()) {
        CHECK_THROW(m_tcp.write(reinterpret_cast<const char*>(&s), sizeof(s)) == sizeof(s), std::runtime_error("socket write of size failed"));
        CHECK_THROW(m_tcp.write(m_buff.data(), s) == static_cast<qint64>(s), std::runtime_error("socket write of data fail"));
        CHECK_THROW(m_tcp.waitForBytesWritten(1000), std::runtime_error("unfinished socket write"));
    }

    DEBUG_GUARD(QLatin1String("sent=") + QString::number(s));
}


Internal::ClientPrivate::ConnectionGuard::ConnectionGuard(ClientPrivate *parent)
    : m_parent(parent)
{
    CHECK_THROW(m_parent->m_port > 0, std::runtime_error("uninitialized port"));
    m_parent->m_tcp.connectToHost("localhost", m_parent->m_port);
    CHECK_THROW(m_parent->m_tcp.waitForConnected(100),
                std::runtime_error("failed to connect to dcd-server with port = " + std::to_string(m_parent->m_port)));
}


Internal::ClientPrivate::ConnectionGuard::~ConnectionGuard()
{
    m_parent->m_tcp.close();
}
