#ifndef DLANGOUTLINEMODEL_H
#define DLANGOUTLINEMODEL_H

#include <QAbstractItemModel>

#include <utils/fileutils.h>

namespace DCodeModel {
struct Scope;
}

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

    static const int NO_REVISION = -1;


    DlangOutlineModel(DlangTextEditorWidget *object = 0);
    const DCodeModel::Scope &scope() const;

    bool needUpdateForEditor() const;
    bool needUpdate(const Utils::FileName &filePath, int rev) const;

    const DCodeModel::Scope *byIndex(const QModelIndex &index) const;
    QModelIndex byCursor(int pos) const;
    bool getLocation(const QModelIndex &index, QString &filePath, int &offset) const;

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
    void update(const Utils::FileName &filename, int rev, const QString &sources);
    void updateForCurrentEditor();
signals:
    void modelUpdated();
private:
    void fillOffsets();
    void fix();
private:
    DCodeModel::Scope* m_scope;
    QMap<int, const DCodeModel::Scope*> m_offsets;
    struct DocumentState
    {
        Utils::FileName filePath;
        int rev;
        DocumentState() : rev(NO_REVISION) {}
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
