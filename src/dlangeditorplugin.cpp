#include "dlangeditorplugin.h"
#include "dlangeditorconstants.h"

#include "codemodel/dmodel.h"
#include "codemodel/dcdmodel.h"
#include "codemodel/dcdoptions.h"
#include "codemodel/dastedmodel.h"
#include "codemodel/dastedoptions.h"
#include "codemodel/dummymodel.h"
#include "dlangeditor.h"
#include "dlangeditorutils.h"
#include "dlangoptionspage.h"
#include "dlangoutline.h"
#include "dlangcompletionassistprovider.h"
#include "dlanghoverhandler.h"
#include "locator/dlanglocatorcurrentdocumentfilter.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>

#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>
#include <cpptools/cppmodelmanager.h>

#include <utils/mimetypes/mimedatabase.h>
#include <projectexplorer/projecttree.h>

#include <QtPlugin>

using namespace DlangEditor::Internal;
using namespace DlangEditor::Utils;
using namespace DlangEditor;

using namespace Core;
using MimeDatabase = ::Utils::MimeDatabase;

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

    MimeDatabase::addMimeTypes(QLatin1String(":/dlangeditor/DlangEditor.mimetypes.xml"));

    if (!configureDcdCodeModel(errorString)) {
        return false;
    }

    if (!configureDastedCodeModel(errorString)) {
        return false;
    }

    if (!configureDummyCodeModel(errorString)) {
        return false;
    }

    if (!DCodeModel::ModelManager::instance().setCurrentModel(DlangOptionsPage::codeModel(), errorString)) {
        if (!DCodeModel::ModelManager::instance().setCurrentModel(DCodeModel::DUMMY_MODEL,
                                                                  errorString)) {
            return false;
        }
    }

    connect(CppTools::CppModelManager::instance(),
            SIGNAL(projectPartsUpdated(ProjectExplorer::Project*)),
            this, SLOT(onImportPathsUpdate(ProjectExplorer::Project*)));

    connect(this, SIGNAL(projectImportsUpdated(QString,QStringList)),
            &(DCodeModel::ModelManager::instance()),
            SLOT(onImportPathsUpdate(QString,QStringList)));

    addAutoReleasedObject(new DlangOptionsPage);
    addAutoReleasedObject(new DlangEditorFactory);
    addAutoReleasedObject(new DlangCompletionAssistProvider);
    addAutoReleasedObject(new DlangOutlineWidgetFactory);

    // Locator
    addAutoReleasedObject(new DlangLocatorCurrentDocumentFilter);

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

void DlangEditorPlugin::onImportPathsUpdate(ProjectExplorer::Project *project)
{
    qDebug() << "onImportPathsUpdate";
    if (!(project->projectLanguages() == Core::Context("DLANG"))) {
        return;
    }
    QStringList list;
    CppTools::CppModelManager *modelmanager =
            CppTools::CppModelManager::instance();
    if (!modelmanager) {
        return;
    }
    QString projectName = project ? project->displayName()
                                  : QLatin1String("defaultDastedProjectName");
    if (project) {
        CppTools::ProjectInfo pinfo = modelmanager->projectInfo(project);
        if (pinfo.isValid()) {
            foreach (const CppTools::ProjectPart::HeaderPath &header, pinfo.headerPaths()) {
                if (header.isValid()) {
                    list.push_back(header.path);
                }
            }
        }
    }
    list.append(Dcd::DcdOptionsPage::includePaths());
    list.removeDuplicates();
    emit projectImportsUpdated(projectName, list);
}

bool DlangEditorPlugin::configureDcdCodeModel(QString *errorString)
{
    Dcd::Factory::instance().setPortRange(Dcd::DcdOptionsPage::portsRange());
    Dcd::Factory::instance().setProcessName(Dcd::DcdOptionsPage::dcdServerExecutable());
    Dcd::Factory::instance().setServerLog(Dcd::DcdOptionsPage::dcdServerLogPath());

    Dcd::Factory::instance().setNameGetter([]() {
        return currentProjectName(QLatin1String("DCD_default_project_name"));
    });

    Dcd::Factory::instance().setServerInitializer([](QSharedPointer<Dcd::Server> server) {
        // append include paths from project settings
        try {
            QStringList list;
            CppTools::CppModelManager *modelmanager =
                    CppTools::CppModelManager::instance();
            if (modelmanager) {
                ProjectExplorer::Project *currentProject = getCurrentProject();
                if (currentProject) {
                    CppTools::ProjectInfo pinfo = modelmanager->projectInfo(currentProject);
                    if (pinfo.isValid()) {
                        foreach (const CppTools::ProjectPart::HeaderPath &header, pinfo.headerPaths()) {
                            if (header.isValid()) {
                                list.push_back(header.path);
                            }
                        }
                    }
                }
            }
            list.append(Dcd::DcdOptionsPage::includePaths());
            list.removeDuplicates();
            Dcd::Client client(server->port());
            client.appendIncludePaths(currentProjectName(), list);
        } catch (...) {
            server->stop();
        }
    });

    return DCodeModel::ModelManager::instance().registerModelStorage(Dcd::DCD_CODEMODEL_ID, []() {
        return DCodeModel::IModelSharedPtr(new Dcd::Client(Dcd::Factory::instance().getPort()));
    }, []() {
        return new Dcd::DcdOptionsPageWidget;
    }, errorString);
}

bool DlangEditorPlugin::configureDastedCodeModel(QString *errorString)
{
    auto port = Dasted::DastedOptionsPage::port();
    auto processName = Dasted::DastedOptionsPage::dastedServerExecutable();
    auto serverLog = Dasted::DastedOptionsPage::dastedServerLogPath();
    bool serverAutoStart = Dasted::DastedOptionsPage::autoStart();
    auto processArguments = Dasted::DastedOptionsPage::dastedParameters();

    auto modelCreator = [this, port, serverAutoStart, processName]() {

        CppTools::CppModelManager *cppmodelmanager =
                CppTools::CppModelManager::instance();
        if (!cppmodelmanager) {
            return DCodeModel::IModelSharedPtr();
        }

        QSharedPointer<Dasted::DastedModel> model(
                    new Dasted::DastedModel(port, serverAutoStart, processName));

        connect(this, SIGNAL(projectImportsUpdated(QString,QStringList)),
                model.data(), SLOT(onImportPathsUpdate(QString,QStringList)));

        return DCodeModel::IModelSharedPtr(model);
    };

    return DCodeModel::ModelManager::instance().registerModelStorage(
                Dasted::DASTED_CODEMODEL_ID, modelCreator,
                []() {
        return new Dasted::DastedOptionsPageWidget;
    }, errorString);
}

bool DlangEditorPlugin::configureDummyCodeModel(QString *errorString)
{
    return DCodeModel::ModelManager::instance().registerModelStorage(
                DCodeModel::DUMMY_MODEL,
                []() {
        return DCodeModel::IModelSharedPtr(new DCodeModel::DummyModel);
    }, []() {
        return new DCodeModel::DummyModelOptionsPageWidget;
    }, errorString);
}

