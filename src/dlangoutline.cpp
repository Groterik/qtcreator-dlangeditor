#include "dlangoutline.h"

#include "dlangimagecache.h"

#include <QStandardItemModel>

using namespace DlangEditor;

bool DlangEditor::DlangOutlineWidgetFactory::supportsEditor(Core::IEditor *editor) const
{
    return editor ? (qobject_cast<DlangTextEditor*>(editor) != 0) : false;
}

TextEditor::IOutlineWidget *DlangOutlineWidgetFactory::createWidget(Core::IEditor *editor)
{
    Q_ASSERT(editor);
    return new DlangOutlineWidget(qobject_cast<DlangTextEditor*>(editor));
}


DlangOutlineWidget::DlangOutlineWidget(DlangTextEditor *editor)
    : m_editor(editor)
{
    m_treeView = new Utils::NavigationTreeView(this);
    m_treeView->setExpandsOnDoubleClick(false);
    m_treeView->setDragEnabled(true);
    m_treeView->setDragDropMode(QAbstractItemView::DragOnly);
    m_treeView->setModel(editor->outline());
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


DlangOutlineModel::DlangOutlineModel(QObject *object)
    : QStandardItemModel(object)
{

}

const DCodeModel::Scope &DlangOutlineModel::scope() const
{
    return m_scope;
}

void DlangOutlineModel::updateForEditor(DlangTextEditor *editor)
{
    clear();
    if (!editor || !editor->textDocument()) {
        return;
    }

    try {
        DCodeModel::IModelSharedPtr model = DCodeModel::Factory::instance().getModel();
        model->getCurrentDocumentSymbols(editor->textDocument()->plainText(), m_scope);
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
            item->appendRow(item);
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
