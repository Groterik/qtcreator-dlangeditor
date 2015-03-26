#ifndef DLANGOUTLINE_H
#define DLANGOUTLINE_H

#include <texteditor/ioutlinewidget.h>

namespace Utils {
class NavigationTreeView;
class TreeViewComboBox;
}

namespace DlangEditor {

class DlangTextEditorWidget;

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
