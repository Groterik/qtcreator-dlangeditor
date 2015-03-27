#ifndef DLANGOUTLINEMODEL_H
#define DLANGOUTLINEMODEL_H

#include <QAbstractItemModel>

#include "codemodel/dmodel.h"

namespace DlangEditor {

class DlangTextEditorWidget;

class DlangOutlineModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    enum Role {
        FileNameRole = Qt::UserRole + 1,
        CursorOffsetRole
    };


    DlangOutlineModel(DlangTextEditorWidget *object = 0);
    const DCodeModel::Scope &scope() const;
    bool needUpdate() const;
    const DCodeModel::Scope *byIndex(const QModelIndex &index) const;
    QModelIndex byCursor(int pos) const;

    // abstract QAbstractItemModel
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    // override
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    Qt::DropActions supportedDragActions() const Q_DECL_OVERRIDE;
    QStringList mimeTypes() const Q_DECL_OVERRIDE;
    QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;

public slots:
    void updateForEditor(DlangTextEditorWidget *editor);
    void update();
signals:
    void modelUpdated();
private:
    void fillOffsets();
    void fix();
private:
    DCodeModel::Scope m_scope;
    QMap<int, const DCodeModel::Scope*> m_offsets;
    struct DocumentState
    {
        QString filePath;
        int rev;
    } m_documentState;
    DlangTextEditorWidget *m_editor;

    class ModelResetGuard
    {
    public:
        ModelResetGuard(DlangOutlineModel *model);
        ~ModelResetGuard();

    private:
        DlangOutlineModel *m_model;
    };
};


}

#endif // DLANGOUTLINEMODEL_H
