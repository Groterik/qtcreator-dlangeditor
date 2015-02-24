#include "dlangindenter.h"

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
            || ch == QLatin1Char('}')) {
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
    int prevIndent = 0;
    for (int i = 0; i < prevLen; ++i) {
        const char c = text.at(i).toLatin1();
        if (!text.at(i).isSpace() && prevIndent == 0) {
            prevIndent = i;
        }

        switch (c) {
        case '{': indent += tabSize; padding = 0; break;
        case '}': indent -= tabSize; padding = 0; break;
        case ')':
        case ':':
        case ';': padding = 0; break;
        default: padding = tabSize; break;
        }
    }

    if (prevIndent >= 0 && text.at(prevIndent) == QChar('}')) {
        indent += tabSize;
    }

    text = block.text().trimmed();
    if (text.startsWith('}')) {
        indent -= tabSize;
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
