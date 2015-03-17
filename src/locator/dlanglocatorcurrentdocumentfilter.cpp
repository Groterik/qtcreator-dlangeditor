#include "dlanglocatorcurrentdocumentfilter.h"

#include "codemodel/dmodel.h"
#include "dlangimagecache.h"

#include <utils/fileutils.h>
#include <texteditor/texteditor.h>

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

    DCodeModel::OutlineList list;
    try {
        DCodeModel::IModelSharedPtr model = DCodeModel::Factory::instance().getModel();
        model->getCurrentDocumentSymbols(editor->textDocument()->plainText(), list);
    }
    catch (...) {
        return goodEntries;
    }

    foreach (auto& info, list) {
        if (future.isCanceled())
            break;

        QString matchString = info.symbol.data;

        if ((hasWildcard && regexp.exactMatch(matchString))
            || (!hasWildcard && matcher.indexIn(matchString) != -1))
        {
            QVariant id = qVariantFromValue(info.symbol.location.position);
            QString name = matchString;
            QString extraInfo = info.extra;
            Core::LocatorFilterEntry filterEntry(this, name, id,
                                                 DlangEditor::DlangIconCache::instance().fromType(info.symbol.type));
            filterEntry.extraInfo = extraInfo;

            if (matchString.startsWith(entry, caseSensitivityForPrefix))
                betterEntries.append(filterEntry);
            else
                goodEntries.append(filterEntry);
        }
    }

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
    Core::EditorManager::openEditorAt(textEditor->document()->filePath(), line, column);
}

void DlangLocatorCurrentDocumentFilter::refresh(QFutureInterface<void> &future)
{
    Q_UNUSED(future)
}

