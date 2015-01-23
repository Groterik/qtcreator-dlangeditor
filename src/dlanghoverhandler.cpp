#include "dlanghoverhandler.h"

#include "dlangeditorconstants.h"
#include "dcdsupport.h"
#include "dlangassistprocessor.h"
#include "dlangeditor.h"

#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/idocument.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorer.h>
#include <texteditor/texteditor.h>

using namespace DlangEditor;

DlangHoverHandler::DlangHoverHandler(QObject */*parent*/) :
    TextEditor::BaseHoverHandler()
{
}

void DlangHoverHandler::identifyMatch(TextEditor::TextEditorWidget *editor, int pos)
{
    if (!editor) {
        return;
    }
    const TextEditor::TextDocument* doc = editor->textDocument();
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
                Dcd::DcdFactory::ClientPointer client = Dcd::DcdFactory::instance()->client(projectName);
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

static const QString DLANG_DOC_META = QLatin1String("$(D");

void DlangHoverHandler::decorateToolTip()
{
    QString tip = toolTip();
    if (tip.length() > 1000) {
        tip.resize(1000);
        tip.append("...");
    }
    int pos = 0;
    while ((pos = tip.indexOf(DLANG_DOC_META, pos)) != -1) {
        int balance = 1;
        for (int end = pos + DLANG_DOC_META.length();  end < tip.length(); ++end) {
            if (tip.at(end) == QLatin1Char('(')) {
                ++balance;
            } else if (tip.at(end) == QLatin1Char(')')) {
                --balance;
                if (balance == 0) {
                    tip.replace(end, 1, QLatin1String("</b>"));
                    tip.replace(pos, DLANG_DOC_META.length(), QLatin1String("<b>"));
                    break;
                }
            }
        }
        if (balance > 0) {
            tip.replace(pos, DLANG_DOC_META.length(), QLatin1String("<b>"));
            tip.append(QLatin1String("</b>"));
        }
        ++pos;
    }
    tip.replace(QLatin1String("\\n"), QLatin1String("\n"));
    setToolTip(tip);
}
