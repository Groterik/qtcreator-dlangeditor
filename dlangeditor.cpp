#include "dlangeditor.h"

#include "dlangeditorconstants.h"
#include "dlangindenter.h"
#include "dlangautocompleter.h"
#include "dlangcompletionassistprovider.h"
#include "dlangassistprocessor.h"
#include "dcdsupport.h"

#include <texteditor/texteditorsettings.h>
#include <utils/uncommentselection.h>

#include <coreplugin/mimedatabase.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/icontext.h>
#include <texteditor/texteditoractionhandler.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/highlighterutils.h>
#include <extensionsystem/pluginmanager.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>

#include <QTextBlock>
#include <QElapsedTimer>
#include <QDebug>

using namespace DlangEditor;

DlangTextEditor::DlangTextEditor(DlangTextEditorWidget *parent) :
    TextEditor::BaseTextEditor(parent)
{
    setId(DlangEditor::Constants::DLANG_EDITOR_ID);
    setContext(Core::Context(DlangEditor::Constants::DLANG_EDITOR_CONTEXT_ID,
              TextEditor::Constants::C_TEXTEDITOR));
}

bool DlangTextEditor::duplicateSupported() const
{
    return true;
}

Core::IEditor *DlangTextEditor::duplicate()
{
    DlangTextEditorWidget *result = new DlangTextEditorWidget(qobject_cast<DlangTextEditorWidget *>(editorWidget()));
    TextEditor::TextEditorSettings::initializeEditor(result);
    return result->editor();
}

TextEditor::CompletionAssistProvider *DlangTextEditor::completionAssistProvider()
{
    return ExtensionSystem::PluginManager::getObject<DlangCompletionAssistProvider>();
}

DlangTextEditorWidget::DlangTextEditorWidget(QWidget *parent)
    : TextEditor::BaseTextEditorWidget(new DlangDocument, parent)
{
    setAutoCompleter(new DlangAutoCompleter);
    setParenthesesMatchingEnabled(true);
    setCodeFoldingSupported(true);
}

TextEditor::BaseTextEditor *DlangTextEditorWidget::createEditor()
{
    return new DlangTextEditor(this);
}

void DlangTextEditorWidget::unCommentSelection()
{
    Utils::unCommentSelection(this);
}

void DlangTextEditorWidget::contextMenuEvent(QContextMenuEvent *e)
{
    showDefaultContextMenu(e, DlangEditor::Constants::DLANG_EDITOR_CONTEXT_MENU);
}

TextEditor::BaseTextEditorWidget::Link DlangTextEditorWidget::findLinkAt(const QTextCursor &c, bool resolveTarget, bool inNextSplit)
{
    if (!resolveTarget) {
        return Link();
    }
    Q_UNUSED(inNextSplit);
    ProjectExplorer::Project *currentProject = ProjectExplorer::ProjectExplorerPlugin::currentProject();
    QString projectName = currentProject ? currentProject->displayName() : QString();
    Dcd::DcdClient *client = DcdFactory::instance()->client(projectName);
    if (!client) {
        return Link();
    }
    Dcd::DcdClient::Location loc;
    try {
        client->findSymbolLocation(this->document()->toPlainText(), c.position(), loc);
    }
    catch (...) {
        return Link();
    }
    if (loc.isNull()) {
        return Link();
    }

    if (loc.filename == "stdin") {
        loc.filename = baseTextDocument()->filePath();
    }
    QElapsedTimer timer;
    timer.start();
    QFile f(loc.filename);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return Link();
    }
    int lines = 0;
    int linePos = 0;
    for (int i = 0; i < loc.position; ) {
        char buff[1024];
        qint64 b = f.read(buff, std::min(1024, loc.position - i));
        if (!b) break;
        for (int j = 0; j < b; ++j) {
            if (buff[j] == '\n') {
                ++lines;
                linePos = i + j;
            }
        }
        i += b;
    }
    qDebug() << "Line " << timer.elapsed();
    return Link(loc.filename , lines + 1, loc.position - linePos - 1);
}


DlangEditorFactory::DlangEditorFactory(QObject *parent)
    : Core::IEditorFactory(parent)
{

    setId(DlangEditor::Constants::DLANG_EDITOR_ID);
    setDisplayName(tr(DlangEditor::Constants::DLANG_EDITOR_DISPLAY_NAME));
    addMimeType(DlangEditor::Constants::DLANG_MIMETYPE);

    new TextEditor::TextEditorActionHandler(this, DlangEditor::Constants::DLANG_EDITOR_CONTEXT_ID,
            TextEditor::TextEditorActionHandler::UnCommentSelection
            | TextEditor::TextEditorActionHandler::FollowSymbolUnderCursor);

    Core::ActionContainer *contextMenu =
            Core::ActionManager::createMenu(DlangEditor::Constants::DLANG_EDITOR_CONTEXT_MENU);
    Core::Command *cmd;
    Core::Context dlangEditorContext = Core::Context(DlangEditor::Constants::DLANG_EDITOR_CONTEXT_ID);

    cmd = Core::ActionManager::command(TextEditor::Constants::FOLLOW_SYMBOL_UNDER_CURSOR);
    contextMenu->addAction(cmd);

    contextMenu->addSeparator(dlangEditorContext);

    cmd = Core::ActionManager::command(TextEditor::Constants::UN_COMMENT_SELECTION);
    contextMenu->addAction(cmd);
}

Core::IEditor *DlangEditorFactory::createEditor()
{
    DlangTextEditorWidget *rc = new DlangTextEditorWidget();
    TextEditor::TextEditorSettings::initializeEditor(rc);
    return rc->editor();
}


DlangDocument::DlangDocument()
    : TextEditor::BaseTextDocument()
{
    setMimeType(DlangEditor::Constants::DLANG_MIMETYPE);
    setSyntaxHighlighter(TextEditor::createGenericSyntaxHighlighter(Core::MimeDatabase::findByType(mimeType())));
    setIndenter(new DlangIndenter);
}

QString DlangDocument::defaultPath() const
{
    QFileInfo fi(filePath());
    return fi.absolutePath();
}

QString DlangDocument::suggestedFileName() const
{
    QFileInfo fi(filePath());
    return fi.fileName();
}
