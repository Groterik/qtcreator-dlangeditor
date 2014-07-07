#ifndef DLANGOPTIONSPAGE_H
#define DLANGOPTIONSPAGE_H

#include <coreplugin/dialogs/ioptionspage.h>

#include <QWidget>
#include <QPointer>

namespace Utils {
class PathChooser;
}

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
private:
    Utils::PathChooser *m_client;
    Utils::PathChooser *m_server;
    Utils::PathChooser *m_phobos;
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
private:
    QPointer<DlangOptionsPageWidget> m_widget;
};

}

#endif // DLANGOPTIONSPAGE_H
