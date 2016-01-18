#ifndef DLANGEDITOR_H
#define DLANGEDITOR_H

#include "dlangeditor_global.h"

#include <extensionsystem/iplugin.h>

namespace ProjectExplorer {
class Project;
}

namespace DlangEditor {
namespace Internal {

class DlangEditorPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "DlangEditor.json")

public:
    DlangEditorPlugin();
    ~DlangEditorPlugin();

    bool initialize(const QStringList &arguments, QString *errorString) Q_DECL_OVERRIDE;
    void extensionsInitialized() Q_DECL_OVERRIDE;
    ShutdownFlag aboutToShutdown();
signals:
    void projectImportsUpdated(QString projectName, QStringList imports);

private slots:
    void onImportPathsUpdate(ProjectExplorer::Project *project);
private:
    bool configureDcdCodeModel(QString *errorString);
    bool configureDastedCodeModel(QString *errorString);
    bool configureDummyCodeModel(QString *errorString);
};

} // namespace Internal
} // namespace DlangEditor

#endif // DLANGEDITOR_H

