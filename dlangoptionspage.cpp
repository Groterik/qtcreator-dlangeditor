#include "dlangoptionspage.h"

#include "dlangeditorconstants.h"

#include <utils/pathchooser.h>
#include <coreplugin/icore.h>

#include <QFormLayout>
#include <QDoubleSpinBox>

using namespace DlangEditor;

namespace {
const char S_PHOBOS_DIR[] = "phobosDir";
const char S_DCD_CLIENT[] = "dcdClientExecutable";
const char S_DCD_SERVER[] = "dcdServerExecutable";
const char S_DCD_PORTS_FIRST[] = "dcdServerPortsRangeFirst";
const char S_DCD_PORTS_LAST[] = "dcdServerPortsRangeLast";
}

DlangOptionsPage::DlangOptionsPage()
{
    setId(Constants::DLANG_CODE_MODEL_SETTINGS_ID);
    setDisplayName(Constants::DLANG_CODE_MODEL_SETTINGS_NAME);
    setCategory(Constants::DLANG_SETTINGS_CATEGORY_ID);
    setDisplayCategory(Constants::DLANG_SETTINGS_CATEGORY_NAME);
    //    setCategoryIcon(QLatin1String(Constants::SETTINGS_CATEGORY_DLANG_ICON));
}

DlangOptionsPage::~DlangOptionsPage()
{

}

QWidget *DlangOptionsPage::widget()
{
    if (!m_widget) {
        m_widget = new DlangOptionsPageWidget;
    }
    return m_widget;
}

void DlangOptionsPage::apply()
{
    if (m_widget) {
        QSettings *settings = Core::ICore::settings();
        settings->beginGroup(tr("DlangSettings"));
        settings->setValue(QLatin1String(S_DCD_CLIENT), m_widget->clientExecutable());
        settings->setValue(QLatin1String(S_DCD_SERVER), m_widget->serverExecutable());
        settings->setValue(QLatin1String(S_PHOBOS_DIR), m_widget->phobosPath());
        settings->endGroup();
    }
}

void DlangOptionsPage::finish()
{
    delete m_widget;
}

QString DlangOptionsPage::dcdClientExecutable()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(tr("DlangSettings"));
    QString result = settings->value(QLatin1String(S_DCD_CLIENT), QLatin1String("dcd-client")).toString();
    settings->endGroup();
    return result;
}

QString DlangOptionsPage::dcdServerExecutable()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(tr("DlangSettings"));
    QString result = settings->value(QLatin1String(S_DCD_SERVER), QLatin1String("dcd-server")).toString();
    settings->endGroup();
    return result;
}

QString DlangOptionsPage::phobosDir()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(tr("DlangSettings"));
    QString result = settings->value(QLatin1String(S_PHOBOS_DIR), QLatin1String("/usr/include/dlang/dmd/")).toString();
    settings->endGroup();
    return result;
}

QPair<int, int> DlangOptionsPage::portsRange()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(tr("DlangSettings"));
    int first = settings->value(QLatin1String(S_DCD_PORTS_FIRST), 9167).toInt();
    int last = settings->value(QLatin1String(S_DCD_PORTS_LAST), 9197).toInt();
    settings->endGroup();
    return qMakePair(first, last);
}


DlangOptionsPageWidget::DlangOptionsPageWidget(QWidget *parent)
    : QWidget(parent)
{
    QFormLayout *formLayout = new QFormLayout(this);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_client = new Utils::PathChooser(this);
    m_client->setExpectedKind(Utils::PathChooser::ExistingCommand);
    m_client->setHistoryCompleter(QLatin1String("Dlang.Command.Client.History"));
    m_client->setPath(DlangOptionsPage::dcdClientExecutable());
    formLayout->addRow(tr("DCD client executable:"), m_client);

    m_server = new Utils::PathChooser(this);
    m_server->setExpectedKind(Utils::PathChooser::ExistingCommand);
    m_server->setHistoryCompleter(QLatin1String("Dlang.Command.Server.History"));
    m_server->setPath(DlangOptionsPage::dcdServerExecutable());
    formLayout->addRow(tr("DCD server executable:"), m_server);

    m_phobos = new Utils::PathChooser(this);
    m_phobos->setExpectedKind(Utils::PathChooser::ExistingCommand);
    m_phobos->setHistoryCompleter(QLatin1String("Dlang.Command.PhobosDir.History"));
    m_phobos->setPath(DlangOptionsPage::phobosDir());
    formLayout->addRow(tr("Phobos path:"), m_phobos);

    m_firstPort = new QDoubleSpinBox(this);
    m_firstPort->setDecimals(0);
    formLayout->addRow(tr("First port"), m_firstPort);

    m_lastPort = new QDoubleSpinBox(this);
    m_lastPort->setDecimals(0);
    formLayout->addRow(tr("Last port"), m_lastPort);

}

DlangOptionsPageWidget::~DlangOptionsPageWidget()
{

}

QString DlangOptionsPageWidget::clientExecutable() const
{
    return m_client->path();
}

QString DlangOptionsPageWidget::serverExecutable() const
{
    return m_server->path();
}

QString DlangOptionsPageWidget::phobosPath() const
{
    return m_phobos->path();
}

QPair<int, int> DlangOptionsPageWidget::portsRange() const
{
    return qMakePair(static_cast<int>(m_firstPort->value()), static_cast<int>(m_lastPort->value()));
}
