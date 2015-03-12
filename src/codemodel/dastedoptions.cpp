#include "codemodel/dastedoptions.h"

#include <utils/pathchooser.h>
#include <utils/pathlisteditor.h>
#include <coreplugin/icore.h>

#include <QFormLayout>
#include <QSpinBox>

using namespace Dasted;

namespace {
const QString S_DASTED_SETTINGS = QLatin1String("DastedSettings");
const char S_INCLUDE_DIR[] = "includeDirs";
const char S_DASTED_SERVER[] = "dastedServerExecutable";
const char S_DASTED_SERVER_LOG[] = "dastedServerLog";
const char S_DASTED_PORT[] = "dastedServerPort";
}

DastedOptionsPageWidget::DastedOptionsPageWidget(QWidget *parent)
    : IModelOptionsWidget(parent)
{
    QFormLayout *formLayout = new QFormLayout(this);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_server = new Utils::PathChooser(this);
    m_server->setExpectedKind(Utils::PathChooser::ExistingCommand);
    m_server->setHistoryCompleter(QLatin1String("Dlang.Command.DastedServer.History"));
    m_server->setPath(DastedOptionsPage::dastedServerExecutable());
    formLayout->addRow(tr("Dasted server executable:"), m_server);

    m_serverLog = new Utils::PathChooser(this);
    m_serverLog->setExpectedKind(Utils::PathChooser::SaveFile);
    m_serverLog->setHistoryCompleter(QLatin1String("Dlang.Command.DastedServerLog.History"));
    m_serverLog->setPath(DastedOptionsPage::dastedServerLogPath());
    formLayout->addRow(tr("Dasted server log path:"), m_serverLog);

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

void DastedOptionsPageWidget::apply()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(S_DASTED_SETTINGS);
    settings->setValue(QLatin1String(S_DASTED_SERVER), serverExecutable());
    settings->setValue(QLatin1String(S_DASTED_SERVER_LOG), serverLogPath());
    settings->setValue(QLatin1String(S_INCLUDE_DIR), includePaths());
    settings->setValue(QLatin1String(S_DASTED_PORT), port());
    settings->endGroup();
}


QString DastedOptionsPage::dastedServerExecutable()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(S_DASTED_SETTINGS);
    QString result = settings->value(QLatin1String(S_DASTED_SERVER), QLatin1String("dasted")).toString();
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
