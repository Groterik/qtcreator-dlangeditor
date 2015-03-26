#include "dlangoutlinemodel.h"

#include <utils/qtcassert.h>

#include "dlangeditor.h"
#include "dlangimagecache.h"

using namespace DlangEditor;
using namespace DCodeModel;

DlangOutlineModel::DlangOutlineModel(DlangTextEditorWidget *object)
    : QAbstractItemModel(object), m_editor(object)
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

const Scope *DlangOutlineModel::byIndex(const QModelIndex &index) const
{
    if (!index.isValid() || !index.internalPointer()) {
        return 0;
    }
     return reinterpret_cast<const Scope*>(index.internalPointer());
}

QModelIndex DlangOutlineModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid() || parent.internalPointer() == 0) {
        if (row == 0) { // account for no symbol item
            return createIndex(row, column, (void*)0);
        }
        Scope *symbol = const_cast<Scope*>(&(m_scope.children.at(row - 1)));
        return createIndex(row, column, symbol);
    }
    const Scope *scope = reinterpret_cast<const Scope*>(parent.internalPointer());
    Scope *symbol = const_cast<Scope*>(&(scope->children.at(row)));
    return createIndex(row, column, symbol);
}

QModelIndex DlangOutlineModel::parent(const QModelIndex &child) const
{
    if (!child.isValid() || !child.internalPointer()) {
        return QModelIndex();
    }
    const Scope *scope = reinterpret_cast<const Scope*>(child.internalPointer());
    if (!scope->parent || !scope->parent->parent) {
        return QModelIndex();
    }
    return createIndex(scope->index, 0, scope->parent);
}

int DlangOutlineModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_scope.children.size() + 1;
    }
    const Scope *scope = reinterpret_cast<const Scope*>(parent.internalPointer());
    if (!scope) {
        return 0;
    }
    return scope->children.size();
}

int DlangOutlineModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant DlangOutlineModel::data(const QModelIndex &index, int role) const
{
    // account for no symbol item
    if (!index.parent().isValid() && index.row() == 0) {
        switch (role) {
        case Qt::DisplayRole:
            if (rowCount() > 1)
                return tr("<Select Symbol>");
            else
                return tr("<No Symbols>");
        default:
            return QVariant();
        }
    }

    const Scope *scope = byIndex(index);
    if (!scope) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole: {
        return scope->master.name;
    } break;

    case Qt::EditRole: {
        QString name = scope->master.name;
        if (name.isEmpty())
            name = QLatin1String("anonymous");
        return name;
    } break;

    case Qt::DecorationRole: {
        return DlangIconCache::instance().fromType(scope->master.type);
    } break;

    case FileNameRole: {
        return scope->master.location.filename;
    } break;

    case CursorOffsetRole: {
        return scope->master.location.position;
    } break;

    default:
        return QVariant();
    }
    return QVariant();
}

Qt::ItemFlags DlangOutlineModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
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

    {
    ModelResetGuard resetGuard(this);

    QTC_ASSERT(m_editor && m_editor->textDocument() && m_editor->document(), return);

    m_documentState.filePath = m_editor->textDocument()->filePath();
    m_documentState.rev = m_editor->document()->revision();

    try {
        DCodeModel::IModelSharedPtr model = DCodeModel::Factory::instance().getModel();
        model->getCurrentDocumentSymbols(m_editor->textDocument()->plainText(), m_scope);
    } catch (...) {
        return;
    }

    m_scope = DCodeModel::toTree(m_scope);
    m_scope.fixParents();
    }
    emit modelUpdated();
}


DlangOutlineModel::ModelResetGuard::ModelResetGuard(DlangOutlineModel *model)
    : m_model(model)
{
    m_model->beginResetModel();
}

DlangOutlineModel::ModelResetGuard::~ModelResetGuard()
{
    m_model->endResetModel();
}
