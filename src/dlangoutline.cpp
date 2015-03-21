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
    QStandardItemModel *model = new QStandardItemModel;
    m_treeView = new Utils::NavigationTreeView(this);
    m_treeView->setModel(model);
    m_model = DCodeModel::Factory::instance().getModel();
}

QList<QAction *> DlangOutlineWidget::filterMenuActions() const
{

}

void DlangOutlineWidget::setCursorSynchronization(bool syncWithCursor)
{

}

void DlangOutlineWidget::modelUpdated()
{

}

void DlangOutlineWidget::updateSelectionInTree(const QModelIndex &index)
{

}

void DlangOutlineWidget::updateTextCursor(const QModelIndex &index)
{

}

void DlangOutlineWidget::onItemActivated(const QModelIndex &index)
{

}


DlangOutlineModel::DlangOutlineModel(QObject *object)
    : QStandardItemModel(object)
{

}

void DlangOutlineModel::updateForEditor(DlangTextEditor *editor)
{
    clear();
    if (!editor || !editor->textDocument()) {
        return;
    }

    DCodeModel::Scope outline;
    try {
        DCodeModel::IModelSharedPtr model = DCodeModel::Factory::instance().getModel();
        model->getCurrentDocumentSymbols(editor->textDocument()->plainText(), outline);
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

    prepareModel(outline, 0);
}
