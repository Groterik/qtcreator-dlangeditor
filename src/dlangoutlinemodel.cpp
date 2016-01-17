#include "dlangoutlinemodel.h"

#include "codemodel/dmodel.h"

#include "dlangeditorutils.h"
#include <utils/qtcassert.h>
#include <utils/fileutils.h>
#if QTCREATOR_MINOR_VERSION >= 5
#  include <utils/dropsupport.h>
#endif


#include "dlangeditor.h"
#include "dlangimagecache.h"

using namespace DlangEditor;
using namespace DCodeModel;

DlangOutlineModel::DlangOutlineModel(DlangTextEditorWidget *object)
    : QAbstractItemModel(object), m_editor(object)
{
    m_scope = new DCodeModel::Scope;
    fix();
}

const DCodeModel::Scope &DlangOutlineModel::scope() const
{
    return *m_scope;
}

bool DlangOutlineModel::needUpdateForEditor() const
{
    QTC_ASSERT(m_editor && m_editor->document(), return false);
#if QTCREATOR_MINOR_VERSION < 4
    return needUpdate(Utils::FileName::fromString(m_editor->textDocument()->filePath()), m_editor->document()->revision());
#else
    return needUpdate(m_editor->textDocument()->filePath(), m_editor->document()->revision());
#endif
}

bool DlangOutlineModel::needUpdate(const ::Utils::FileName &filePath, int rev) const
{
    return rev != m_documentState.rev || filePath != m_documentState.filePath;
}

const Scope *DlangOutlineModel::byIndex(const QModelIndex &index) const
{
    if (!index.isValid() || !index.internalPointer()) {
        return 0;
    }
    return reinterpret_cast<const Scope*>(index.internalPointer());
}

QModelIndex DlangOutlineModel::byCursor(int pos) const
{
    auto it = m_offsets.upperBound(pos);
    if (it == m_offsets.begin()) {
        return QModelIndex();
    }
    --it;
    DCodeModel::Scope *scope = const_cast<DCodeModel::Scope*>(it.value());
    return createIndex(it.value()->index, 0, scope);
}

bool DlangOutlineModel::getLocation(const QModelIndex &index, QString &filePath, int &offset) const
{
    const auto *scope = byIndex(index);
    if (!scope) {
        return false;
    }
    filePath = scope->symbol.location.filename;
    offset = scope->symbol.location.position;
    return true;
}

QModelIndex DlangOutlineModel::index(int row, int column, const QModelIndex &parent) const
{
    const Scope *scope = parent.isValid() ? reinterpret_cast<const Scope*>(parent.internalPointer()) : m_scope;
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
    const Scope *scope = parent.isValid() ? reinterpret_cast<const Scope*>(parent.internalPointer()) : m_scope;
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
    const Scope *scope = byIndex(index);
    if (!scope) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole: {
        return DCodeModel::toOutlineString(scope->symbol);
    } break;

    case Qt::EditRole: {
        return DCodeModel::toOutlineString(scope->symbol);
    } break;

    case Qt::DecorationRole: {
        return DlangIconCache::instance().fromType(scope->symbol.type);
    } break;

    case FileNameRole: {
        return scope->symbol.location.filename;
    } break;

    case CursorOffsetRole: {
        return scope->symbol.location.position;
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

Qt::DropActions DlangOutlineModel::supportedDragActions() const
{
    return Qt::MoveAction;
}

QStringList DlangOutlineModel::mimeTypes() const
{
#if QTCREATOR_MINOR_VERSION < 4
    return ::Utils::FileDropSupport::mimeTypesForFilePaths();
#else
    return ::Utils::DropSupport::mimeTypesForFilePaths();
#endif
}

QMimeData *DlangOutlineModel::mimeData(const QModelIndexList &indexes) const
{
#if QTCREATOR_MINOR_VERSION < 4
    auto mimeData = new ::Utils::FileDropMimeData;
#else
    auto mimeData = new ::Utils::DropMimeData;
#endif
    foreach (const QModelIndex &index, indexes) {
        const QVariant fileName = data(index, FileNameRole);
        if (!fileName.canConvert<QString>())
            continue;
        const QVariant offset = data(index, CursorOffsetRole);
        if (!offset.canConvert<int>())
            continue;
        int line = 0;
        int column = 0;
        m_editor->convertPosition(offset.value<int>(), &line, &column);
        mimeData->addFile(fileName.toString(), line, column);
    }
    return mimeData;
}

void DlangOutlineModel::updateForEditor(DlangTextEditorWidget *editor)
{
    m_editor = editor;
    updateForCurrentEditor();
}

void DlangOutlineModel::update(const ::Utils::FileName &filename, int rev, const QString &sources)
{
    if (!needUpdate(filename, rev)) {
        return;
    }

    {
        ModelResetGuard resetGuard(this);

        QTC_ASSERT(m_editor && m_editor->textDocument() && m_editor->document(), return);

        m_documentState.filePath = filename;
        m_documentState.rev = rev;

        try {
            DCodeModel::IModelSharedPtr model = DCodeModel::ModelManager::instance().getCurrentModel();
            model->getCurrentDocumentSymbols(Utils::currentProjectName(),
                                             sources, *m_scope);
        } catch (...) {
            fix();
            return;
        }
        fix();
    }
    emit modelUpdated();
}

void DlangOutlineModel::updateForCurrentEditor()
{
    QTC_ASSERT(m_editor && m_editor->textDocument() && m_editor->document(), return);
#if QTCREATOR_MINOR_VERSION < 4
    update(Utils::FileName::fromString(m_editor->textDocument()->filePath()), m_editor->document()->revision(),
                  m_editor->textDocument()->plainText());
#else
    update(m_editor->textDocument()->filePath(), m_editor->document()->revision(),
                  m_editor->textDocument()->plainText());
#endif
}

static void fillOffsets(const DCodeModel::Scope &s, QMap<int, const DCodeModel::Scope*>& offsets)
{
    for (auto &c : s.children) {
        offsets.insert(c.symbol.location.position, &c);
        fillOffsets(c, offsets);
    }
}

void DlangOutlineModel::fillOffsets()
{
    m_offsets.clear();
    ::fillOffsets(*m_scope, m_offsets);
}

void DlangOutlineModel::fix()
{
    if (m_scope->children.empty()) {
        DCodeModel::Scope s;
        s.symbol.name = "<No Symbols>";
        s.symbol.type = DCodeModel::SYMBOL_NO_TYPE;
        m_scope->children.push_back(s);
    }
    m_scope->fixParents();
    fillOffsets();
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
