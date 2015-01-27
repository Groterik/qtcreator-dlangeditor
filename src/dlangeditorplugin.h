#ifndef DLANGEDITOR_H
#define DLANGEDITOR_H

#include "dlangeditor_global.h"

#include <extensionsystem/iplugin.h>

namespace DlangEditor {
namespace Internal {

class DlangEditorPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "DlangEditor.json")

public:
    DlangEditorPlugin();
    ~DlangEditorPlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    ShutdownFlag aboutToShutdown();

private slots:
private:
    void setDcdConfiguration();
};

} // namespace Internal
} // namespace DlangEditor

#endif // DLANGEDITOR_H

