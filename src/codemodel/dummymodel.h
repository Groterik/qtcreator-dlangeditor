#ifndef DUMMYMODEL_H
#define DUMMYMODEL_H

#include "codemodel/dmodel.h"
#include "codemodel/dmodeloptions.h"

namespace DCodeModel {

const char DUMMY_MODEL[] = "Dummy D Code Model";

class DummyModel : public QObject, public DCodeModel::IModel
{
    Q_OBJECT
public:
    explicit DummyModel(QObject *parent = 0);

    virtual DCodeModel::ModelId id() const Q_DECL_OVERRIDE;
    void complete(const QString &projectName,
                  const DCodeModel::Sources &sources,
                  int position,
                  DCodeModel::CompletionList &result) Q_DECL_OVERRIDE;
    void appendIncludePaths(const QString &projectName,
                            const QStringList &includePaths) Q_DECL_OVERRIDE;
    void getDocumentationComments(const QString &projectName,
                                  const DCodeModel::Sources &sources,
                                  int position,
                                  QStringList &result) Q_DECL_OVERRIDE;
    void findSymbolLocation(const QString &projectName,
                            const DCodeModel::Sources &sources,
                            int position,
                            DCodeModel::Symbol &result) Q_DECL_OVERRIDE;
    void getSymbolsByName(const QString &projectName,
                          const DCodeModel::Sources &sources,
                          const QString &name,
                          DCodeModel::SymbolList &result) Q_DECL_OVERRIDE;
    void getCurrentDocumentSymbols(const QString &projectName,
                                   const DCodeModel::Sources &sources,
                                   DCodeModel::Scope &result) Q_DECL_OVERRIDE;

signals:

public slots:
};

class DummyModelOptionsPageWidget : public DCodeModel::IModelOptionsWidget
{
    Q_OBJECT
public:
    DummyModelOptionsPageWidget(QWidget *parent = 0);

    // pure virtual
    void apply() Q_DECL_OVERRIDE;
};

} // namespace DCodeModel

#endif // DUMMYMODEL_H
