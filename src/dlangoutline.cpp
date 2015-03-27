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
    : m_editor(editor), m_enableCursorSync(true)
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

    connect(m_editor->outline(), SIGNAL(modelUpdated()), this, SLOT(modelUpdated()));
    connect(m_editor, SIGNAL(cursorPositionChanged()), this, SLOT(updateSelectionInTree()));
    connect(m_treeView, SIGNAL(activated(QModelIndex)), this, SLOT(onItemActivated(QModelIndex)));
    modelUpdated();
}

QList<QAction *> DlangOutlineWidget::filterMenuActions() const
{
    return QList<QAction*>();
}

void DlangOutlineWidget::setCursorSynchronization(bool syncWithCursor)
{
    m_enableCursorSync = syncWithCursor;
    if (m_enableCursorSync)
        updateSelectionInTree();
}

void DlangOutlineWidget::modelUpdated()
{
    m_treeView->expandAll();
    updateSelectionInTree();
}

void DlangOutlineWidget::updateSelectionInTree()
{
    if (m_enableCursorSync) {
        QModelIndex ind = m_editor->outline()->byCursor(m_editor->textCursor().position());
        m_treeView->setCurrentIndex(ind);
    }
}

static void gotoSymbolInEditor(DlangTextEditorWidget *editor, const QModelIndex &index)
{
    QString filePath;
    int offset = 0;
    if (!editor->outline()->getLocation(index, filePath, offset)) {
        return;
    }
    editor->setCursorPosition(offset);
    editor->setFocus();
}

void DlangOutlineWidget::onItemActivated(const QModelIndex &index)
{
    gotoSymbolInEditor(m_editor, index);
}

DlangTextEditorOutline::DlangTextEditorOutline(DlangTextEditorWidget *editorWidget)
    : QWidget(editorWidget), m_editorWidget(editorWidget)
{
    m_combo = new ::Utils::TreeViewComboBox(this);
    m_combo->setModel(m_editorWidget->outline());

    m_combo->setMinimumContentsLength(22);
    QSizePolicy policy = this->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    this->setSizePolicy(policy);

    connect(editorWidget->outline(), SIGNAL(modelUpdated()), this, SLOT(update()));
    connect(m_editorWidget, SIGNAL(cursorPositionChanged()), this, SLOT(updateSelectionInCombo()));
    connect(m_combo, SIGNAL(activated(int)), this, SLOT(onItemActivated()));

    update();
}

void DlangTextEditorOutline::update()
{
    m_combo->view()->expandAll();
    updateSelectionInCombo();
}

void DlangTextEditorOutline::updateSelectionInCombo()
{
    QModelIndex ind = m_editorWidget->outline()->byCursor(m_editorWidget->textCursor().position());
    m_combo->setCurrentIndex(ind);
}

void DlangTextEditorOutline::onItemActivated()
{
    const QModelIndex index = m_combo->view()->currentIndex();
    gotoSymbolInEditor(m_editorWidget, index);
}
