#include "dlangautocompleter.h"

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
    Q_UNUSED(cursor)
    Q_UNUSED(text)
    Q_UNUSED(la)
    Q_UNUSED(skippedChars)
    return QString();
}

QString DlangAutoCompleter::insertParagraphSeparator(const QTextCursor &cursor) const
{
    Q_UNUSED(cursor)
    return QString();
}
