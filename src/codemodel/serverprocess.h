#ifndef SERVERPROCESS_H
#define SERVERPROCESS_H

#include <QProcess>
#include <QStringList>

namespace DlangEditor {
namespace Utils {

class ServerDaemon : public QObject
{
    Q_OBJECT
public:
    ServerDaemon(QObject* parent = 0, const QString &processName = QString(), const QStringList &args = QStringList());
    virtual ~ServerDaemon();

    const QStringList &arguments() const;
    void setArguments(const QStringList &args);
    void setOutputFile(const QString& filePath);

    void start();
    void stop();
    bool isRunning() const;
signals:
    /**
     * @brief The signal is emitted when an error occurs with the dcd-server or
     * dcd-server exits
     */
    void error(QString);
    void finished();
private slots:
    void onFinished(int errorCode);
    void onError(QProcess::ProcessError error);
private:
    QString m_processName;
    QStringList m_args;
    QProcess m_process;
    QString m_filePath;
};

} // namespace Utils
} // namespace DlangEditor

#endif // SERVERPROCESS_H
