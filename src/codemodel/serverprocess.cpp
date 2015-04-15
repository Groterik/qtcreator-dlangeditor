#include "codemodel/serverprocess.h"

#include "dlangdebughelper.h"

#include <QDebug>

using namespace DlangEditor::Utils;

static void startProcess(QProcess &p, const QString &processName, const QStringList &args,
                  const QString &filePath, QIODevice::OpenMode mode = QIODevice::ReadWrite)
{
    DEBUG_GUARD(processName + QChar(' ') + args.join(' '));
    if (p.state() != QProcess::NotRunning) {
        throw std::runtime_error("process is already running");
    }
    if (!filePath.isEmpty()) {
        p.setStandardOutputFile(filePath+ QLatin1String(".out"), QIODevice::Truncate | QIODevice::Unbuffered);
        p.setStandardErrorFile(filePath + QLatin1String(".err"), QIODevice::Truncate | QIODevice::Unbuffered);
    }
    p.start(processName, args, mode);
    if (!p.waitForStarted(1000)) {
        throw std::runtime_error("process start timeout");
    }
}

ServerDaemon::ServerDaemon(QObject *parent, const QString &processName, const QStringList &args)
    : QObject(parent), m_processName(processName), m_args(args)
{
    connect(&m_process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onError(QProcess::ProcessError)));
}

ServerDaemon::~ServerDaemon()
{
    DEBUG_GUARD(m_processName + m_args.join(' '));
    stop();
    m_process.waitForFinished(3000);
}

const QStringList &ServerDaemon::arguments() const
{
    return m_args;
}

void ServerDaemon::setArguments(const QStringList &args)
{
    m_args = args;
}

void ServerDaemon::setOutputFile(const QString &filePath)
{
    m_filePath = filePath;
}

void ServerDaemon::start()
{
    startProcess(m_process, m_processName, m_args, m_filePath);
}

void ServerDaemon::stop()
{
    m_process.kill();
}

bool ServerDaemon::isRunning() const
{
    return (m_process.state() == QProcess::Running);
}

void ServerDaemon::onFinished(int errorCode)
{
    DEBUG_GUARD(m_processName + m_args.join(' '));
    qDebug() << m_process.readAllStandardError();
    if (errorCode != 0) {
        emit error(tr("Process has been terminated with exit code %1").arg(errorCode));
    }
    emit finished();
}

void ServerDaemon::onError(QProcess::ProcessError error)
{
    DEBUG_GUARD(m_processName + m_args.join(' '));
    switch (error) {
    case QProcess::FailedToStart:
        emit this->error(tr("process failed to start"));
        break;
    case QProcess::Crashed:
        emit this->error(tr("process crashed"));
        break;
    case QProcess::Timedout:
        emit this->error(tr("process starting timeout"));
        break;
    default:
        emit this->error(tr("unknown process error"));
        break;
    }
    stop();
}
