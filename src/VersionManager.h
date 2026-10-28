#ifndef VERSION_MANAGER_H
#define VERSION_MANAGER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QStandardPaths>
#include <QDir>

struct MCVersion {
    QString id;           //例如 "1.20.1", "1.21"
    QString type;         // "release", "snapshot", "old_beta", "old_alpha"
    QString releaseTime;
    QString downloadUrl;  // клиент jar URL (для Vanilla)
    bool isVanilla = true;
};

struct InstallerInfo {
    QString installerType; // "fabric" или "forge"
    QString version;       // версия loadera
    QString mcVersion;
    QString downloadUrl;
};

class VersionManager : public QObject
{
    Q_OBJECT
public:
    explicit VersionManager(QObject *parent = nullptr);
    ~VersionManager();

    QString versionsDir() const;

    // Получить все версии Minecraft (Vanilla) с launchmeta/mojang
    void fetchVanillaVersions();

    // Установить Fabric для указанной версии MC
    void installFabric(const QString &mcVersion);

    // Установить Forge для указанной версии MC
    void installForge(const QString &mcVersion, const QString &forgeVersion = QString());

    // Путь к папке экземпляра игры
    QString instanceDir(const QString &versionId) const;

signals:
    void vanillaVersionsFetched(const QList<MCVersion> &versions);
    void vanillaVersionsFailed(const QString &error);
    void fabricInstalled(const QString &mcVersion, const QString &installPath);
    void fabricInstallFailed(const QString &error);
    void forgeInstalled(const QString &mcVersion, const QString &installPath);
    void forgeInstallFailed(const QString &error);
    void installProgress(int percent, const QString &status);

private:
    QNetworkAccessManager *m_network;
    QString m_versionsRoot;

    void downloadFile(const QUrl &url, const QString &destPath);
    void onNetworkFinished(QNetworkReply *reply);
};

#endif // VERSION_MANAGER_H
