#ifndef DLANGDEBUGHELPER_H
#define DLANGDEBUGHELPER_H

#include <QString>
#include <QElapsedTimer>

namespace DlangEditor {
class DlangDebugHelper
{
public:
    DlangDebugHelper(const char* func, const char* str);
    ~DlangDebugHelper();
private:
    QString str;
    QString func;
    QElapsedTimer timer;
};
} // namespace

#ifndef NDEBUG
#define DEBUG_GUARD(s) DlangEditor::DlangDebugHelper _guard_func(Q_FUNC_INFO, s)
#else
#define DEBUG_GUARD(s)
#endif

#endif // DLANGDEBUGHELPER_H
