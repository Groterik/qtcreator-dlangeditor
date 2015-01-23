#ifndef DLANGASSISTPROCESSOR_H
#define DLANGASSISTPROCESSOR_H

#include <QMap>

#include <texteditor/codeassist/iassistprocessor.h>
#include <dcdsupport.h>

namespace DlangEditor {

class DlangAssistProcessor : public TextEditor::IAssistProcessor
{
public:
    DlangAssistProcessor();
    virtual ~DlangAssistProcessor();

    // pure TextEditor::IAssistProcessor

    /**
     * @brief The method is called everytime when completion is needed
     * @param interface has usefull information about document and position of completion
     * @return proposal for completion
     */
    virtual TextEditor::IAssistProposal *perform(const TextEditor::AssistInterface *interface);
private:
    bool accepts();

    TextEditor::IAssistProposal *proposal();
    TextEditor::IAssistProposal *completeAt();

    QScopedPointer<const TextEditor::AssistInterface> m_interface;
    Dcd::DcdFactory::ClientPointer m_client;
    int m_proposalOffset;
};

} // namespace DlangEditor

#endif // DLANGASSISTPROCESSOR_H
