#ifndef DLANGUSESELECTIONUPDATER_H
#define DLANGUSESELECTIONUPDATER_H

#include <QObject>
#include <QTimer>
#include <QScopedPointer>
#include <QFutureWatcher>
#include <QTextEdit>

#include "dcdsupport.h"

namespace DlangEditor {

class DlangTextEditorWidget;

struct UseSelectionResult
{
    QTextDocument* docPtr;
    QString docPath;
    int rev;
    int pos;
    QString symbol;
    Dcd::Client::SymbolList list;
};

class DlangUseSelectionUpdater : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DlangUseSelectionUpdater)
public:
    explicit DlangUseSelectionUpdater(DlangTextEditorWidget *editor = 0);
    ~DlangUseSelectionUpdater();

    enum CallType { Synchronous, Asynchronous };

signals:

public slots:
    void scheduleUpdate();
    void abortSchedule();
    void update(CallType callType = Asynchronous);

private slots:
    void onFindUsesFinished();

private:

    void updateSynchronously();
    void updateAsynchronously();
    void processResults(const UseSelectionResult& result);

    typedef QList<QTextEdit::ExtraSelection> ExtraSelections;

    DlangTextEditorWidget* m_editorWidget;
    QTimer m_timer;
    QScopedPointer<QFutureWatcher<UseSelectionResult>> m_findUsesWatcher;
};

} // namespace DlangEditor

#endif // DLANGUSESELECTIONUPDATER_H
