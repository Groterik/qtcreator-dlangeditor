#include "dlangeditor.h"

#include "dlangeditorconstants.h"
#include "dlangindenter.h"
#include "dlangautocompleter.h"
#include "dlangcompletionassistprovider.h"
#include "dlangassistprocessor.h"
#include "dlanghoverhandler.h"
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

inline bool isFullIdentifierChar(const QChar& c)
{
    return !c.isNull() && (c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.'));
}

bool DlangEditor::getFullIdentifier(const TextEditor::TextDocument *doc, int pos, int &begin, int &size)
{
    QChar c;
    begin = pos;
    do {
        c = doc->characterAt(begin--);
    } while (isFullIdentifierChar(c));
    begin += 2;
    if (begin == pos) {
        return false;
    }
    int end = pos + 1;
    do {
        c = doc->characterAt(end++);
    } while (isFullIdentifierChar(c));

    size = end - begin - 1;
    return size != 0;
}

DlangTextEditor::DlangTextEditor() :
    TextEditor::BaseTextEditor()
{
    addContext(DlangEditor::Constants::DLANG_EDITOR_CONTEXT_ID);
    setDuplicateSupported(true);
}

TextEditor::CompletionAssistProvider *DlangTextEditor::completionAssistProvider()
{
    return ExtensionSystem::PluginManager::getObject<DlangCompletionAssistProvider>();
}

QString DlangTextEditor::contextHelpId() const
{
    int pos = position();
    const TextEditor::TextDocument* doc = const_cast<DlangTextEditor*>(this)->textDocument();
    int begin, size;
    return getFullIdentifier(doc, pos, begin, size) ? QLatin1String("D/") + doc->textAt(begin, size) : QString();
}

DlangTextEditorWidget::DlangTextEditorWidget(QWidget *parent)
    : TextEditor::TextEditorWidget(parent)
{
    setParenthesesMatchingEnabled(true);
    setCodeFoldingSupported(true);
}

void DlangTextEditorWidget::unCommentSelection()
{
    Utils::unCommentSelection(this);
}

void DlangTextEditorWidget::contextMenuEvent(QContextMenuEvent *e)
{
    showDefaultContextMenu(e, DlangEditor::Constants::DLANG_EDITOR_CONTEXT_MENU);
}

TextEditor::TextEditorWidget::Link DlangTextEditorWidget::findLinkAt(const QTextCursor &c, bool resolveTarget, bool inNextSplit)
{
    if (!resolveTarget) {
        return Link();
    }
    Q_UNUSED(inNextSplit);
    ProjectExplorer::Project *currentProject = ProjectExplorer::ProjectExplorerPlugin::currentProject();
    QString projectName = currentProject ? currentProject->displayName() : QString();
    Dcd::DcdFactory::ClientPointer client = Dcd::DcdFactory::instance()->client(projectName);
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
        loc.filename = textDocument()->filePath();
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


DlangEditorFactory::DlangEditorFactory()
    : TextEditor::TextEditorFactory()
{

    setId(DlangEditor::Constants::DLANG_EDITOR_ID);
    setDisplayName(tr(DlangEditor::Constants::DLANG_EDITOR_DISPLAY_NAME));
    addMimeType(DlangEditor::Constants::DLANG_MIMETYPE);

    setDocumentCreator([]() { return new DlangDocument; });
    setEditorWidgetCreator([]() { return new DlangTextEditorWidget; });
    setEditorCreator([]() { return new DlangTextEditor; });

    setIndenterCreator([]() { return new DlangIndenter; });
    setAutoCompleterCreator([]() { return new DlangAutoCompleter; });

    setCompletionAssistProvider(new DlangCompletionAssistProvider);

    setParenthesesMatchingEnabled(true);
    setMarksVisible(true);
    setCodeFoldingSupported(true);

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


DlangDocument::DlangDocument()
    : TextEditor::TextDocument()
{
    setId(Constants::DLANG_EDITOR_ID);
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
