#include "dlangoutline.h"

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
}
