#ifndef DASTEDOPTIONS_H
#define DASTEDOPTIONS_H

#include <QWidget>
#include <QString>
#include <QPair>

#include "codemodel/dmodeloptions.h"

namespace Utils {
class PathChooser;
class PathListEditor;
}

QT_FORWARD_DECLARE_CLASS(QSpinBox)
QT_FORWARD_DECLARE_CLASS(QCheckBox)

namespace Dasted {

class DastedOptionsPageWidget : public DCodeModel::IModelOptionsWidget
{
    Q_OBJECT
public:
    DastedOptionsPageWidget(QWidget *parent = 0);
    virtual ~DastedOptionsPageWidget();

    QString serverExecutable() const;
    QString serverLogPath() const;
    QStringList includePaths() const;
    int port() const;
    bool autoStart() const;

    // pure virtual
    void apply() Q_DECL_OVERRIDE;

private:
    Utils::PathChooser *m_server;
    Utils::PathChooser *m_serverLog;
    Utils::PathListEditor *m_includes;
    QCheckBox *m_autoStart;
    QSpinBox *m_port;
};

class DastedOptionsPage
{
public:
    static QString dastedServerExecutable();
    static QString dastedServerLogPath();
    static QStringList includePaths();
    static int port();
    static bool autoStart();
};

} // namespace Dcd

#endif // DASTEDOPTIONS_H
