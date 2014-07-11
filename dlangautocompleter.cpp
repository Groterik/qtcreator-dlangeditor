#include "dlangautocompleter.h"

#include <QTextCursor>
#include <QTextDocument>
#include <QTextBlock>

using namespace DlangEditor;

DlangAutoCompleter::DlangAutoCompleter()
{
}

bool DlangAutoCompleter::contextAllowsAutoParentheses(const QTextCursor &cursor, const QString &textToInsert) const
{
    Q_UNUSED(cursor);
    Q_UNUSED(textToInsert);
    return true;
}

QString DlangAutoCompleter::insertMatchingBrace(const QTextCursor &cursor, const QString &text, QChar la, int *skippedChars) const
{
    Q_UNUSED(skippedChars)

    if (text.isEmpty() || !shouldInsertMatchingText(la))
        return QString();

    QString result;

    foreach (const QChar &ch, text) {
        if      (ch == QLatin1Char('('))  result += QLatin1Char(')');
        else if (ch == QLatin1Char('['))  result += QLatin1Char(']');
        else if (ch == QLatin1Char('"'))  result += QLatin1Char('"');
        else if (ch == QLatin1Char('\'')) result += QLatin1Char('\'');
        else if (ch == QLatin1Char('{')) {
            const QString blockText = cursor.block().text().mid(cursor.positionInBlock());
            const QString trimmedBlockText = blockText.trimmed();
            if (!trimmedBlockText.isEmpty() && trimmedBlockText.at(0) == QLatin1Char(')')) {
                result += QLatin1Char('}');
            }
        }

    }

    return result;
}

QString DlangAutoCompleter::insertParagraphSeparator(const QTextCursor &cursor) const
{
    QChar c = ' ';
    Q_ASSERT(c.isSpace());
    for (int i = 1; c.isSpace(); ++i) {
        c = cursor.document()->characterAt(cursor.position() - i);
    }
    if (c != QLatin1Char('{')) {
        return QString();
    }
    return "}";
}

bool DlangAutoCompleter::shouldInsertMatchingText(QChar c) const
{
    switch (c.unicode()) {
    case '{': case '}':
    case ']': case ')':
    case ';': case ',':
        return true;
    default:
        if (c.isSpace())
            return true;
    }
    return false;
}
