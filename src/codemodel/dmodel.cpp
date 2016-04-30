#include "codemodel/dmodel.h"

using namespace DCodeModel;


ModelManager &ModelManager::instance()
{
    static ModelManager inst;
    return inst;
}

IModelSharedPtr ModelManager::getModelById(const QString &id) const
{
    auto it = m_storages.find(id);
    return it == m_storages.end() ? IModelSharedPtr() : it.value()->model();
}

IModelSharedPtr ModelManager::getCurrentModel() const
{
    return m_currentModelStorage ? m_currentModelStorage->model() : IModelSharedPtr();
}

bool ModelManager::registerModelStorage(ModelId id, QSharedPointer<IModelStorage> m, QString *errorString)
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
    FunctorModelStorage(ModelManager::ModelCreator m, ModelManager::WidgetCreator w) : m(m), w(w) {}
    virtual IModelSharedPtr model() Q_DECL_OVERRIDE
    {
        return m();
    }

    virtual IModelOptionsWidget *widget() Q_DECL_OVERRIDE
    {
        return w();
    }

private:
    ModelManager::ModelCreator m;
    ModelManager::WidgetCreator w;
};


bool ModelManager::registerModelStorage(ModelId id, ModelManager::ModelCreator m, ModelManager::WidgetCreator w, QString *errorString)
{
    QSharedPointer<IModelStorage> ptr(new FunctorModelStorage(m, w));
    return registerModelStorage(id, ptr, errorString);
}

bool ModelManager::setCurrentModel(ModelId id, QString *errorString)
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

QList<ModelId> ModelManager::modelIds() const
{
    return m_storages.keys();
}

ModelId ModelManager::currentModelId() const
{
    return m_currentId;
}

IModelStorage *ModelManager::modelStorage(const QString &id) const
{
    auto it = m_storages.find(id);
    if (it == m_storages.end()) {
        throw std::runtime_error("bad model id");
    }
    return it.value().data();
}

void ModelManager::onImportPathsUpdate(QString projectName, QStringList imports)
{
    return getCurrentModel()->appendIncludePaths(projectName, imports);
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
    const int childrenCount = children.size();
    for (int i = 0; i < childrenCount; ++i) {
        children[i].parent = this;
        children[i].index = i;
        children[i].fixParents();
    }
}

QString DCodeModel::toOutlineString(const Symbol &symbol)
{
    if (symbol.type == SYMBOL_BLOCK) {
        switch (symbol.subType) {
        default: return "-";
        case SYMBOL_IN: return "in";
        case SYMBOL_OUT: return "out";
        case SYMBOL_SCOPE: return "-";
        case SYMBOL_UNITTEST: return "unittest";
        }
        return "";
    }
    QString result = symbol.name;
    if (!symbol.templateParameters.isEmpty()) {
        result += "(" + symbol.templateParameters + ")";
    }
    if (!symbol.parameters.isEmpty()) {
        result += "(" + symbol.parameters + ")";
    }
    if (!symbol.qualifiers.isEmpty()) {
        result += " " + symbol.qualifiers;
    }
    if (!symbol.typeName.isEmpty()) {
        result += ": " + symbol.typeName;
    }
    return result;
}
