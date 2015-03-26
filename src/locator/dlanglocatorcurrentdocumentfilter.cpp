#include "dlanglocatorcurrentdocumentfilter.h"

#include <functional>

#include "codemodel/dmodel.h"
#include "dlangimagecache.h"
#include "dlangeditor.h"
#include "dlangoutlinemodel.h"

#include <utils/fileutils.h>
#include <texteditor/texteditor.h>
#include <texteditor/textdocument.h>
#include <coreplugin/editormanager/editormanager.h>
#include <utils/qtcassert.h>

using namespace DlangEditor;

DlangLocatorCurrentDocumentFilter::DlangLocatorCurrentDocumentFilter()
{
    setId("Dlang Symbols in current Document");
    setDisplayName(tr("D Symbols in Current Document"));
    setShortcutString(QString(QLatin1Char('.')));
    setPriority(Medium);
    setIncludedByDefault(false);
}

DlangLocatorCurrentDocumentFilter::~DlangLocatorCurrentDocumentFilter()
{

}

QList<Core::LocatorFilterEntry> DlangLocatorCurrentDocumentFilter::matchesFor(QFutureInterface<Core::LocatorFilterEntry> &future, const QString &origEntry)
{
    QString entry = trimWildcards(origEntry);
    QList<Core::LocatorFilterEntry> goodEntries;
    QList<Core::LocatorFilterEntry> betterEntries;
    QStringMatcher matcher(entry, Qt::CaseInsensitive);
    const QChar asterisk = QLatin1Char('*');
    QRegExp regexp(asterisk + entry + asterisk, Qt::CaseInsensitive, QRegExp::Wildcard);
    const auto *editor = TextEditor::BaseTextEditor::currentTextEditor();
    if (!regexp.isValid() || !editor)
        return goodEntries;
    bool hasWildcard = (entry.contains(asterisk) || entry.contains(QLatin1Char('?')));
    const Qt::CaseSensitivity caseSensitivityForPrefix = caseSensitivity(entry);

    auto widget = qobject_cast<DlangTextEditorWidget*>(editor->widget());
    QTC_ASSERT(widget, return goodEntries);
    widget->outline()->update();
    auto& scope = widget->outline()->scope();

    QStringList scopeStack;
    std::function<void(const DCodeModel::Scope&)> makeLocatorList = [&](const DCodeModel::Scope &scope)
    {
        foreach (auto &sym, scope.symbols) {
            if (future.isCanceled())
                break;

            QString matchString = sym.name;

            if ((hasWildcard && regexp.exactMatch(matchString))
                    || (!hasWildcard && matcher.indexIn(matchString) != -1))
            {
                QVariant id = qVariantFromValue(sym.location.position);
                QString name = matchString;
                QString extraInfo = scopeStack.join('.');
                Core::LocatorFilterEntry filterEntry(this, name, id,
                                                     DlangEditor::DlangIconCache::instance().fromType(sym.type));
                filterEntry.extraInfo = extraInfo;

                if (matchString.startsWith(entry, caseSensitivityForPrefix))
                    betterEntries.append(filterEntry);
                else
                    goodEntries.append(filterEntry);
            }
        }

        foreach (auto &child, scope.children) {
            scopeStack.push_back(child.master.name);
            makeLocatorList(child);
            scopeStack.pop_back();
        }
    };

    makeLocatorList(scope);

    // entries are unsorted by design!

    betterEntries += goodEntries;
    return betterEntries;
}

void DlangLocatorCurrentDocumentFilter::accept(Core::LocatorFilterEntry selection) const
{
    auto textEditor = TextEditor::BaseTextEditor::currentTextEditor();
    if (!textEditor) {
        return;
    }
    int position = selection.internalData.toInt();
    int line = -1;
    int column = -1;
    textEditor->convertPosition(position, &line, &column);
#if QTCREATOR_MINOR_VERSION < 4
    Core::EditorManager::openEditorAt(textEditor->document()->filePath(), line, column);
#else
    Core::EditorManager::openEditorAt(textEditor->document()->filePath().toString(), line, column);
#endif
}

void DlangLocatorCurrentDocumentFilter::refresh(QFutureInterface<void> &future)
{
    Q_UNUSED(future)
}

