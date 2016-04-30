#ifndef DASTEDMODEL_H
#define DASTEDMODEL_H

#include "codemodel/dmodel.h"
#include "serverprocess.h"

#include <QObject>
#include <QStringList>
#include <QProcess>
#include <QMap>
#include <QVector>
#include <QSharedPointer>

#include <functional>

namespace Dasted {

const char DASTED_CODEMODEL_ID[] = "Dasted Code model";

class DastedServer : public DlangEditor::Utils::ServerDaemon
{
    Q_OBJECT
public:
    DastedServer(const QString &processName, int port, QObject *parent = 0);
    virtual ~DastedServer();
    int port() const;
private:
    int m_port;
    QStringList m_importPaths;
};

namespace Internal {
class ClientPrivate;
}

class DastedModel : public QObject, public DCodeModel::IModel
{
    Q_OBJECT
public:

    DastedModel(int port, bool autoStartServer,
                const QString &processName);

    void setPort(int port);
    int port() const;

    virtual ~DastedModel();
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
public slots:
    void onImportPathsUpdate(QString projectName, QStringList imports);
    void onServerError(QString error);

private:
    void startServer(const QString &processName, int port);

    Internal::ClientPrivate* d;
    QSharedPointer<DastedServer> m_server;
};

DCodeModel::SymbolType fromChar(unsigned char c);
DCodeModel::SymbolSubType subTypefromChar(unsigned char c);

} // namespace Dasted

#endif // DASTEDMODEL_H
