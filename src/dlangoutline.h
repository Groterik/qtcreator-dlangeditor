#ifndef DLANGOUTLINE_H
#define DLANGOUTLINE_H

#include "codemodel/dmodel.h"

#include <texteditor/ioutlinewidget.h>

#include <QStandardItemModel>

namespace Utils {
class NavigationTreeView;
class TreeViewComboBox;
}

namespace DlangEditor {

class DlangTextEditorWidget;

class DlangOutlineModel : public QStandardItemModel
{
    Q_OBJECT
public:
    DlangOutlineModel(DlangTextEditorWidget *object = 0);
    const DCodeModel::Scope &scope() const;
    bool needUpdate() const;
public slots:
    void updateForEditor(DlangTextEditorWidget *editor);
    void update();
private:
    DCodeModel::Scope m_scope;
    struct DocumentState
    {
        QString filePath;
        int rev;
    } m_documentState;
    DlangTextEditorWidget *m_editor;
};

class DlangOutlineWidget : public TextEditor::IOutlineWidget
{
    Q_OBJECT
public:
    DlangOutlineWidget(DlangTextEditorWidget *editor);

    // IOutlineWidget
    virtual QList<QAction*> filterMenuActions() const;
    virtual void setCursorSynchronization(bool syncWithCursor);

private slots:
    void modelUpdated();
    void updateSelectionInTree(const QModelIndex &index);
    void updateTextCursor(const QModelIndex &index);
    void onItemActivated(const QModelIndex &index);

private:
    bool syncCursor();

private:
    DlangTextEditorWidget *m_editor;
    ::Utils::NavigationTreeView *m_treeView;

    bool m_enableCursorSync;
    bool m_blockCursorSync;
};

class DlangTextEditorOutline : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(DlangTextEditorOutline)

public:
    explicit DlangTextEditorOutline(DlangTextEditorWidget *editorWidget);

    void update();
private:
    DlangTextEditorWidget *m_editorWidget;

    ::Utils::TreeViewComboBox *m_combo;
};

class DlangOutlineWidgetFactory : public TextEditor::IOutlineWidgetFactory
{
    Q_OBJECT
public:
    bool supportsEditor(Core::IEditor *editor) const;
    TextEditor::IOutlineWidget *createWidget(Core::IEditor *editor);
};


} // namespace DlangEditor

#endif // DLANGOUTLINE_H
