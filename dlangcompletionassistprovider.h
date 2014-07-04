#ifndef DLANGCOMPLETIONASSISTPROVIDER_H
#define DLANGCOMPLETIONASSISTPROVIDER_H

#include <texteditor/codeassist/completionassistprovider.h>

class DlangCompletionAssistProvider : public TextEditor::CompletionAssistProvider
{
    Q_OBJECT
public:
    explicit DlangCompletionAssistProvider();

    // pure TextEditor::CompletionAssistProvider
    virtual bool supportsEditor(const Core::Id &editorId) const;
    virtual TextEditor::IAssistProcessor *createProcessor() const;

    // others

signals:

public slots:

};

#endif // DLANGCOMPLETIONASSISTPROVIDER_H
