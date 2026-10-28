#include "LaunchManager.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

LaunchManager::LaunchManager(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus) {
        emit gameExited(code);
    });
    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError err){
        Q_UNUSED(err)
        emit launchFailed(m_process->errorString());
    });
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this](){
        emit launchOutput(QString::fromUtf8(m_process->readAllStandardOutput()));
    });
    connect(m_process, &QProcess::readyReadStandardError, this, [this](){
        emit launchOutput(QString::fromUtf8(m_process->readAllStandardError()));
    });
}

LaunchManager::~LaunchManager() = default;

bool LaunchManager::isRunning() const
{
    return m_process->state() == QProcess::Running;
}

void LaunchManager::launch(const QString &versionId,
                           const QString &instanceDir,
                           const QString &javaPath,
                           int memoryMB,
                           const ElyAccount &account,
                           const QStringList &extraJvmArgs)
{
    Q_UNUSED(versionId)
    QStringList arguments;

    // Папка игры
    arguments << "--workDir" << instanceDir;

    // JVM аргументы
    arguments << "-Xmx" + QString::number(memoryMB) + "M";
    arguments << "-Xms" + QString::number(memoryMB / 2) + "M";
    for (const auto &arg : extraJvmArgs) arguments << arg;

    // authlib-injector для ely.by (skins/sessions)
    arguments << "-javaagent:" + QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
              + "/authlib-injector.jar=ely.by";

    // Основной класс и аргументы игры (упрощённо — для полной версии нужно парсить JSON версии)
    arguments << "--username" << account.username;
    arguments << "--uuid" << account.uuid;
    arguments << "--accessToken" << account.accessToken;
    arguments << "--clientToken" << account.clientToken;
    arguments << "--version" << versionId;

    m_process->setWorkingDirectory(instanceDir);
    m_process->start(javaPath, arguments);

    if (!m_process->waitForStarted(5000)) {
        emit launchFailed("Не удалось запустить Java");
        return;
    }

    emit gameStarted(m_process->processId());
}
