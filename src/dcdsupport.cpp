#include "dcdsupport.h"

#include "dlangdebughelper.h"
#include <dlangoptionspage.h>

#include <stdexcept>
#include <bitset>
#include <sstream>

#include <QProcess>
#include <QTextStream>
#include <QDebug>
#include <QTcpSocket>

#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>
#include <cpptools/cppmodelmanager.h>

#include <msgpack.hpp>

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
        uninitialized = 0,
        /// Autocompletion
        autocomplete,
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
        std::vector<ubyte> sourceCode;

        /**
         * The cursor position
         */
        size_t cursorPosition;

        /**
         * Name of symbol searched for
         */
        std::string searchName;

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
        std::vector<char> completionKinds;

        /**
         * Symbol locations for symbol searches.
         */
        std::vector<size_t> locations;

        MSGPACK_DEFINE(completionType, symbolFilePath, symbolLocation, docComments, completions, completionKinds, locations)
};
} // namespace Dcd

class Client
{
public:
    Client(int port);

    void setPort(int port) {
        this->port = port;
    }

private:

    void send(const AutocompleteRequest &req);
    void recv(AutocompleteResponse &rep);

    void reqRep(const AutocompleteRequest &req, AutocompleteResponse &rep);

    class ConnectionGuard
    {
    public:
        ConnectionGuard(Client *parent);
        ~ConnectionGuard();

    private:
        Client *m_parent;
    };

    msgpack::sbuffer buff;
    msgpack::unpacked unp;
    int port;
    QTcpSocket tcp;
};

DcdClient::DcdClient(const QString &projectName, const QString &processName, int port, QObject *parent)
    : QObject(parent), m_projectName(projectName), m_port(port), m_processName(processName)
{
    m_portArguments << QLatin1String("--port") << QString::number(port);
}

const QString &DcdClient::projectName() const
{
    return m_projectName;
}

void DcdClient::setOutputFile(const QString &filePath)
{
    m_filePath = filePath;
}

