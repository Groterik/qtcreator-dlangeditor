#include "dlangoutline.h"

#include <utils/qtcassert.h>
#include <coreplugin/find/itemviewfind.h>
#include <utils/navigationtreeview.h>
#include <utils/treeviewcombobox.h>

#include "dlangeditor.h"
#include "dlangoutlinemodel.h"
#include "dlangimagecache.h"

#include <QVBoxLayout>

using namespace DlangEditor;

bool DlangEditor::DlangOutlineWidgetFactory::supportsEditor(Core::IEditor *editor) const
{
    return editor ? (qobject_cast<DlangTextEditor*>(editor) != 0) : false;
}

TextEditor::IOutlineWidget *DlangOutlineWidgetFactory::createWidget(Core::IEditor *editor)
{
    Q_ASSERT(editor);
    auto dlangEditor = qobject_cast<DlangTextEditor*>(editor);
    QTC_ASSERT(dlangEditor, return 0);
    auto dlangEditorWidget = qobject_cast<DlangTextEditorWidget*>(dlangEditor->widget());
    QTC_ASSERT(dlangEditorWidget, return 0);
    return new DlangOutlineWidget(dlangEditorWidget);
}


DlangOutlineWidget::DlangOutlineWidget(DlangTextEditorWidget *editor)
    : m_editor(editor)
{
    m_treeView = new Utils::NavigationTreeView(this);
    m_treeView->setExpandsOnDoubleClick(false);
    m_treeView->setDragEnabled(true);
    m_treeView->setDragDropMode(QAbstractItemView::DragOnly);
    m_treeView->setModel(editor->outline());

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(Core::ItemViewFind::createSearchableWrapper(m_treeView));
    setLayout(layout);

    setFocusProxy(m_treeView);

    connect(editor->outline(), SIGNAL(modelUpdated()), this, SLOT(modelUpdated()));
    editor->outline()->update();
}

QList<QAction *> DlangOutlineWidget::filterMenuActions() const
{
    return QList<QAction*>();
}

void DlangOutlineWidget::setCursorSynchronization(bool syncWithCursor)
{
    m_enableCursorSync = syncWithCursor;
//    if (m_enableCursorSync)
//        updateSelectionInTree(m_editor->outline()->modelIndex());
}

void DlangOutlineWidget::modelUpdated()
{
    m_treeView->expandAll();
}

void DlangOutlineWidget::updateSelectionInTree(const QModelIndex &index)
{
    Q_UNUSED(index)
}

void DlangOutlineWidget::updateTextCursor(const QModelIndex &index)
{
    Q_UNUSED(index)
}

void DlangOutlineWidget::onItemActivated(const QModelIndex &index)
{
    Q_UNUSED(index)
}


DlangTextEditorOutline::DlangTextEditorOutline(DlangTextEditorWidget *editorWidget)
    : QWidget(editorWidget), m_editorWidget(editorWidget)
{
    m_combo = new ::Utils::TreeViewComboBox(this);
    m_combo->setMinimumContentsLength(22);
    QSizePolicy policy = this->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    this->setSizePolicy(policy);
    m_combo->setMaxVisibleItems(40);
    m_combo->view()->expandAll();
    m_combo->setModel(m_editorWidget->outline());
    connect(editorWidget->outline(), SIGNAL(modelUpdated()), this, SLOT(update()));
}

void DlangTextEditorOutline::update()
{
    m_combo->view()->expandAll();
}
