#ifndef DLANGINDENTER_H
#define DLANGINDENTER_H

#include <texteditor/indenter.h>

class DlangIndenter : public TextEditor::Indenter
{
public:
    DlangIndenter();
    virtual bool isElectricCharacter(const QChar &ch) const;
    virtual void indentBlock(QTextDocument *doc,
                             const QTextBlock &block,
                             const QChar &typedChar,
                             const TextEditor::TabSettings &tabSettings);
};

#endif // DLANGINDENTER_H
