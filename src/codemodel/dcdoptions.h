#ifndef DCDOPTIONS_H
#define DCDOPTIONS_H

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

namespace Dcd {

class DcdOptionsPageWidget : public DCodeModel::IModelOptionsWidget
{
    Q_OBJECT
public:
    DcdOptionsPageWidget(QWidget *parent = 0);
    virtual ~DcdOptionsPageWidget();

    QString serverExecutable() const;
    QString serverLogPath() const;
    QStringList includePaths() const;
    QPair<int, int> portsRange() const;
    bool hoverEnable() const;

    // pure virtual
    void apply() Q_DECL_OVERRIDE;

private:
    Utils::PathChooser *m_server;
    Utils::PathChooser *m_serverLog;
    Utils::PathListEditor *m_includes;
    QSpinBox *m_firstPort;
    QSpinBox *m_lastPort;
    QCheckBox *m_hoverEnable;
};

class DcdOptionsPage
{
public:
    static QString dcdServerExecutable();
    static QString dcdServerLogPath();
    static QStringList includePaths();
    static QPair<int, int> portsRange();
    static bool hoverEnable();
};

} // namespace Dcd

#endif // DCDOPTIONS_H
