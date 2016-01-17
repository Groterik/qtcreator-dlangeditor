#include "dlangeditorutils.h"

#include <projectexplorer/project.h>
#include <projectexplorer/projecttree.h>

using namespace DlangEditor::Utils;

ProjectExplorer::Project *DlangEditor::Utils::getCurrentProject()
{
    return ProjectExplorer::ProjectTree::currentProject();
}

QString DlangEditor::Utils::currentProjectName(const QString &defaultValue)
{
    auto project = getCurrentProject();
    return project ? project->displayName() : defaultValue;
}
