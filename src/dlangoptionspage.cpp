#include "dlangoptionspage.h"

#include "dlangeditorconstants.h"
#include "codemodel/dmodeloptions.h"
#include "codemodel/dmodel.h"

#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>

#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>

using namespace DlangEditor;

namespace {
const QString S_CODEMODEL_SETTINGS = QLatin1String("DlangCodeModelSettings");
const char S_CODEMODEL_NAME[] = "dlangCodeModel";
}

DlangOptionsPageWidget::DlangOptionsPageWidget(QWidget *parent)
    : QWidget(parent), m_codeModelWidget(0)
{
    m_mainLayout = new QVBoxLayout;
    QFormLayout *modelFormLayout = new QFormLayout;
    this->setLayout(m_mainLayout);

    QHBoxLayout *modelLayout = new QHBoxLayout;
    modelFormLayout->addRow("Code model", modelLayout);

    m_codeModel = new QComboBox;
    m_codeModel->addItems(DCodeModel::Factory::instance().modelIds());
    m_codeModelApply = new QPushButton("Apply");
    m_codeModelApply->setEnabled(false);
    m_codeModelCancel = new QPushButton("Cancel");
    m_codeModelCancel->setEnabled(false);
    modelLayout->addWidget(m_codeModel, 1);
    modelLayout->addWidget(m_codeModelApply);
    modelLayout->addWidget(m_codeModelCancel);

    m_mainLayout->addLayout(modelFormLayout);

    m_codeModelLayout = new QVBoxLayout;
    m_mainLayout->addLayout(m_codeModelLayout);

    const QString model = DCodeModel::Factory::instance().currentModelId();
    m_codeModel->setCurrentText(model);

    setModelWidget(model);

    m_warningMessage = new QLabel;
    m_warningMessage->setVisible(false);
    m_mainLayout->addWidget(m_warningMessage, 0, Qt::AlignBottom);
}

DlangOptionsPageWidget::~DlangOptionsPageWidget()
{

}

QString DlangOptionsPageWidget::codeModelId() const
{
    return m_codeModel->currentText();
}

void DlangOptionsPageWidget::apply()
{
    if (m_codeModelWidget) {
        m_codeModelWidget->apply();
    }
}

void DlangOptionsPageWidget::setModelWidget(const QString modelId)
{
    try {
        auto ms = DCodeModel::Factory::instance().modelStorage(modelId);
        if (m_codeModelWidget) {
            disconnect(m_codeModelWidget, SIGNAL(updatedAndNeedRestart()), this, SLOT(needRestart()));
            m_codeModelLayout->removeWidget(m_codeModelWidget);
        }
        if (ms) {
            m_codeModelWidget = ms->widget();
            connect(m_codeModelWidget, SIGNAL(updatedAndNeedRestart()), this, SLOT(needRestart()));
        }
        if (!m_codeModelWidget) {
            throw std::runtime_error("bad model settings widget");
        }

        m_codeModelLayout->addWidget(m_codeModelWidget);

    } catch (const std::exception& ex) {
        m_codeModel->setCurrentText("");
        m_codeModelWidget = 0;
        configuartionError(QLatin1String(ex.what()));
    }
}

static inline QString imageString(const char *img)
{
    return QLatin1String("<img src=\'") +
           QLatin1String(img) +
           QLatin1String("\'>  ");
}

void DlangOptionsPageWidget::needRestart()
{
    m_warningMessage->setText(imageString(Core::Constants::ICON_WARNING) +
                              QLatin1String("Some options need to restart QtCreator"));
    m_warningMessage->setVisible(true);
}

void DlangOptionsPageWidget::configuartionError(const QString &err)
{
    m_warningMessage->setText(imageString(Core::Constants::ICON_ERROR) + err);
    m_warningMessage->setVisible(!err.isEmpty());
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
        m_widget->apply();
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
