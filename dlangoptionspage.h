#ifndef DLANGOPTIONSPAGE_H
#define DLANGOPTIONSPAGE_H

#include <coreplugin/dialogs/ioptionspage.h>

#include <QWidget>
#include <QPointer>
#include <QPair>

namespace Utils {
class PathChooser;
}

QT_FORWARD_DECLARE_CLASS(QDoubleSpinBox)

namespace DlangEditor {

class DlangOptionsPageWidget : public QWidget
{
    Q_OBJECT
public:
    DlangOptionsPageWidget(QWidget *parent = 0);
    virtual ~DlangOptionsPageWidget();
    QString clientExecutable() const;
    QString serverExecutable() const;
    QString phobosPath() const;
    QPair<int, int> portsRange() const;
private:
    Utils::PathChooser *m_client;
    Utils::PathChooser *m_server;
    Utils::PathChooser *m_phobos;
    QDoubleSpinBox *m_firstPort;
    QDoubleSpinBox *m_lastPort;
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
    static QString phobosDir();
    static QPair<int, int> portsRange();
private:
    QPointer<DlangOptionsPageWidget> m_widget;
};

}

#endif // DLANGOPTIONSPAGE_H
