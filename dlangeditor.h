#ifndef DLANGTEXTEDITOR_H
#define DLANGTEXTEDITOR_H

#include <texteditor/textdocument.h>
#include <texteditor/texteditor.h>
#include <texteditor/texteditor.h>
#include <coreplugin/editormanager/ieditorfactory.h>
#include <utils/uncommentselection.h>

namespace DlangEditor {

class DlangTextEditorWidget;
class DlangUseSelectionUpdater;

class DlangTextEditor : public TextEditor::BaseTextEditor
{
    Q_OBJECT
public:
    explicit DlangTextEditor();

    // custom
    virtual TextEditor::CompletionAssistProvider *completionAssistProvider();
    virtual QString contextHelpId() const;

    // others

signals:

public slots:

private:
    DlangTextEditor(TextEditor::TextEditorWidget*);

};

class DlangTextEditorWidget : public TextEditor::TextEditorWidget
{
    Q_OBJECT
public:
    explicit DlangTextEditorWidget(QWidget *parent = 0);
    virtual ~DlangTextEditorWidget();

    // virtual TextEditor::TextEditorWidget
    virtual void finalizeInitialization();

    // others

signals:

public slots:
    virtual void unCommentSelection();
private:
    virtual void contextMenuEvent(QContextMenuEvent *e);
    virtual Link findLinkAt(const QTextCursor &c, bool resolveTarget, bool inNextSplit);

    DlangUseSelectionUpdater *m_useSelectionsUpdater;
};

class DlangEditorFactory : public TextEditor::TextEditorFactory
{
    Q_OBJECT

public:
    DlangEditorFactory();

    // others

private:
    const QStringList m_mimeTypes;
};

class DlangDocument : public TextEditor::TextDocument
{
    Q_OBJECT

public:
    DlangDocument();
    QString defaultPath() const;
    QString suggestedFileName() const;
};

bool getFullIdentifier(const TextEditor::TextDocument* doc, int pos, int &begin, int &size);

} // namespace DlangEditor

#endif // DLANGTEXTEDITOR_H
