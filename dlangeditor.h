#ifndef DLANGTEXTEDITOR_H
#define DLANGTEXTEDITOR_H

#include <texteditor/basetextdocument.h>
#include <texteditor/basetexteditor.h>
#include <coreplugin/editormanager/ieditorfactory.h>
#include <utils/uncommentselection.h>

namespace DlangEditor {

class DlangTextEditorWidget;

class DlangTextEditor : public TextEditor::BaseTextEditor
{
    Q_OBJECT
public:
    explicit DlangTextEditor(DlangTextEditorWidget *parent = 0);

    // pure TextEditor::BaseTextEditor
    virtual bool duplicateSupported() const;
    virtual Core::IEditor *duplicate();

    // custom
    virtual TextEditor::CompletionAssistProvider *completionAssistProvider();
    virtual QString contextHelpId() const;

    // others

signals:

public slots:

private:
    DlangTextEditor(TextEditor::BaseTextEditorWidget*);

};

class DlangTextEditorWidget : public TextEditor::BaseTextEditorWidget
{
    Q_OBJECT
public:
    explicit DlangTextEditorWidget(QWidget *parent = 0);

    // pure TextEditor::BaseTextEditorWidget
    virtual TextEditor::BaseTextEditor *createEditor();

    // others

signals:

public slots:
    virtual void unCommentSelection();
private:
    virtual void contextMenuEvent(QContextMenuEvent *e);
    virtual Link findLinkAt(const QTextCursor &c, bool resolveTarget, bool inNextSplit);
};

class DlangEditorFactory : public Core::IEditorFactory
{
    Q_OBJECT

public:
    DlangEditorFactory(QObject *parent);

    // pure Core::IEditorFactory
    Core::IEditor *createEditor();

    // others

private:
    const QStringList m_mimeTypes;
};

class DlangDocument : public TextEditor::BaseTextDocument
{
    Q_OBJECT

public:
    DlangDocument();
    QString defaultPath() const;
    QString suggestedFileName() const;
};

bool getFullIdentifier(const TextEditor::ITextEditorDocument* doc, int pos, int &begin, int &size);

} // namespace DlangEditor

#endif // DLANGTEXTEDITOR_H
