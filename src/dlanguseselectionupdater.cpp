#include "dlanguseselectionupdater.h"
#include "dlangeditor.h"
#include "dcdsupport.h"

#include <QtConcurrent>

#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/fontsettings.h>


using namespace DlangEditor;

enum { updateUseSelectionsInternalInMs = 1000 };

DlangUseSelectionUpdater::DlangUseSelectionUpdater(DlangTextEditorWidget *editor)
    : m_editorWidget(editor)
{
    m_timer.setSingleShot(true);
    m_timer.setInterval(updateUseSelectionsInternalInMs);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));
}

DlangUseSelectionUpdater::~DlangUseSelectionUpdater()
{

}

void DlangUseSelectionUpdater::scheduleUpdate()
{
    m_timer.start();
}

void DlangUseSelectionUpdater::abortSchedule()
{
    m_timer.stop();
}

void DlangUseSelectionUpdater::update(DlangUseSelectionUpdater::CallType callType)
{
    if (callType == Synchronous) {
        updateSynchronously();
    } else updateAsynchronously();
}

void DlangUseSelectionUpdater::onFindUsesFinished()
{
    if (!m_findUsesWatcher) {
        return;
    }
    if (m_findUsesWatcher->isCanceled())
        return;
    processResults(m_findUsesWatcher->result());

    m_findUsesWatcher.reset();
}

struct Params
{
    QTextDocument* docPtr;
    QString docPath;
    int pos;
    int rev;
    QString document;
    Params(QTextDocument *d, const QString& path, int pos, int rev)
        : docPtr(d), docPath(path), pos(pos), rev(rev), document(d->toPlainText()) {}
};

UseSelectionResult findUses(const Params p)
{
    UseSelectionResult result;
    result.docPtr = p.docPtr;
    result.docPath = p.docPath;
    result.pos = p.pos;
    result.rev = p.rev;
    try {
        QPair<int, int> symbolRange = Dcd::findSymbol(p.document, p.pos);
        const int symbolLength = symbolRange.second - symbolRange.first;
        result.symbol = p.document.mid(symbolRange.first, symbolLength);
        QString projectName = ProjectExplorer::ProjectExplorerPlugin::currentProject() ?
                    ProjectExplorer::ProjectExplorerPlugin::currentProject()->displayName() : QString();
        Dcd::DcdFactory::ClientPointer client = Dcd::DcdFactory::instance()->client(projectName);
        if (!client) {
            throw std::runtime_error("bad client ptr");
        }
        if (symbolLength > 0) {
            client->getSymbolsByName(p.document, result.symbol, result.list);
        }
    } catch (std::exception& err) {
        qDebug() << "UseSelection error: " << err.what();
    } catch (...) {
        qDebug() << "UseSelection error: unknown";
    }
    return result;
}

void DlangUseSelectionUpdater::updateSynchronously()
{
    const Params params = Params(m_editorWidget->document(),
                                 m_editorWidget->textDocument()->filePath(),
                                 m_editorWidget->position(),
                                 m_editorWidget->document()->revision());
    processResults(findUses(params));
}

void DlangUseSelectionUpdater::updateAsynchronously()
{
    if (m_findUsesWatcher)
        m_findUsesWatcher->cancel();
    m_findUsesWatcher.reset(new QFutureWatcher<UseSelectionResult>);
    connect(m_findUsesWatcher.data(), SIGNAL(finished()), this, SLOT(onFindUsesFinished()));

    const Params params = Params(m_editorWidget->document(),
                                 m_editorWidget->textDocument()->filePath(),
                                 m_editorWidget->position(),
                                 m_editorWidget->document()->revision());
    m_findUsesWatcher->setFuture(QtConcurrent::run(&findUses, params));
}

void DlangUseSelectionUpdater::processResults(const UseSelectionResult &result)
{
    if (result.docPtr != m_editorWidget->document()) {
        return;
    }
    if (result.rev != m_editorWidget->document()->revision()) {
        return;
    }
    if (result.docPath != m_editorWidget->textDocument()->filePath()) {
        return;
    }
    if (result.pos != m_editorWidget->position()) {
        return;
    }

    const int symbolLength = result.symbol.length();
    ExtraSelections selections;
    foreach (const auto& l, result.list) {
        if (l.first.filename == "stdin") {
            QTextEdit::ExtraSelection sel;
            sel.format = m_editorWidget->textDocument()->fontSettings().toTextCharFormat(TextEditor::C_OCCURRENCES);
            sel.cursor = QTextCursor(m_editorWidget->document());
            sel.cursor.setPosition(l.first.position + symbolLength);
            sel.cursor.setPosition(l.first.position, QTextCursor::KeepAnchor);
            selections.append(sel);
        }
    }
    m_editorWidget->setExtraSelections(TextEditor::TextEditorWidget::CodeSemanticsSelection, selections);
}

