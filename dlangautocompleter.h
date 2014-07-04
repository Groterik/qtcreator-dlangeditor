#ifndef DLANGAUTOCOMPLETER_H
#define DLANGAUTOCOMPLETER_H

#include <texteditor/autocompleter.h>

class DlangAutoCompleter : public TextEditor::AutoCompleter
{
public:
    DlangAutoCompleter();

    virtual bool contextAllowsAutoParentheses(const QTextCursor &cursor, const QString &textToInsert) const;
    virtual QString insertMatchingBrace(const QTextCursor &cursor, const
                                        QString &text,
                                        QChar la,
                                        int *skippedChars) const;
    virtual QString insertParagraphSeparator(const QTextCursor &cursor) const;
private:
    bool shouldInsertMatchingText(QChar c) const;
};

#endif // DLANGAUTOCOMPLETER_H
