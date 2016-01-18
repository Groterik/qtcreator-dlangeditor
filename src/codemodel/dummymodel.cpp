#include "dummymodel.h"

#include "codemodel/dmodel.h"
#include "codemodel/dmodeloptions.h"

#include <QLabel>
#include <QVBoxLayout>

using namespace DCodeModel;

DummyModel::DummyModel(QObject *parent) : QObject(parent)
{

}

DCodeModel::ModelId DCodeModel::DummyModel::id() const
{
    return DUMMY_MODEL;
}

void DCodeModel::DummyModel::complete(const QString &projectName, const QString &source,
                                      int position, DCodeModel::CompletionList &result)
{
    Q_UNUSED(projectName)
    Q_UNUSED(source)
    Q_UNUSED(position)
    Q_UNUSED(result)
}

void DCodeModel::DummyModel::appendIncludePaths(const QString &projectName,
                                                const QStringList &includePaths)
{
    Q_UNUSED(projectName)
    Q_UNUSED(includePaths)
}

void DCodeModel::DummyModel::getDocumentationComments(const QString &projectName,
                                                      const QString &sources, int position,
                                                      QStringList &result)
{
    Q_UNUSED(projectName)
    Q_UNUSED(sources)
    Q_UNUSED(position)
    Q_UNUSED(result)
}

void DCodeModel::DummyModel::findSymbolLocation(const QString &projectName, const QString &sources,
                                                int position, DCodeModel::Symbol &result)
{
    Q_UNUSED(projectName)
    Q_UNUSED(sources)
    Q_UNUSED(position)
    Q_UNUSED(result)
}

void DCodeModel::DummyModel::getSymbolsByName(const QString &projectName, const QString &sources,
                                              const QString &name, DCodeModel::SymbolList &result)
{
    Q_UNUSED(projectName)
    Q_UNUSED(sources)
    Q_UNUSED(name)
    Q_UNUSED(result)
}

void DCodeModel::DummyModel::getCurrentDocumentSymbols(const QString &projectName,
                                                       const QString &sources,
                                                       DCodeModel::Scope &result)
{
    Q_UNUSED(projectName)
    Q_UNUSED(sources)
    Q_UNUSED(result)
}

DCodeModel::DummyModelOptionsPageWidget::DummyModelOptionsPageWidget(QWidget *parent)
    : IModelOptionsWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    auto label = new QLabel("Does not do anything useful.");
    layout->addWidget(label);
}

void DCodeModel::DummyModelOptionsPageWidget::apply()
{
    // do nothing
}
