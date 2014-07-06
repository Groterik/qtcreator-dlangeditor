#include "dlangassistprocessor.h"

#include <texteditor/codeassist/iassistinterface.h>

using namespace DlangEditor;

DlangAssistProcessor::DlangAssistProcessor()
{

}

DlangAssistProcessor::~DlangAssistProcessor()
{

}

TextEditor::IAssistProposal *DlangAssistProcessor::perform(const TextEditor::IAssistInterface *interface)
{
    m_interface.reset(static_cast<const TextEditor::IAssistInterface *>(interface));

    if (interface->reason() != TextEditor::ExplicitlyInvoked && !accepts())
        return 0;

    return proposals();
}

bool DlangAssistProcessor::accepts()
{
    return false;
}

TextEditor::IAssistProposal *DlangAssistProcessor::proposals()
{
    return 0;
}


