#ifndef LAUNCH_MANAGER_H
#define LAUNCH_MANAGER_H

#include <QObject>
#include <QString>
#include <QProcess>
#include "AccountManager.h"

class LaunchManager : public QObject
{
    Q_OBJECT
public:
    explicit LaunchManager(QObject *parent = nullptr);
    ~LaunchManager();

    // Запустить Minecraft
    void launch(const QString &versionId,
                const QString &instanceDir,
                const QString &javaPath,
                int memoryMB,
                const ElyAccount &account,
                const QStringList &extraJvmArgs = QStringList());

    bool isRunning() const;

signals:
    void gameStarted(int pid);
    void gameExited(int exitCode);
    void launchFailed(const QString &error);
    void launchOutput(const QString &text);

private:
    QProcess *m_process;
};

#endif // LAUNCH_MANAGER_H
