#ifndef DLANGOPTIONSPAGE_H
#define DLANGOPTIONSPAGE_H

#include <coreplugin/dialogs/ioptionspage.h>

#include <QWidget>
#include <QPointer>
#include <QPair>

QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QVBoxLayout)

namespace DlangEditor {

class DlangOptionsPageWidget : public QWidget
{
    Q_OBJECT
public:
    DlangOptionsPageWidget(QWidget *parent = 0);
    virtual ~DlangOptionsPageWidget();

    QString codeModelId() const;

public slots:
    void setModelWidget(const QString modelId);
private:
    QComboBox *m_codeModel;
    QPushButton *m_codeModelApply;
    QPushButton *m_codeModelCancel;
    QWidget *m_codeModelWidget;
    QVBoxLayout *mainLayout;
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
