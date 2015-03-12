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
#include <QStackedWidget>
#include <QLabel>

using namespace DlangEditor;

namespace {
const QString S_CODEMODEL_SETTINGS = QLatin1String("DlangCodeModelSettings");
const char S_CODEMODEL_NAME[] = "dlangCodeModel";
}

DlangOptionsPageWidget::DlangOptionsPageWidget(QWidget *parent)
    : QWidget(parent)
{
    m_mainLayout = new QVBoxLayout;
    QFormLayout *modelFormLayout = new QFormLayout;
    this->setLayout(m_mainLayout);

    QHBoxLayout *modelLayout = new QHBoxLayout;
    modelFormLayout->addRow("Code model", modelLayout);

    m_codeModel = new QComboBox;
    m_codeModel->addItems(DCodeModel::Factory::instance().modelIds());
    modelLayout->addWidget(m_codeModel, 1);

    m_mainLayout->addLayout(modelFormLayout);

    m_codeModelStack = new QStackedWidget;
    m_mainLayout->addWidget(m_codeModelStack);

    resetModelToCurrent();

    m_warningMessage = new QLabel;
    m_warningMessage->setVisible(false);
    m_mainLayout->addWidget(m_warningMessage, 0, Qt::AlignBottom);

    connect(m_codeModel, &QComboBox::currentTextChanged, [=](){
        setModelWidget(m_codeModel->currentText());
    });
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
    if (modelWidget()) {
        DCodeModel::Factory::instance().setCurrentModel(m_codeModel->currentText(), 0);
        modelWidget()->apply();
    }
}

void DlangOptionsPageWidget::setModelWidget(const QString &modelId)
{
    try {
        setModelWidgetThrow(modelId);
    } catch (const std::exception& ex) {
        resetModelToCurrent();
        configuartionError(QLatin1String(ex.what()));
    } catch (...) {
        resetModelToCurrent();
        configuartionError(QLatin1String("unknown error"));
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

void DlangOptionsPageWidget::resetModelToCurrent()
{
    const QString model = DCodeModel::Factory::instance().currentModelId();
    m_codeModel->setCurrentText(model);
    try {
        setModelWidgetThrow(model);
    } catch (...) {
    }
}

void DlangOptionsPageWidget::setModel(const QString &modelId)
{
    QString err;
    if (!DCodeModel::Factory::instance().setCurrentModel(modelId, &err)) {
        configuartionError(err);
        return;
    }
    m_codeModel->setCurrentText(modelId);
}

void DlangOptionsPageWidget::setModelWidgetThrow(const QString &modelId)
{
    auto it = m_codeModelMap.find(modelId);
    if (it != m_codeModelMap.end()) {
        auto w = m_codeModelStack->widget(it.value());
        if (!w) {
            throw std::runtime_error("bad model index");
        }
        m_codeModelStack->setCurrentWidget(w);
        return;
    }
    auto ms = DCodeModel::Factory::instance().modelStorage(modelId);
    if (!ms) {
        throw std::runtime_error("bad model storage");
    }
    auto w = ms->widget();
    if (!w) {
        throw std::runtime_error("bad model options page");
    }
    auto ind = m_codeModelStack->addWidget(w);
    m_codeModelMap.insert(modelId, ind);
    m_codeModelStack->setCurrentWidget(w);

    connect(w, SIGNAL(updatedAndNeedRestart()), this, SLOT(needRestart()));
}

DCodeModel::IModelOptionsWidget *DlangOptionsPageWidget::modelWidget() const
{
    auto w = m_codeModelStack->currentWidget();
    if (!w) {
        return 0;
    }
    return qobject_cast<DCodeModel::IModelOptionsWidget*>(w);
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
