#include "dlangoptionspage.h"

#include "dlangeditorconstants.h"
#include "codemodel/dmodel.h"

#include <coreplugin/icore.h>

#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
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
    : QWidget(parent), m_codeModelWidget(0)
{
    mainLayout = new QVBoxLayout;
    QFormLayout *modelFormLayout = new QFormLayout;
    this->setLayout(mainLayout);

    QHBoxLayout *modelLayout = new QHBoxLayout;
    modelFormLayout->addRow("Code model", modelLayout);

    m_codeModel = new QComboBox;
    m_codeModel->addItems(DCodeModel::Factory::instance().modelIds());
    m_codeModelApply = new QPushButton("Apply");
    m_codeModelCancel = new QPushButton("Cancel");
    modelLayout->addWidget(m_codeModel, 1);
    modelLayout->addWidget(m_codeModelApply);
    modelLayout->addWidget(m_codeModelCancel);

    mainLayout->addLayout(modelFormLayout);

    const QString model = DCodeModel::Factory::instance().currentModelId();
    m_codeModel->setCurrentText(model);

    setModelWidget(model);

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

void DlangOptionsPageWidget::setModelWidget(const QString modelId)
{
    try {
        auto ms = DCodeModel::Factory::instance().modelStorage(modelId);
        if (m_codeModelWidget) {
            mainLayout->removeWidget(m_codeModelWidget);
        }
        if (ms) {
            m_codeModelWidget = ms->widget();
        }
        if (!m_codeModelWidget) {
            throw std::runtime_error("bad model settings widget");
        }

        mainLayout->addWidget(m_codeModelWidget);

    } catch (...) {
        m_codeModel->setCurrentText("");
        m_codeModelWidget = 0;
    }
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
