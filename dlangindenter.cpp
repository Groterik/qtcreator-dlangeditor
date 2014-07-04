#include "dlangindenter.h"

#include <texteditor/tabsettings.h>

#include <QTextDocument>
#include <QTextBlock>
#include <QTextCursor>

DlangIndenter::DlangIndenter()
{
}

bool DlangIndenter::isElectricCharacter(const QChar &ch) const
{
    if (ch == QLatin1Char('{')
            || ch == QLatin1Char('}')) {
        return true;
    }
    return false;
}

void DlangIndenter::indentBlock(QTextDocument *doc,
                                const QTextBlock &block,
                                const QChar &typedChar,
                                const TextEditor::TabSettings &tabSettings)
{
    // At beginning: Leave as is.
    if (block == doc->begin())
        return;

    const int tabsize = tabSettings.m_indentSize;

    QTextBlock previous = block.previous();
    QString previousText = previous.text();
    while (previousText.trimmed().isEmpty()) {
        previous = previous.previous();
        if (previous == doc->begin())
            return;
        previousText = previous.text();
    }

    int adjust = 0;
    if (previousText.contains(QLatin1Char('{')))
        adjust = tabsize;

    if (block.text().contains(QLatin1Char('}')) || typedChar == QLatin1Char('}'))
        adjust += -tabsize;

    // Count the indentation of the previous line.
    int i = 0;
    while (i < previousText.size()) {
        if (!previousText.at(i).isSpace()) {
            tabSettings.indentLine(block, tabSettings.columnAt(previousText, i)
                                   + adjust);
            break;
        }
        ++i;
    }
}
