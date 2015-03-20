#include "codemodel/dmodel.h"

using namespace DCodeModel;


Factory &Factory::instance()
{
    static Factory inst;
    return inst;
}

IModelSharedPtr Factory::getModelById(const QString &id) const
{
    auto it = m_storages.find(id);
    return it == m_storages.end() ? IModelSharedPtr() : it.value()->model();
}

IModelSharedPtr Factory::getModel() const
{
    return m_currentModelStorage ? m_currentModelStorage->model() : IModelSharedPtr();
}

bool Factory::registerModelStorage(ModelId id, QSharedPointer<IModelStorage> m, QString *errorString)
{
    if (!m) {
        if (errorString) {
            *errorString = "bad model (null pointer)";
        }
        return false;
    }
    auto it = m_storages.find(id);
    if (it != m_storages.end()) {
        if (errorString) {
            *errorString = "id already registered";
        }
        return false;
    }
    m_storages.insert(id, m);
    emit updated();
    return true;
}

class FunctorModelStorage : public IModelStorage
{
public:
    FunctorModelStorage(Factory::ModelCreator m, Factory::WidgetCreator w) : m(m), w(w) {}
    virtual IModelSharedPtr model() Q_DECL_OVERRIDE
    {
        return m();
    }

    virtual IModelOptionsWidget *widget() Q_DECL_OVERRIDE
    {
        return w();
    }

private:
    Factory::ModelCreator m;
    Factory::WidgetCreator w;
};


bool Factory::registerModelStorage(ModelId id, Factory::ModelCreator m, Factory::WidgetCreator w, QString *errorString)
{
    QSharedPointer<IModelStorage> ptr(new FunctorModelStorage(m, w));
    return registerModelStorage(id, ptr, errorString);
}

bool Factory::setCurrentModel(ModelId id, QString *errorString)
{
    auto it = m_storages.find(id);
    if (it == m_storages.end()) {
        if (errorString) {
            *errorString = "model not found";
        }
        return false;
    }
    m_currentId = id;
    m_currentModelStorage = it.value();
    return true;
}

QList<ModelId> Factory::modelIds() const
{
    return m_storages.keys();
}

ModelId Factory::currentModelId() const
{
    return m_currentId;
}

IModelStorage *Factory::modelStorage(const QString &id) const
{
    auto it = m_storages.find(id);
    if (it == m_storages.end()) {
        throw std::runtime_error("bad model id");
    }
    return it.value().data();
}

inline bool isSymbolChar(QChar c)
{
    return !c.isNull() && (c.isLetterOrNumber() ||  c == QLatin1Char('_'));
}

QPair<int, int> DCodeModel::findSymbol(const QString &text, int pos)
{
    int bpos = pos - 1;
    for (; bpos >= 0 && isSymbolChar(text.at(bpos)); --bpos) {}
    int epos = pos;
    const int len = text.length();
    for (; epos < len && isSymbolChar(text.at(epos)); ++epos) {}
    return qMakePair(bpos + 1, epos);
}


Scope::Scope()
    : parent(0)
{

}

void Scope::fixParents()
{
    for (auto &c : children) {
        c.parent = this;
        c.fixParents();
    }
}
