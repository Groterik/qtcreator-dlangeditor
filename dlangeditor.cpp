#include "dlangeditor.h"

#include "dlangeditorconstants.h"
#include "dlangindenter.h"
#include "dlangautocompleter.h"
#include "dlangcompletionassistprovider.h"

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


DlangEditorFactory::DlangEditorFactory(QObject *parent)
    : Core::IEditorFactory(parent)
{

    setId(DlangEditor::Constants::DLANG_EDITOR_ID);
    setDisplayName(tr(DlangEditor::Constants::DLANG_EDITOR_DISPLAY_NAME));
    addMimeType(DlangEditor::Constants::DLANG_MIMETYPE);

    new TextEditor::TextEditorActionHandler(this, DlangEditor::Constants::DLANG_EDITOR_CONTEXT_ID,
            TextEditor::TextEditorActionHandler::UnCommentSelection
            | TextEditor::TextEditorActionHandler::JumpToFileUnderCursor);

    Core::ActionContainer *contextMenu =
            Core::ActionManager::createMenu(DlangEditor::Constants::DLANG_EDITOR_CONTEXT_MENU);
    Core::Command *cmd;
    Core::Context dlangEditorContext = Core::Context(DlangEditor::Constants::DLANG_EDITOR_CONTEXT_ID);

    cmd = Core::ActionManager::command(TextEditor::Constants::JUMP_TO_FILE_UNDER_CURSOR);
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
