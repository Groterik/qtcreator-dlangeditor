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
