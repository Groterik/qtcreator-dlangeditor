#ifndef DLANGTEXTEDITOR_H
#define DLANGTEXTEDITOR_H

#include <texteditor/basetextdocument.h>
#include <texteditor/basetexteditor.h>
#include <coreplugin/editormanager/ieditorfactory.h>

class DlangEditorWidget;

class DlangTextEditor : public TextEditor::BaseTextEditor
{
    Q_OBJECT
public:
    explicit DlangTextEditor(DlangEditorWidget *parent = 0);

signals:

public slots:

};

class DlangEditorWidget : public TextEditor::BaseTextEditorWidget
{
    Q_OBJECT
public:
    explicit DlangEditorWidget(QWidget *parent = 0);

    // pure TextEditor::BaseTextEditorWidget
    virtual TextEditor::BaseTextEditor *createEditor();

    // others

signals:

public slots:
    virtual void unCommentSelection();
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

#endif // DLANGTEXTEDITOR_H
