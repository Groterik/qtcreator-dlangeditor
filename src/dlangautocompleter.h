#ifndef DLANGAUTOCOMPLETER_H
#define DLANGAUTOCOMPLETER_H

#include <texteditor/autocompleter.h>

namespace DlangEditor {

/**
 * @brief The class responsibilities are autocompletion the matching braces, parentheses, etc
 * Also this class affects indention by allowing electric chars
 */
class DlangAutoCompleter : public TextEditor::AutoCompleter
{
public:
    DlangAutoCompleter();

    /**
     * @brief Allow auto parentheses.
     * By default, contextAllowsElectricCharacters() = contextAllowsAutoParentheses()
     *  and it is important for indention of curly braces, etc
     */
    virtual bool contextAllowsAutoParentheses(const QTextCursor &cursor, const QString &textToInsert) const Q_DECL_OVERRIDE;
    virtual QString insertMatchingBrace(const QTextCursor &cursor, const
                                        QString &text,
                                        QChar la,
                                        int *skippedChars) const Q_DECL_OVERRIDE;
    virtual QString insertParagraphSeparator(const QTextCursor &cursor) const Q_DECL_OVERRIDE;
private:
    bool shouldInsertMatchingText(QChar c) const;
};

} // namespace DlangEditor

#endif // DLANGAUTOCOMPLETER_H
