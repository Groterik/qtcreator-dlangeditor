#ifndef DMODEL_H
#define DMODEL_H

#include <functional>

#include <QString>
#include <QMap>
#include <QList>
#include <QVector>
#include <QSharedPointer>
#include <QWidget>

namespace DCodeModel {


struct Location {
    QString filename;
    int position;
    Location() {}
    Location(const QString& s, int line) : filename(s), position(line) {}
    bool isNull() const {
        return filename.isNull() || filename.isEmpty();
    }
};

enum SymbolType {
    SYMBOL_NO_TYPE = 0,
    SYMBOL_ENUM_VAR, SYMBOL_VAR, SYMBOL_CLASS, SYMBOL_INTERFACE,
    SYMBOL_STRUCT, SYMBOL_UNION, SYMBOL_MEMBER_VAR, SYMBOL_KEYWORD, SYMBOL_FUNCTION,
    SYMBOL_ENUM_NAME, SYMBOL_PACKAGE, SYMBOL_MODULE, SYMBOL_ARRAY, SYMBOL_ASSOC_ARRAY,
    SYMBOL_ALIAS, SYMBOL_TEMPLATE, SYMBOL_MIXIN,
    SYMBOL_IDENTIFIER_TYPE_SIZE
};

SymbolType fromString(QChar c);

enum CompletionType {
    COMPLETION_BAD_TYPE = 0,
    COMPLETION_IDENTIFIER, COMPLETION_CALLTIP,
    COMPLETION_TYPE_SIZE
};

struct Symbol
{
    SymbolType type;
    QString data;
    Location location;
};

typedef QList<Symbol> SymbolList;
typedef QVector<Symbol> SymbolVector;

struct CompletionList
{
    CompletionType type;
    SymbolList list;
};

typedef QString ModelId;

class IModel
{
public:

    virtual ~IModel() {}
    virtual IModel *copy() const = 0;
    virtual ModelId id() const = 0;
    /**
     * @brief Complete by position in the file
     * @param source
     * @param position
     * @param[out] result result of completion (may be empty, of course)
     * @return throws on error
     */
    virtual void complete(const QString &source, int position, CompletionList &result) = 0;
    /**
     * @brief Send request to dcd-server to add include path
     * @param includePath
     * @return throws on error
     */
    virtual void appendIncludePaths(const QStringList &includePaths) = 0;
    /**
     * @brief Gets documentation comments
     * @param sources
     * @param position
     * @param[out] result string list of documentation comments
     * @return throws on error
     */
    virtual void getDocumentationComments(const QString &sources, int position, QStringList &result) = 0;
    /**
     * @brief Gets symbols by name
     * @param sources
     * @param position
     * @param[out] result string list of documentation comments
     * @return throws on error
     */
    virtual void findSymbolLocation(const QString &sources, int position, Symbol &result) = 0;

    /**
     * @brief Gets symbols by name
     * @param sources
     * @param name
     * @param[out] result string list of documentation comments
     * @return throws on error
     */
    virtual void getSymbolsByName(const QString &sources, const QString &name, SymbolList &result) = 0;

    /**
     * @brief Gets current document symbols
     * @param sources
     * @param[out] result string list of documentation comments
     * @return throws on error
     */
    virtual void getCurrentDocumentSymbols(const QString &sources, SymbolList &result) = 0;
};

typedef QSharedPointer<IModel> IModelSharedPtr;

class IModelStorage
{
public:
    virtual ~IModelStorage() {}

    virtual IModelSharedPtr model() = 0;
    virtual QWidget *widget() = 0;
};

class Factory : public QObject
{
    Q_OBJECT
public:
    Factory() {}

    typedef std::function<IModelSharedPtr()> ModelCreator;
    typedef std::function<QWidget*()> WidgetCreator;

    static Factory &instance();

    IModelSharedPtr getModelById(const QString &id) const;
    IModelSharedPtr getModel() const;
    bool registerModelStorage(ModelId id, QSharedPointer<IModelStorage> m, QString *errorString);
    bool registerModelStorage(ModelId id, ModelCreator m, WidgetCreator w, QString *errorString);
    bool setCurrentModel(ModelId id, QString *errorString);

    QList<ModelId> modelIds() const;

signals:
    void updated();

private:
    ModelId m_currentId;
    QSharedPointer<IModelStorage> m_currentModelStorage;
    QMap<ModelId, QSharedPointer<IModelStorage> > m_storages;
};

QPair<int, int> findSymbol(const QString& text, int pos);

} // namespace DCodeModel

#endif //DCDMODEL_H
