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
    int prevIndent = -1;
    for (int pos = 0; pos < prevLen; ++pos) {
        const QChar qc = text.at(pos);
        if (!qc.isSpace()) {
            if (prevIndent == -1) {
                prevIndent = pos;
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

    if (prevIndent >= 0) {
        QChar c = text.at(prevIndent);
        if (c == QChar('}')) {
            indent += tabSize;
        } else {
            QChar n = prevIndent + 1 < prevLen ? text.at(prevIndent + 1) : QChar();
            if ((c == QChar('*') && DdocAutoCompleter::isDdocComment(QTextCursor(prev)))
                    || (c == QChar('/') && (n == QChar('*') || n == QChar('/')))) {
                padding = 0;
                indent = 0;
            }
        }
    } else {
        prevIndent = 0;
    }

    text = block.text().trimmed();
    if (text.startsWith('}')) {
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

    if (prevData.indent != prevIndent) {
        prevData.indent = prevIndent;
        setBlockIndenterData(prev, prevData);
    }

    IndenterUserFormat blockData;
    blockData.indent = indent + prevIndent - prevData.padding + padding;
    blockData.padding = padding;

    if (padding) {
        if (prevData.padding) {
            indent = prevData.indent;
            padding = prevData.padding;
        }
    }

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
    tabSettings.indentLine(block, align.indent, align.padding);
}
