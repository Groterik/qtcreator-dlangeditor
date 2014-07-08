#include "dlangassistprocessor.h"

#include <dlangcompletionassistprovider.h>

#include <texteditor/codeassist/iassistinterface.h>

using namespace DlangEditor;

QChar characterAt(const TextEditor::IAssistInterface* interface, int offset = 0)
{
    return interface->characterAt(interface->position() + offset);
}

bool findFunctionArgumentsBegin(const TextEditor::IAssistInterface* interface, int &offset)
{
    QChar c = characterAt(interface);
    Q_ASSERT(c.isSpace());
    int off;
    for (off = 1; !c.isNull() && c != QLatin1Char('(') && c != QLatin1Char(')'); ++off) {
        c = characterAt(interface, -off);
    }
    if (c == QLatin1Char('(')) {
        offset = -off;
        return true;
    }
    return false;
}

DlangAssistProcessor::DlangAssistProcessor()
{

}

DlangAssistProcessor::~DlangAssistProcessor()
{

}

TextEditor::IAssistProposal *DlangAssistProcessor::perform(const TextEditor::IAssistInterface *interface)
{
    m_interface.reset(static_cast<const TextEditor::IAssistInterface *>(interface));
    m_proposalOffset = 0;

    if (interface->reason() != TextEditor::ExplicitlyInvoked && !accepts())
        return 0;

    return proposals();
}

bool DlangAssistProcessor::accepts()
{
    QChar currentChar = characterAt(m_interface.data());
    switch (DlangCompletionAssistProvider::completionTypeOfChar(currentChar)) {
    case DlangCompletionAssistProvider::DLANG_DOT:
        return true;
    case DlangCompletionAssistProvider::DLANG_COMMA:
        return findFunctionArgumentsBegin(m_interface.data(), m_proposalOffset);
    default:
        return false;
    }
}

TextEditor::IAssistProposal *DlangAssistProcessor::proposals()
{
    // run dcd-client
}