void startProcess(QProcess &p, const QString &processName, const QStringList &args,
                  const QString &filePath, QIODevice::OpenMode mode = QIODevice::ReadWrite)
{
    if (p.state() != QProcess::NotRunning) {
        throw std::runtime_error("process is already running");
    }
    if (!filePath.isEmpty()) {
        p.setStandardOutputFile(filePath, QIODevice::Append | QIODevice::Unbuffered);
        p.setStandardErrorFile(filePath, QIODevice::Append | QIODevice::Unbuffered);
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

void DcdClient::complete(const QString &filePath, int position, CompletionList &result)
{
    DEBUG_GUARD("");
    QStringList args = m_portArguments;
    args << QLatin1String("-c") + QString::number(position) << filePath;
    qDebug() << "dcd-client process " << args;
    QProcess process;
    startProcess(process, m_processName, args, m_filePath);
    waitForFinished(process);
    QByteArray array(process.readAllStandardOutput());
    return parseOutput(array, result);
}

void DcdClient::completeFromArray(const QString &array, int position, DcdClient::CompletionList &result)
{
    DEBUG_GUARD(QString::number(position));
    QStringList args = m_portArguments;
    args << QLatin1String("-c") + QString::number(position);
    QProcess process;
    startProcess(process, m_processName, args, m_filePath);
    process.write(array.toLatin1());
    if (!process.waitForBytesWritten(5000)) {
        throw std::runtime_error("process writing data timeout");
    }
    process.closeWriteChannel();
    waitForFinished(process);
    QByteArray output(process.readAllStandardOutput());
    return parseOutput(output, result);
}

void DcdClient::appendIncludePath(const QString &includePath)
{
    DEBUG_GUARD("");
    QStringList args = m_portArguments;
    args << QLatin1String("-I") + includePath;
    qDebug() << "dcd-client process " << args;
    QProcess process;
    startProcess(process, m_processName, args, m_filePath);
    waitForFinished(process);
}

void DcdClient::findSymbolLocation(const QString &array, int position, DcdClient::Location &result)
{
    if (position > 0) {
        --position;
    }
    DEBUG_GUARD(QString::number(position));
    position = findSymbol(array, position).second;
    QStringList args = m_portArguments;
    args << QLatin1String("-c") + QString::number(position) << "-l";
    qDebug() << "dcd-client process " << args;
    QProcess process;
    startProcess(process, m_processName, args, m_filePath);
    process.write(array.toLatin1());
    if (!process.waitForBytesWritten(2000)) {
        throw std::runtime_error("process writing data timeout");
    }
    process.closeWriteChannel();
    waitForFinished(process);
    QString str(process.readAllStandardOutput());
    QStringList list = str.split('\t');
    result = list.size() == 2 ? Location(list.front(), list.back().toInt()) : Location(QString(), 0);
}

void DcdClient::getDocumentationComments(const QString &array, int position, QStringList &result)
{
    DEBUG_GUARD(QString::number(position));
    QStringList args = m_portArguments;
    args << QLatin1String("-c") + QString::number(position) << "-d";
    qDebug() << "dcd-client process " << args;
    QProcess process;
    startProcess(process, m_processName, args, m_filePath);
    process.write(array.toLatin1());
    if (!process.waitForBytesWritten(2000)) {
        throw std::runtime_error("process writing data timeout");
    }
    process.closeWriteChannel();
    waitForFinished(process);
    QString str(process.readAllStandardOutput());
    result = str.split('\n');
}

void DcdClient::getSymbolsByName(const QString &array, const QString &name, DcdClient::DcdSymbolList &result)
{
    DEBUG_GUARD(name);
    QStringList args = m_portArguments;
    args << QLatin1String("--search") << name;
    qDebug() << "dcd-client process " << args;
    QProcess process;
    startProcess(process, m_processName, args, m_filePath);
    process.write(array.toLatin1());
    if (!process.waitForBytesWritten(2000)) {
        throw std::runtime_error("process writing data timeout");
    }
    process.closeWriteChannel();
    waitForFinished(process);
    result.clear();
    parseSymbols(process.readAllStandardOutput(), result);
}

void DcdClient::parseOutput(const QByteArray &output, DcdClient::CompletionList &result)
{
    result.list.clear();
    result.type = DCD_BAD_TYPE;
    QTextStream stream(output);
    QString line = stream.readLine();
    if (line == QLatin1String("identifiers")) {
        return parseIdentifiers(stream, result);
    } else if (line == QLatin1String("calltips")) {
        return parseCalltips(stream, result);
    } else if (line.isEmpty()) {
        return;
    } else {
        throw std::runtime_error("unknown output type");
    }
}

void DcdClient::parseIdentifiers(QTextStream &stream, DcdClient::CompletionList &result)
{
    QString line;
    do {
           line = stream.readLine();
           if (line.isNull() || line.isEmpty()) break;
           QStringList tokens = line.split(QLatin1Char('\t'));
           if (tokens.size() != 2) {
               throw std::runtime_error("Failed to parse identifiers");
           }
           result.type = DCD_IDENTIFIER;
           result.list.push_back(DcdCompletion());
           result.list.back().data = tokens.front();
           result.list.back().type = DcdCompletion::fromString(tokens.back());
    } while (stream.status() == QTextStream::Ok);
}

void DcdClient::parseCalltips(QTextStream &stream, DcdClient::CompletionList &result)
{
    QString line;
    do {
           line = stream.readLine();
           if (line.isNull() || line.isEmpty()) break;
           result.type = DCD_CALLTIP;
           result.list.push_back(DcdCompletion());
           result.list.back().data = line;
           result.list.back().type = DcdCompletion::DCD_NO_TYPE;
    } while (stream.status() == QTextStream::Ok);
}

void DcdClient::parseSymbols(const QByteArray &output, DcdClient::DcdSymbolList &result)
{
    QTextStream stream(output);
    QString line;
    do {
           line = stream.readLine();
           if (line.isNull() || line.isEmpty()) break;
           QStringList ls = line.split('\t');
           if (ls.length() != 3) break;
           Location loc(ls.at(0), ls.at(2).toInt());
           DcdCompletion::IdentifierType type = DcdCompletion::fromString(ls.at(1));
           result.push_back(qMakePair(loc, type));
    } while (stream.status() == QTextStream::Ok);
}


DcdCompletion::IdentifierType DcdCompletion::fromString(const QString &name)
{
    char c = name.at(0).toLatin1();
    switch (c) {
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


DcdServer::DcdServer(const QString& projectName, const QString &processName, int port, QObject *parent)
    : QObject(parent), m_projectName(projectName), m_port(port), m_processName(processName)
{
    m_process = new QProcess(this);
    connect(m_process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onError(QProcess::ProcessError)));
}

DcdServer::~DcdServer()
{
    DEBUG_GUARD("");
    stop();
    m_process->waitForFinished(10000);
}

int DcdServer::port() const
{
    return m_port;
}

const QString &DcdServer::projectName() const
{
    return m_projectName;
}

void DcdServer::setOutputFile(const QString &filePath)
{
    m_filePath = filePath;
}

void DcdServer::start()
{
    startProcess(*m_process, m_processName, QStringList() << QLatin1String("--port") << QString::number(m_port), m_filePath);
}

void DcdServer::stop()
{
    m_process->kill();
}

bool DcdServer::isRunning() const
{
    return (m_process && m_process->state() == QProcess::Running);
}

void DcdServer::onFinished(int errorCode)
{
    qDebug() << "DCD server finished";
    if (errorCode != 0) {
        emit error(tr("DCD server process has been terminated with exit code %1").arg(errorCode));
        qWarning("DCD server: %s", static_cast<QProcess*>(sender())->readAllStandardError().data());
    }
}

void DcdServer::onError(QProcess::ProcessError error)
{
    qDebug() << "DCD server error";
    switch (error) {
    case QProcess::FailedToStart:
        emit this->error(tr("DCD server failed to start"));
        break;
    case QProcess::Crashed:
        emit this->error(tr("DCD server crashed"));
        break;
    case QProcess::Timedout:
        emit this->error(tr("DCD server starting timeout"));
        break;
    default:
        emit this->error(tr("DCD server unknown error"));
        break;
    }
    stop();
}

// Factory
DcdFactory::ClientPointer DcdFactory::client(const QString &projectName)
{
    try {
        MapString::iterator it = mapChannels.find(projectName);
        if (it == mapChannels.end()) {
            int port =  m_firstPort + currentPortOffset % (m_lastPort - m_firstPort + 1);
            ServerPointer server(new Dcd::DcdServer(projectName, DlangEditor::DlangOptionsPage::dcdServerExecutable(), port, this));
            server->setOutputFile(DlangEditor::DlangOptionsPage::dcdServerLogPath());
            server->start();
            connect(server.data(), SIGNAL(error(QString)), this, SLOT(onError(QString)));
            ClientPointer client(new Dcd::DcdClient(projectName, DlangEditor::DlangOptionsPage::dcdClientExecutable(), port, this));
            appendIncludePaths(client);

            it = mapChannels.insert(projectName, qMakePair(client, server));
            ++currentPortOffset;
        } else {
            if (!it.value().second->isRunning()) {
                it.value().second->stop();
                mapChannels.erase(it);
                return ClientPointer();
            }
        }
        return it.value().first;
    } catch (std::exception &ex) {
        qDebug("Client exception: %s", ex.what());
    } catch (...) {
        qDebug("Client exception: unknown");
    }
    return ClientPointer();
}

void DcdFactory::appendIncludePaths(ClientPointer client)
{
    // append default include paths from options page
    QStringList list = DlangEditor::DlangOptionsPage::includePaths();

    // append include paths from project settings
    CppTools::CppModelManager *modelmanager =
            CppTools::CppModelManager::instance();
    if (modelmanager) {
        ProjectExplorer::Project *currentProject = ProjectExplorer::ProjectExplorerPlugin::currentProject();
        if (currentProject) {
            CppTools::ProjectInfo pinfo = modelmanager->projectInfo(currentProject);
            if (pinfo.isValid()) {
                foreach (const CppTools::ProjectPart::HeaderPath &header, pinfo.headerPaths()) {
                    if (header.isValid()) {
                        list.push_back(header.path);
                    }
                }
            }
        }
    }
    list.removeDuplicates();

    foreach (const QString& l, list) {
        client->appendIncludePath(l);
    }
}

void DcdFactory::setPortRange(int first, int last)
{
    m_firstPort = first;
    m_lastPort = std::max(last, first);
}

QPair<int, int> DcdFactory::portRange() const
{
    return qMakePair(m_firstPort, m_lastPort);
}

DcdFactory *DcdFactory::instance()
{
    static DcdFactory inst(DlangEditor::DlangOptionsPage::portsRange());
    return &inst;
}

void DcdFactory::onError(QString error)
{
    qDebug("DcdFactory::onError: %s", error.toStdString().data());
    qWarning("DcdFactory::onError: %s", error.toStdString().data());
    Dcd::DcdServer *server = qobject_cast<Dcd::DcdServer*>(sender());
    server->stop();
    mapChannels.remove(server->projectName());
}

DcdFactory::DcdFactory(QPair<int, int> range)
    : currentPortOffset(0)
{
    setPortRange(range.first, range.second);
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


Client::Client(int port)
{
    tcp.connectToHost("localhost", port);
}

#define CHECK_RETURN(op, result) if (!(op)) return result
#define CHECK_THROW(op, exc) if (!(op)) throw exc

void Client::recv(AutocompleteResponse &rep)
{
    tcp.waitForDisconnected(1000);
    QByteArray arr = tcp.readAll();
    CHECK_THROW(!arr.isNull(), std::runtime_error("null byte array from socket"));
    msgpack::unpack(&unp, arr.data(), arr.size());
    rep = unp.get().as<AutocompleteResponse>();
}

void Client::reqRep(const AutocompleteRequest &req, AutocompleteResponse &rep)
{
    ConnectionGuard g(this);
    send(req);
    recv(rep);
}

void Client::send(const AutocompleteRequest &req)
{
    msgpack::pack(&buff, req);
    size_t s = buff.size();
    if (buff.size()) {
        CHECK_THROW(tcp.write(reinterpret_cast<const char*>(&s), sizeof(s)) == sizeof(s), std::runtime_error("socket write of size failed"));
        CHECK_THROW(tcp.write(buff.data(), s) == static_cast<qint64>(s), std::runtime_error("socket write of data fail"));
    }
}


Client::ConnectionGuard::ConnectionGuard(Client *parent)
    : m_parent(parent)
{
    m_parent->tcp.connectToHost("localhost", m_parent->port);
    CHECK_THROW(!m_parent->tcp.waitForConnected(100),
                std::runtime_error("Failed to connect to dcd-server with port = " + std::to_string(m_parent->port)));
}


Client::ConnectionGuard::~ConnectionGuard()
{
    m_parent->tcp.close();
}
