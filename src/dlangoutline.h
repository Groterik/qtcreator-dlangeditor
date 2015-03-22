#ifndef DLANGOUTLINE_H
#define DLANGOUTLINE_H

#include "codemodel/dmodel.h"
#include "dlangeditor.h"

#include <texteditor/ioutlinewidget.h>

#include <utils/navigationtreeview.h>

#include <QStandardItemModel>

namespace DlangEditor {

class DlangOutlineModel : public QStandardItemModel
{
    Q_OBJECT
public:
    DlangOutlineModel(QObject *object = 0);
    const DCodeModel::Scope &scope() const;
public slots:
    void updateForEditor(DlangTextEditor *editor);
private:
    DCodeModel::Scope m_scope;
};

class DlangOutlineWidget : public TextEditor::IOutlineWidget
{
    Q_OBJECT
public:
    DlangOutlineWidget(DlangTextEditor *editor);

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
    DlangTextEditor *m_editor;
    ::Utils::NavigationTreeView *m_treeView;

    bool m_enableCursorSync;
    bool m_blockCursorSync;
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
