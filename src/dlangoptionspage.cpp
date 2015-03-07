#include "dlangoptionspage.h"

#include "dlangeditorconstants.h"
#include "codemodel/dmodel.h"

#include <coreplugin/icore.h>

#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

using namespace DlangEditor;

namespace {
const QString S_CODEMODEL_SETTINGS = QLatin1String("DlangCodeModelSettings");
const char S_CODEMODEL_NAME[] = "dlangCodeModel";
const char S_DCD_SERVER[] = "dcdServerExecutable";
const char S_DCD_SERVER_LOG[] = "dcdServerLog";
const char S_DCD_PORTS_FIRST[] = "dcdServerPortsRangeFirst";
const char S_DCD_PORTS_LAST[] = "dcdServerPortsRangeLast";
const char S_HOVER_ENABLE[] = "hoverEnable";
}

DlangOptionsPageWidget::DlangOptionsPageWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QHBoxLayout *modelLayout = new QHBoxLayout;
    mainLayout->addLayout(modelLayout);

    m_codeModel = new QComboBox;
    m_codeModelApply = new QPushButton("Apply");
    m_codeModelCancel = new QPushButton("Cancel");
    QLabel *codeModelLabel = new QLabel("Code model");
    modelLayout->addWidget(codeModelLabel);
    modelLayout->addWidget(m_codeModel);
    modelLayout->addWidget(m_codeModelApply);
    modelLayout->addWidget(m_codeModelCancel);

    connect(&(DCodeModel::Factory::instance()), &DCodeModel::Factory::updated, this, [=]() {
        m_codeModel->clear();
        m_codeModel->addItems(DCodeModel::Factory::instance().modelIds());
    });
}

DlangOptionsPageWidget::~DlangOptionsPageWidget()
{

}

QString DlangOptionsPageWidget::codeModelId() const
{
    return m_codeModel->currentText();
}

DlangOptionsPage::DlangOptionsPage()
{
    setId(Constants::DLANG_CODE_MODEL_SETTINGS_ID);
    setDisplayName(Constants::DLANG_CODE_MODEL_SETTINGS_NAME);
    setCategory(Constants::DLANG_SETTINGS_CATEGORY_ID);
    setDisplayCategory(Constants::DLANG_SETTINGS_CATEGORY_NAME);
    setCategoryIcon(QLatin1String(Constants::DLANG_SETTINGS_CATEGORY_ICON));
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
        settings->beginGroup(S_CODEMODEL_SETTINGS);
        settings->setValue(QLatin1String(S_CODEMODEL_NAME), m_widget->codeModelId());
        settings->endGroup();
    }
}

void DlangOptionsPage::finish()
{
    delete m_widget;
}

QString DlangOptionsPage::codeModel()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(S_CODEMODEL_SETTINGS);
    QString result = settings->value(QLatin1String(S_CODEMODEL_NAME), QLatin1String("")).toString();
    settings->endGroup();
    return result;
}
