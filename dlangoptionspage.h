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

namespace DlangEditor {

class DlangOptionsPageWidget : public QWidget
{
    Q_OBJECT
public:
    DlangOptionsPageWidget(QWidget *parent = 0);
    virtual ~DlangOptionsPageWidget();
    QString clientExecutable() const;
    QString serverExecutable() const;
    QString serverLogPath() const;
    QStringList includePaths() const;
    QPair<int, int> portsRange() const;
private:
    Utils::PathChooser *m_client;
    Utils::PathChooser *m_server;
    Utils::PathChooser *m_serverLog;
    Utils::PathListEditor *m_includes;
    QSpinBox *m_firstPort;
    QSpinBox *m_lastPort;
};

class DlangOptionsPage : public Core::IOptionsPage
{
public:
    DlangOptionsPage();
    virtual ~DlangOptionsPage();

    // pure Core::IOptionsPage
    virtual QWidget *widget();
    virtual void apply();
    virtual void finish();

    // others
    static QString dcdClientExecutable();
    static QString dcdServerExecutable();
    static QString dcdServerLogPath();
    static QStringList includePaths();
    static QPair<int, int> portsRange();
private:
    QPointer<DlangOptionsPageWidget> m_widget;
};

} // namespace DlangEditor

#endif // DLANGOPTIONSPAGE_H
