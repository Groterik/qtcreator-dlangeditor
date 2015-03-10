#include "codemodel/dcdoptions.h"

#include <utils/pathchooser.h>
#include <utils/pathlisteditor.h>
#include <coreplugin/icore.h>

#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>

using namespace Dcd;

namespace {
const QString S_DCD_SETTINGS = QLatin1String("DcdSettings");
const char S_INCLUDE_DIR[] = "includeDir";
const char S_DCD_SERVER[] = "dcdServerExecutable";
const char S_DCD_SERVER_LOG[] = "dcdServerLog";
const char S_DCD_PORTS_FIRST[] = "dcdServerPortsRangeFirst";
const char S_DCD_PORTS_LAST[] = "dcdServerPortsRangeLast";
const char S_HOVER_ENABLE[] = "hoverEnable";
}

DcdOptionsPageWidget::DcdOptionsPageWidget(QWidget *parent)
    : IModelOptionsWidget(parent)
{
    QFormLayout *formLayout = new QFormLayout(this);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_server = new Utils::PathChooser(this);
    m_server->setExpectedKind(Utils::PathChooser::ExistingCommand);
    m_server->setHistoryCompleter(QLatin1String("Dlang.Command.Server.History"));
    m_server->setPath(DcdOptionsPage::dcdServerExecutable());
    formLayout->addRow(tr("DCD server executable:"), m_server);

    m_serverLog = new Utils::PathChooser(this);
    m_serverLog->setExpectedKind(Utils::PathChooser::SaveFile);
    m_serverLog->setHistoryCompleter(QLatin1String("Dlang.Command.ServerLog.History"));
    m_serverLog->setPath(DcdOptionsPage::dcdServerLogPath());
    formLayout->addRow(tr("DCD server log path:"), m_serverLog);

    m_includes = new Utils::PathListEditor(this);
    m_includes->setPathList(DcdOptionsPage::includePaths());
    formLayout->addRow(tr("Include paths:"), m_includes);

    m_firstPort = new QSpinBox(this);
    m_firstPort->setRange(0, 100000);
    m_firstPort->setValue(DcdOptionsPage::portsRange().first);
    formLayout->addRow(tr("First port"), m_firstPort);
    connect(m_firstPort, SIGNAL(valueChanged(int)), this, SIGNAL(updatedAndNeedRestart()));

    m_lastPort = new QSpinBox(this);
    m_lastPort->setRange(0, 100000);
    m_lastPort->setValue(DcdOptionsPage::portsRange().second);
    formLayout->addRow(tr("Last port"), m_lastPort);
    connect(m_lastPort, SIGNAL(valueChanged(int)), this, SIGNAL(updatedAndNeedRestart()));

    m_hoverEnable = new QCheckBox(this);
    m_hoverEnable->setChecked(DcdOptionsPage::hoverEnable());
    formLayout->addRow(tr("Help tooltips"), m_hoverEnable);

}

DcdOptionsPageWidget::~DcdOptionsPageWidget()
{
    disconnect(m_firstPort, SIGNAL(valueChanged(int)), this, SIGNAL(updatedAndNeedRestart()));
    disconnect(m_lastPort, SIGNAL(valueChanged(int)), this, SIGNAL(updatedAndNeedRestart()));
}

QString DcdOptionsPageWidget::serverExecutable() const
{
    return m_server->path();
}

QString DcdOptionsPageWidget::serverLogPath() const
{
    return m_serverLog->path();
}

QStringList DcdOptionsPageWidget::includePaths() const
{
    return m_includes->pathList();
}

QPair<int, int> DcdOptionsPageWidget::portsRange() const
{
    return qMakePair(static_cast<int>(m_firstPort->value()), static_cast<int>(m_lastPort->value()));
}

bool DcdOptionsPageWidget::hoverEnable() const
{
    return m_hoverEnable->isChecked();
}

void DcdOptionsPageWidget::apply()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(S_DCD_SETTINGS);
    settings->setValue(QLatin1String(S_DCD_SERVER), serverExecutable());
    settings->setValue(QLatin1String(S_DCD_SERVER_LOG), serverLogPath());
    settings->setValue(QLatin1String(S_INCLUDE_DIR), includePaths());
    settings->setValue(QLatin1String(S_DCD_PORTS_FIRST), portsRange().first);
    settings->setValue(QLatin1String(S_DCD_PORTS_LAST), portsRange().second);
    settings->setValue(QLatin1String(S_HOVER_ENABLE), hoverEnable());
    settings->endGroup();
}


// DcdOptionsPage
QString DcdOptionsPage::dcdServerExecutable()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(S_DCD_SETTINGS);
    QString result = settings->value(QLatin1String(S_DCD_SERVER), QLatin1String("dcd-server")).toString();
    settings->endGroup();
    return result;
}

QString DcdOptionsPage::dcdServerLogPath()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(S_DCD_SETTINGS);
    QString result = settings->value(QLatin1String(S_DCD_SERVER_LOG), QLatin1String("")).toString();
    settings->endGroup();
    return result;
}

QStringList DcdOptionsPage::includePaths()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(S_DCD_SETTINGS);
    QStringList result = settings->value(QLatin1String(S_INCLUDE_DIR), QStringList("/usr/include/dlang/dmd/")).toStringList();
    settings->endGroup();
    return result;
}

QPair<int, int> DcdOptionsPage::portsRange()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(S_DCD_SETTINGS);
    int first = settings->value(QLatin1String(S_DCD_PORTS_FIRST), 9167).toInt();
    int last = settings->value(QLatin1String(S_DCD_PORTS_LAST), 9197).toInt();
    settings->endGroup();
    return qMakePair(first, last);
}

bool DcdOptionsPage::hoverEnable()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(S_DCD_SETTINGS);
    bool result = settings->value(QLatin1String(S_HOVER_ENABLE), false).toBool();
    settings->endGroup();
    return result;
}
