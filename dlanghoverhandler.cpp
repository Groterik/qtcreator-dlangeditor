#include "dlanghoverhandler.h"

#include "dlangeditorconstants.h"
#include "dcdsupport.h"
#include "dlangassistprocessor.h"
#include "dlangeditor.h"

#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/idocument.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorer.h>
#include <texteditor/itexteditor.h>
#include <texteditor/basetexteditor.h>

using namespace DlangEditor;

DlangHoverHandler::DlangHoverHandler(QObject *parent) :
    TextEditor::BaseHoverHandler(parent)
{
}


bool DlangEditor::DlangHoverHandler::acceptEditor(Core::IEditor *editor)
{
    return editor->document()->id() == Constants::DLANG_EDITOR_ID;
}

void DlangHoverHandler::identifyMatch(TextEditor::ITextEditor *editor, int pos)
{
    if (!editor) {
        return;
    }
    const TextEditor::ITextEditorDocument* doc = qobject_cast<TextEditor::ITextEditorDocument*>(editor->document());
    if (!doc) {
        return;
    }
    if (pos != lastPos) {
        lastPos = pos;
        int begin, size;
        if (!getFullIdentifier(doc, pos, begin, size)) {
            return;
        }
        QString ident = doc->textAt(begin, size);
        if (ident != lastSymbol) {
            ProjectExplorer::Project *currentProject = ProjectExplorer::ProjectExplorerPlugin::currentProject();
            QString projectName = currentProject ? currentProject->displayName() : QString();
            try {
                Dcd::DcdClient *client = DcdFactory::instance()->client(projectName);
                if (!client) {
                    return;
                }
                QStringList res;
                client->getDocumentationComments(doc->plainText(), pos, res);
                if (!res.empty()) {
                    lastTooltip = res.front();
                } else {
                    lastTooltip.clear();
                }
            }
            catch (...) {
                lastTooltip.clear();
            }
            lastSymbol = ident;
        }

    }
    setToolTip(lastTooltip);
}

void DlangHoverHandler::decorateToolTip()
{

}
