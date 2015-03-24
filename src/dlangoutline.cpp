#include "dlangoutline.h"

#include <utils/qtcassert.h>
#include <coreplugin/find/itemviewfind.h>

#include "dlangeditor.h"
#include "dlangimagecache.h"

#include <QStandardItemModel>
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

    connect(editor->outline(), SIGNAL(modelReset()), this, SLOT(modelUpdated()));
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


DlangOutlineModel::DlangOutlineModel(DlangTextEditorWidget *object)
    : QStandardItemModel(object), m_editor(object)
{
    m_documentState.rev = -1;
}

const DCodeModel::Scope &DlangOutlineModel::scope() const
{
    return m_scope;
}

bool DlangOutlineModel::needUpdate() const
{
    QTC_ASSERT(m_editor && m_editor->document(), return false);
    return m_editor->document()->revision() != m_documentState.rev
            || m_editor->textDocument()->filePath() != m_documentState.filePath;
}

void DlangOutlineModel::updateForEditor(DlangTextEditorWidget *editor)
{
    m_editor = editor;
    update();
}

void DlangOutlineModel::update()
{
    if (!needUpdate()) {
        return;
    }
    clear();
    QTC_ASSERT(m_editor && m_editor->textDocument() && m_editor->document(), return);

    m_documentState.filePath = m_editor->textDocument()->filePath();
    m_documentState.rev = m_editor->document()->revision();

    try {
        DCodeModel::IModelSharedPtr model = DCodeModel::Factory::instance().getModel();
        model->getCurrentDocumentSymbols(m_editor->textDocument()->plainText(), m_scope);
    } catch (...) {
        clear();
        return;
    }

    std::function<void(const DCodeModel::Scope&, QStandardItem*)> prepareModel = [&](const DCodeModel::Scope& scope, QStandardItem *parent) {
        foreach (auto &sym, scope.symbols) {
            QStandardItem *item = new QStandardItem;
            item->setText(sym.data);
            item->setIcon(DlangIconCache::instance().fromType(sym.type));
            if (parent) {
                parent->appendRow(item);
            } else {
                this->appendRow(item);
            }
        }

        foreach (auto &child, scope.children) {
            QStandardItem *item = new QStandardItem;
            item->setText(child.name);
            if (parent) {
                parent->appendRow(item);
            } else {
                this->appendRow(item);
            }
            prepareModel(child, item);
        }
    };

    prepareModel(m_scope, 0);
}
