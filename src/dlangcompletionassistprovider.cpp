#include "dlangcompletionassistprovider.h"

#include "dlangeditorconstants.h"

#include "dlangassistprocessor.h"

#include <texteditor/codeassist/keywordscompletionassist.h>

using namespace DlangEditor;

DlangCompletionAssistProvider::DlangCompletionAssistProvider() :
    TextEditor::CompletionAssistProvider()
{
}

bool DlangCompletionAssistProvider::supportsEditor(Core::Id editorId) const
{
    return editorId == DlangEditor::Constants::DLANG_EDITOR_ID;
}

TextEditor::IAssistProcessor *DlangCompletionAssistProvider::createProcessor() const
{
    return new DlangAssistProcessor;
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
