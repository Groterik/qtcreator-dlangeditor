#ifndef DLANGASSISTPROCESSOR_H
#define DLANGASSISTPROCESSOR_H

#include <texteditor/codeassist/iassistprocessor.h>

namespace DlangEditor {

class DlangAssistProcessor : public TextEditor::IAssistProcessor
{
public:
    DlangAssistProcessor();
    virtual ~DlangAssistProcessor();

    // pure TextEditor::IAssistProcessor
    virtual TextEditor::IAssistProposal *perform(const TextEditor::IAssistInterface *interface);
    QScopedPointer<const TextEditor::IAssistInterface> m_interface;
private:
    bool accepts();
    TextEditor::IAssistProposal *proposals();
    TextEditor::IAssistProposal *completeAt();
    int m_proposalOffset;
};

} // namespace DlangEditor

#endif // DLANGASSISTPROCESSOR_H
