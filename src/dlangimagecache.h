#ifndef DLANGIMAGECACHE_H
#define DLANGIMAGECACHE_H

#include "codemodel/dmodel.h"

#include <QIcon>
#include <QVector>

namespace DlangEditor {

class DlangIconCache
{
public:

    DlangIconCache();

    const QIcon &fromType(DCodeModel::SymbolType type) const;

    static DlangIconCache &instance();

private:
    QVector<QIcon> mapping;
};

} // namespace DlangEditor

#endif // DLANGIMAGECACHE_H
