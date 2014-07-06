#include "dlangcompletionassistprovider.h"

#include "dlangeditorconstants.h"

#include <texteditor/codeassist/keywordscompletionassist.h>

DlangCompletionAssistProvider::DlangCompletionAssistProvider() :
    TextEditor::CompletionAssistProvider()
{
}

bool DlangCompletionAssistProvider::supportsEditor(const Core::Id &editorId) const
{
    return editorId == DlangEditor::Constants::DLANG_EDITOR_ID;
}

TextEditor::IAssistProcessor *DlangCompletionAssistProvider::createProcessor() const
{
    return new TextEditor::KeywordsCompletionAssistProcessor(TextEditor::Keywords());
}

bool DlangCompletionAssistProvider::isActivationCharSequence(const QString &sequence) const
{
    return completionTypeOfChar(sequence.at(0)) != DLANG_NO_COMPLETION;
}

int DlangCompletionAssistProvider::activationCharSequenceLength() const
{
    return 1;
}

DlangCompletionAssistProvider::DlangCompletionType DlangCompletionAssistProvider::completionTypeOfChar(QChar ch1)
{
    switch (ch1.toLatin1()) {
    case '.':
        return DLANG_DOT;
    case '(':
        return DLANG_FUNCTION;
    case ',':
        return DLANG_COMMA;
    }
    return DLANG_NO_COMPLETION;
}
