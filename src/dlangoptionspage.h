#ifndef DLANGOPTIONSPAGE_H
#define DLANGOPTIONSPAGE_H

#include <coreplugin/dialogs/ioptionspage.h>

#include <QWidget>
#include <QPointer>
#include <QPair>

namespace Utils {
class PathChooser;
class PathListEditor;
}

QT_FORWARD_DECLARE_CLASS(QSpinBox)
QT_FORWARD_DECLARE_CLASS(QCheckBox)

namespace DlangEditor {

class DlangOptionsPageWidget : public QWidget
{
    Q_OBJECT
public:
    DlangOptionsPageWidget(QWidget *parent = 0);
    virtual ~DlangOptionsPageWidget() Q_DECL_OVERRIDE;
    QString serverExecutable() const;
    QString serverLogPath() const;
    QStringList includePaths() const;
    QPair<int, int> portsRange() const;
    bool hoverEnable() const;
private:
    Utils::PathChooser *m_server;
    Utils::PathChooser *m_serverLog;
    Utils::PathListEditor *m_includes;
    QSpinBox *m_firstPort;
    QSpinBox *m_lastPort;
    QCheckBox *m_hoverEnable;
};

class DlangOptionsPage : public Core::IOptionsPage
{
public:
    DlangOptionsPage();
    virtual ~DlangOptionsPage() Q_DECL_OVERRIDE;

    // pure Core::IOptionsPage
    virtual QWidget *widget() Q_DECL_OVERRIDE;
    virtual void apply() Q_DECL_OVERRIDE;
    virtual void finish() Q_DECL_OVERRIDE;

    // others
    static QString dcdServerExecutable();
    static QString dcdServerLogPath();
    static QStringList includePaths();
    static QPair<int, int> portsRange();
    static bool hoverEnable();
private:
    QPointer<DlangOptionsPageWidget> m_widget;
};

} // namespace DlangEditor

#endif // DLANGOPTIONSPAGE_H
