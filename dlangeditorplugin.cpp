#include "dlangeditorplugin.h"
#include "dlangeditorconstants.h"

#include "dlangeditor.h"
#include "dlangcompletionassistprovider.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>

#include <coreplugin/mimedatabase.h>

#include <QMainWindow>

#include <QtPlugin>

using namespace DlangEditor::Internal;

DlangEditorPlugin::DlangEditorPlugin()
{
    // Create your members
}

DlangEditorPlugin::~DlangEditorPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
}

bool DlangEditorPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)

    if (!Core::MimeDatabase::addMimeTypes(QLatin1String(":/dlangeditor/DlangEditor.mimetypes.xml"), errorString))
        return false;

    addAutoReleasedObject(new DlangEditorFactory(this));
    addAutoReleasedObject(new DlangCompletionAssistProvider);

    return true;
}

void DlangEditorPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
}

ExtensionSystem::IPlugin::ShutdownFlag DlangEditorPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    return SynchronousShutdown;
}

Q_EXPORT_PLUGIN2(DlangEditor, DlangEditorPlugin)

