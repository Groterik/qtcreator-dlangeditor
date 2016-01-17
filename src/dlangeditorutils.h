#ifndef DLANGEDITORUTILS_H
#define DLANGEDITORUTILS_H

#include <QString>

namespace ProjectExplorer {
class Project;
} // namespace ProjectExplorer

namespace DlangEditor {
namespace Utils {

ProjectExplorer::Project *getCurrentProject();

QString currentProjectName(const QString &defaultValue = QString());

} // namespace Utils
} // namespace DlangEditor

#endif // DLANGEDITORUTILS_H
