#include "codemodel/dastedoptions.h"

#include "codemodel/dastedmessages.h"

#include <coreplugin/icore.h>
#include <utils/pathchooser.h>
#include <utils/pathlisteditor.h>

#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QSpinBox>

using namespace Dasted;

namespace {
const QString S_DASTED_SETTINGS = QLatin1String("DastedSettings");
const char S_INCLUDE_DIR[] = "includeDirs";
const char S_DASTED_SERVER[] = "dastedServerExecutable";
const char S_DASTED_PARAMETERS[] = "dastedServerParameters";
const char S_DASTED_SERVER_LOG[] = "dastedServerLog";
const char S_DASTED_PORT[] = "dastedServerPort";
const char S_DASTED_AUTOSTART[] = "dastedServerAutoStart";
}

DastedOptionsPageWidget::DastedOptionsPageWidget(QWidget *parent)
    : IModelOptionsWidget(parent)
{
    QFormLayout *formLayout = new QFormLayout(this);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    auto* versionLayout = new QHBoxLayout;
    auto versionLabel = new QLabel(QString::number(PROTOCOL_VERSION));
    versionLayout->addWidget(versionLabel);
    auto *checkVersionButton = new QPushButton("Check");
    connect(checkVersionButton, &QPushButton::clicked, [this](bool){
        this->checkVersion();
    });
    versionLayout->addWidget(checkVersionButton);
    formLayout->addRow(tr("Dasted protocol version:"), versionLayout);

    m_autoStart = new QCheckBox;
    m_autoStart->setChecked(DastedOptionsPage::autoStart());
    connect(m_autoStart, SIGNAL(stateChanged(int)), this, SIGNAL(updatedAndNeedRestart()));
    formLayout->addRow(tr("Dasted autostart:"), m_autoStart);

    m_server = new Utils::PathChooser(this);
    m_server->setExpectedKind(Utils::PathChooser::ExistingCommand);
    m_server->setHistoryCompleter(QLatin1String("Dlang.Command.DastedServer.History"));
    m_server->setPath(DastedOptionsPage::dastedServerExecutable());
    connect(m_server, SIGNAL(pathChanged(QString)), this, SIGNAL(updatedAndNeedRestart()));
    formLayout->addRow(tr("Dasted executable:"), m_server);

    m_serverParameters = new QLineEdit(this);
    m_serverParameters->setText(DastedOptionsPage::dastedParameters());
    connect(m_serverParameters, SIGNAL(textChanged(QString)),
            this, SIGNAL(updatedAndNeedRestart()));
    formLayout->addRow(tr("Dasted parameters:"), m_serverParameters);

    m_serverLog = new Utils::PathChooser(this);
    m_serverLog->setExpectedKind(Utils::PathChooser::SaveFile);
    m_serverLog->setHistoryCompleter(QLatin1String("Dlang.Command.DastedServerLog.History"));
    m_serverLog->setPath(DastedOptionsPage::dastedServerLogPath());
    connect(m_serverLog, SIGNAL(pathChanged(QString)), this, SIGNAL(updatedAndNeedRestart()));
    formLayout->addRow(tr("Dasted log path:"), m_serverLog);

    m_includes = new Utils::PathListEditor(this);
    m_includes->setPathList(DastedOptionsPage::includePaths());
    formLayout->addRow(tr("Include paths:"), m_includes);

    m_port = new QSpinBox(this);
    m_port->setRange(0, 100000);
    m_port->setValue(DastedOptionsPage::port());
    formLayout->addRow(tr("Port"), m_port);
    connect(m_port, SIGNAL(valueChanged(int)), this, SIGNAL(updatedAndNeedRestart()));
}

DastedOptionsPageWidget::~DastedOptionsPageWidget()
{
    disconnect(m_port, SIGNAL(valueChanged(int)), this, SIGNAL(updatedAndNeedRestart()));
}

QString DastedOptionsPageWidget::serverExecutable() const
{
    return m_server->path();
}

QString DastedOptionsPageWidget::serverParameters() const
{
    return m_serverParameters->text();
}

QString DastedOptionsPageWidget::serverLogPath() const
{
    return m_serverLog->path();
}

QStringList DastedOptionsPageWidget::includePaths() const
{
    return m_includes->pathList();
}

int DastedOptionsPageWidget::port() const
{
    return static_cast<int>(m_port->value());
}

bool DastedOptionsPageWidget::autoStart() const
{
    return m_autoStart->isChecked();
}

void DastedOptionsPageWidget::apply()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(S_DASTED_SETTINGS);
    settings->setValue(QLatin1String(S_DASTED_SERVER), serverExecutable());
    settings->setValue(QLatin1String(S_DASTED_SERVER_LOG), serverLogPath());
    settings->setValue(QLatin1String(S_INCLUDE_DIR), includePaths());
    settings->setValue(QLatin1String(S_DASTED_PORT), port());
    settings->setValue(QLatin1String(S_DASTED_AUTOSTART), autoStart());
    settings->endGroup();
}

bool DastedOptionsPageWidget::checkVersion()
{
    QProcess process;
    process.start(serverExecutable(), QStringList() << "--version");
    if (!process.waitForFinished(1000) ||
            process.exitStatus() != QProcess::NormalExit ||
            process.exitCode() != 0) {
        QMessageBox::warning(this, tr("Dasted error"),
                             QLatin1String("Process failed. Please check the executable path."));
        return false;
    }
    QString processVersion = process.readAllStandardOutput();
    processVersion = processVersion.trimmed();
    if (processVersion != QString::number(PROTOCOL_VERSION)) {
        QMessageBox::warning(this, tr("Version checked"),
                             tr("Version mismatches: dasted process returned %1.").arg(processVersion));
        return false;
    }
    QMessageBox::information(this, tr("Version checked"), tr("Version matches."));
    return true;
}


QString DastedOptionsPage::dastedServerExecutable()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(S_DASTED_SETTINGS);
    QString result = settings->value(QLatin1String(S_DASTED_SERVER), QLatin1String("dasted")).toString();
    settings->endGroup();
    return result;
}

QString DastedOptionsPage::dastedParameters()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(S_DASTED_SETTINGS);
    QString result = settings->value(QLatin1String(S_DASTED_PARAMETERS),
                                     QLatin1String("")).toString();
    settings->endGroup();
    return result;
}

QString DastedOptionsPage::dastedServerLogPath()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(S_DASTED_SETTINGS);
    QString result = settings->value(QLatin1String(S_DASTED_SERVER_LOG), QLatin1String("")).toString();
    settings->endGroup();
    return result;
}

QStringList DastedOptionsPage::includePaths()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(S_DASTED_SETTINGS);
    QStringList result = settings->value(QLatin1String(S_INCLUDE_DIR), QStringList("/usr/include/dlang/dmd/")).toStringList();
    settings->endGroup();
    return result;
}

int DastedOptionsPage::port()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(S_DASTED_SETTINGS);
    int result = settings->value(QLatin1String(S_DASTED_PORT), 11344).toInt();
    settings->endGroup();
    return result;
}

bool DastedOptionsPage::autoStart()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(S_DASTED_SETTINGS);
    bool result = settings->value(QLatin1String(S_DASTED_AUTOSTART), true).toBool();
    settings->endGroup();
    return result;
}
