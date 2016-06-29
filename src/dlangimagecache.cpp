#include "dlangimagecache.h"

#include <cplusplus/Icons.h>

#include <QAtomicPointer>
#include <QMutexLocker>

using namespace DlangEditor;
using CPlusPlus::Icons;

const QIcon &DlangIconCache::fromType(DCodeModel::SymbolType type) const {
    if (type < 0 || type >= mapping.size()) {
        return mapping[DCodeModel::SymbolType::SYMBOL_NO_TYPE];
    }
    return mapping.at(type);
}

static QAtomicPointer<DlangIconCache> staticInstance = 0;
QMutex staticInstanceMutex;

DlangIconCache &DlangIconCache::instance()
{
    if (!(staticInstance.load())) {
        QMutexLocker lock(&staticInstanceMutex);
        if (!(staticInstance.load())) {
            staticInstance = new DlangIconCache;
        }
    }
    return *(staticInstance.load());
}

DlangIconCache::DlangIconCache() {
    mapping.resize(DCodeModel::SymbolType::SYMBOL_IDENTIFIER_TYPE_SIZE);
    mapping[DCodeModel::SymbolType::SYMBOL_ALIAS] = QIcon(QLatin1String(":/dlangeditor/images/alias.png"));
    mapping[DCodeModel::SymbolType::SYMBOL_ARRAY] = QIcon(QLatin1String(":/dlangeditor/images/array.png"));
    mapping[DCodeModel::SymbolType::SYMBOL_ASSOC_ARRAY] = QIcon(QLatin1String(":/dlangeditor/images/assoc_array.png"));
    mapping[DCodeModel::SymbolType::SYMBOL_CLASS] = Icons::iconForType(Icons::ClassIconType);
    mapping[DCodeModel::SymbolType::SYMBOL_ENUM_NAME] = Icons::iconForType(Icons::EnumIconType);
    mapping[DCodeModel::SymbolType::SYMBOL_ENUM_VAR] = Icons::iconForType(Icons::EnumeratorIconType);
    mapping[DCodeModel::SymbolType::SYMBOL_FUNCTION] = Icons::iconForType(Icons::FuncPublicIconType);
    mapping[DCodeModel::SymbolType::SYMBOL_INTERFACE] = QIcon(QLatin1String(":/dlangeditor/images/interface.png"));
    mapping[DCodeModel::SymbolType::SYMBOL_KEYWORD] = Icons::keywordIcon();
    mapping[DCodeModel::SymbolType::SYMBOL_MEMBER_VAR] = QIcon(QLatin1String(":/dlangeditor/images/member_var.png"));
    mapping[DCodeModel::SymbolType::SYMBOL_MIXIN] = QIcon(QLatin1String(":/dlangeditor/images/mixin.png"));
    mapping[DCodeModel::SymbolType::SYMBOL_MODULE] = Icons::iconForType(Icons::NamespaceIconType);
    mapping[DCodeModel::SymbolType::SYMBOL_PACKAGE] = QIcon(QLatin1String(":/core/images/dir.png"));
    mapping[DCodeModel::SymbolType::SYMBOL_STRUCT] = QIcon(QLatin1String(":/dlangeditor/images/struct.png"));
    mapping[DCodeModel::SymbolType::SYMBOL_TEMPLATE] = QIcon(QLatin1String(":/dlangeditor/images/template.png"));
    mapping[DCodeModel::SymbolType::SYMBOL_UNION] = QIcon(QLatin1String(":/dlangeditor/images/union.png"));
    mapping[DCodeModel::SymbolType::SYMBOL_VAR] = Icons::iconForType(Icons::VarPublicIconType);
    mapping[DCodeModel::SymbolType::SYMBOL_BLOCK] = QIcon();
}
