#include "dlangindenter.h"

#include "dlangautocompleter.h"

#include <texteditor/tabsettings.h>
#include <texteditor/textdocumentlayout.h>

#include <QTextDocument>
#include <QTextBlock>
#include <QTextCursor>

using namespace DlangEditor;

enum {
    INDENTER_USER_FORMAT_ID = QTextFormat::UserFormat + 1
};

DlangIndenter::DlangIndenter()
{
}

bool DlangIndenter::isElectricCharacter(const QChar &ch) const
{
    if (ch == QLatin1Char('{')
            || ch == QLatin1Char('}')
            || ch == QLatin1Char(':')) {
        return true;
    }
    return false;
}

class IndenterUserFormat
{
public:
    int indent;
    int padding;
    IndenterUserFormat() : indent(0), padding(0) {}
};

Q_DECLARE_METATYPE(IndenterUserFormat)

/**
 * @brief Calculate indent and padding for block
 * @note Real offset of block = indent + padding (differs from QtC)
 * @param origBlock block to indent
 * @param tabSize size of tab
 * @return indent and padding
 */
IndenterUserFormat calculateIndent(const QTextBlock &origBlock, int tabSize)
{
    QTextBlock block = origBlock;
    QTextBlock prev = block;
    do {
        prev = prev.previous();
        if (!prev.isValid()) {
            return IndenterUserFormat();
        }
    } while (prev.text().trimmed().isEmpty());

    IndenterUserFormat prevData = prev.blockFormat().property(INDENTER_USER_FORMAT_ID).value<IndenterUserFormat>();

    int indent = 0;
    int padding = 0;
    QString text = prev.text();
    const int prevLen = text.length();
    int prevOffset = -1;
    for (int pos = 0; pos < prevLen; ++pos) {
        const QChar qc = text.at(pos);
        if (!qc.isSpace()) {
            if (prevOffset == -1) {
                prevOffset = pos;
            }
        } else {
            continue;
        }

        const char c = qc.toLatin1();
        switch (c) {
        case '{': indent += tabSize; padding = 0; break;
        case '}': indent -= tabSize; padding = 0; break;
        case ':':
        case ';': padding = 0; break;
        default: padding = tabSize; break;
        }
    }

    if (prevOffset >= 0) {
        QChar c = text.at(prevOffset);
        if (c == QChar('}')) {
            indent += tabSize;
        } else {
            QChar n = prevOffset + 1 < prevLen ? text.at(prevOffset + 1) : QChar();
            if ((c == QChar('*') && DdocAutoCompleter::isDdocComment(QTextCursor(prev)))
                    || (c == QChar('/') && (n == QChar('*') || n == QChar('/')))) {
                padding = 0;
                indent = 0;
            }
        }
    } else {
        prevOffset = 0;
    }

    text = block.text().trimmed();
    if (text.startsWith('}')) {
        padding = 0;
        indent -= tabSize;
    } else if (text.startsWith('{')) {
        padding = 0;
    } else if (text.endsWith(':')) {
        padding = -tabSize;
    }


    auto setBlockIndenterData = [](QTextBlock& block, IndenterUserFormat d) {
        QTextCursor c(block);
        auto f = c.blockFormat();
        f.setProperty(INDENTER_USER_FORMAT_ID, QVariant::fromValue(d));
        c.setBlockFormat(f);
    };

    if (prevOffset < 0) {
        prevOffset = 0;
    }

    if (prevData.indent + prevData.padding != prevOffset) {
        prevData.indent = prevOffset;
        prevData.padding = 0;
        setBlockIndenterData(prev, prevData);
    }

    indent += prevData.indent;

    if (padding > 0 && prevData.padding > 0) {
        indent = prevData.indent;
        padding = prevData.padding;
    }

    IndenterUserFormat blockData;
    blockData.indent = indent;
    blockData.padding = padding;

    setBlockIndenterData(block, blockData);

    return blockData;
}

void DlangIndenter::indentBlock(QTextDocument *doc,
                                const QTextBlock &block,
                                const QChar &/*typedChar*/,
                                const TextEditor::TabSettings &tabSettings)
{
    // At beginning: Leave as is.
    if (block == doc->begin())
        return;

    const auto align = calculateIndent(block, tabSettings.m_indentSize);
    tabSettings.indentLine(block, align.indent + align.padding, align.padding);
}
