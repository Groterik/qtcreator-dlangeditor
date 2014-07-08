#include "dcdsupport.h"

#include <QProcess>
#include <QTextStream>

using namespace Dcd;

DcdClient::DcdClient(QString processName, int port)
    : m_port(port), m_processName(processName)
{
    m_process = new QProcess(this);
    m_portArguments << QLatin1String("--port") << QString::number(port);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    m_process->setProgram(processName);
#endif
}

bool DcdClient::complete(const QString &filePath, int position, CompletionList &result)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    m_process->setArguments(m_portArguments << QLatin1String("-c") + QString::number(position) << filePath);
    m_process->start();
#else
    m_process->start(m_processName, m_portArguments << QLatin1String("-c") + QString::number(position) << filePath);
#endif
    m_process->waitForFinished(1000);
    switch (m_process->exitStatus()) {
       case QProcess::NormalExit:
           if (m_process->exitCode() != 0) {
               m_errorString = m_process->readAllStandardError();
           } else {
               QByteArray array(m_process->readAllStandardOutput());
               return parseOutput(array, result);
           }
           break;
       case QProcess::CrashExit:
           m_errorString = m_process->readAllStandardError();
           break;
       default:
           break;
    }
    return false;
}

bool DcdClient::parseOutput(const QByteArray &output, DcdClient::CompletionList &result)
{
    result.clear();
    QTextStream stream(output);
    QString line;
    stream.readLine();
    if (line == QLatin1String("identifiers")) {
        return parseIdentifiers(stream, result);
    } else if (line == QLatin1String("calltips")) {
        return parseCalltips(stream, result);
    } else {
        m_errorString = "Unknown output type";
    }
    return false;
}

bool DcdClient::parseIdentifiers(QTextStream &stream, DcdClient::CompletionList &result)
{
    QString line;
    do {
           line = stream.readLine();
           if (line.isNull() || line.isEmpty()) break;
           QStringList tokens = line.split(QLatin1Char('\t'));
           if (tokens.size() != 2) {
               m_errorString = "Failed to parse identifiers";
               return false;
           }
           result.push_back(DcdCompletion());
           result.back().data = tokens.front();
           result.back().type = DcdCompletion::DCD_IDENTIFIER;
           result.back().identType = DcdCompletion::fromString(tokens.back());
    } while (stream.status() == QTextStream::Ok);
    return true;
}

bool DcdClient::parseCalltips(QTextStream &stream, DcdClient::CompletionList &result)
{
    QString line;
    do {
           line = stream.readLine();
           if (line.isNull() || line.isEmpty()) break;
           result.push_back(DcdCompletion());
           result.back().data = line;
           result.back().type = DcdCompletion::DCD_CALLTIP;
           result.back().identType = DcdCompletion::DCD_NO_TYPE;
    } while (stream.status() == QTextStream::Ok);
    return true;
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


DcdServer::DcdServer(QString processName, int port)
    : m_port(port), m_processName(processName)
{
    m_process = new QProcess(this);
    connect(m_process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
}

void DcdServer::setProgram(QString program)
{
    m_processName = program;
}

void DcdServer::setPort(int port)
{
    m_port = port;
}

bool DcdServer::start()
{
    m_process->start(m_processName, QStringList() << QLatin1String("--port") << QString::number(m_port), QIODevice::NotOpen);
    m_process->waitForStarted(5000);
    return true;
}

void DcdServer::onFinished(int errorCode)
{
    if (errorCode != 0) {
        emit error(tr("DCD server process has been terminated with exit code %1").arg(errorCode));
    }
}
