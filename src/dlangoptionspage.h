#ifndef DLANGOPTIONSPAGE_H
#define DLANGOPTIONSPAGE_H

#include <coreplugin/dialogs/ioptionspage.h>

#include <QWidget>
#include <QPointer>
#include <QMap>

QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QVBoxLayout)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QStackedWidget)

namespace DCodeModel {
class IModelOptionsWidget;
}

namespace DlangEditor {

class DlangOptionsPageWidget : public QWidget
{
    Q_OBJECT
public:
    DlangOptionsPageWidget(QWidget *parent = 0);
    virtual ~DlangOptionsPageWidget();

    QString codeModelId() const;

    void apply();

public slots:
    void setModelWidget(const QString &modelId);
    void needRestart();
    void configuartionError(const QString &err);
    void resetModelToCurrent();
    void setModel(const QString &modelId);
private:
    void setModelWidgetThrow(const QString &modelId);
    DCodeModel::IModelOptionsWidget *modelWidget() const;

    QComboBox *m_codeModel;
    QVBoxLayout *m_mainLayout;
    QLabel *m_warningMessage;
    QStackedWidget *m_codeModelStack;
    QMap<QString, int> m_codeModelMap;
};

class DlangOptionsPage : public Core::IOptionsPage
{
public:
    DlangOptionsPage();
    virtual ~DlangOptionsPage();

    // pure Core::IOptionsPage
    virtual QWidget *widget() Q_DECL_OVERRIDE;
    virtual void apply() Q_DECL_OVERRIDE;
    virtual void finish() Q_DECL_OVERRIDE;

    // others
    static QString codeModel();
private:
    QPointer<DlangOptionsPageWidget> m_widget;
};

} // namespace DlangEditor

#endif // DLANGOPTIONSPAGE_H
