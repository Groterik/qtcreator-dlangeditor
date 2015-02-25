#ifndef DLANGINDENTER_H
#define DLANGINDENTER_H

#include <texteditor/indenter.h>

namespace DlangEditor {

class DlangIndenter : public TextEditor::Indenter
{
public:
    DlangIndenter();
    virtual bool isElectricCharacter(const QChar &ch) const Q_DECL_OVERRIDE;
    virtual void indentBlock(QTextDocument *doc,
                             const QTextBlock &block,
                             const QChar &typedChar,
                             const TextEditor::TabSettings &tabSettings) Q_DECL_OVERRIDE;
};

} // namespace DlangEditor

#endif // DLANGINDENTER_H
