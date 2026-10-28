#include "VersionManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QDebug>
#include <QStandardPaths>

VersionManager::VersionManager(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_versionsRoot = base + "/versions";
    QDir().mkpath(m_versionsRoot);

    connect(m_network, &QNetworkAccessManager::finished,
            this, &VersionManager::onNetworkFinished);
}

VersionManager::~VersionManager() = default;

QString VersionManager::versionsDir() const
{
    return m_versionsRoot;
}

QString VersionManager::instanceDir(const QString &versionId) const
{
    QString dir = m_versionsRoot + "/" + versionId;
    QDir().mkpath(dir);
    return dir;
}

void VersionManager::fetchVanillaVersions()
{
    // manifest с версиями Minecraft (официальный Mojang launchmeta)
    QUrl url("https://launchermeta.mojang.com/mc/game/version_manifest.json");
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "MatronixLauncher/1.0");
    m_network->get(request);
}

void VersionManager::installFabric(const QString &mcVersion)
{
    // Fabric installer API
    QString urlStr = QString("https://meta.fabricmc.net/v2/versions/loader/%1/1.0.0/server/url").arg(mcVersion);
    QUrl url(urlStr);
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "MatronixLauncher/1.0");
    m_network->get(request);
    // В реальности здесь нужно скачать installer.jar и запустить его с параметрами
    // Для MVP простоemit уведомление
    emit fabricInstalled(mcVersion, instanceDir("fabric-" + mcVersion));
}

void VersionManager::installForge(const QString &mcVersion, const QString &forgeVersion)
{
    Q_UNUSED(forgeVersion)
    // Forge через файлы файлов (files.minecraftforge.net) — нужна версия мавена
    // Для MVP emit уведомление
    emit forgeInstalled(mcVersion, instanceDir("forge-" + mcVersion));
}

void VersionManager::downloadFile(const QUrl &url, const QString &destPath)
{
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "MatronixLauncher/1.0");
    m_network->get(request);
    Q_UNUSED(destPath)
}

void VersionManager::onNetworkFinished(QNetworkReply *reply)
{
    QUrl url = reply->url();
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data = reply->readAll();
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError || httpStatus != 200) {
        if (url.toString().contains("fabricmc")) {
            emit fabricInstallFailed(reply->errorString());
        } else if (url.toString().contains("forge")) {
            emit forgeInstallFailed(reply->errorString());
        } else {
            emit vanillaVersionsFailed(reply->errorString());
        }
        return;
    }

    if (url.toString().contains("launchermeta.mojang.com")) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isObject()) {
            emit vanillaVersionsFailed("Некорректный манифест версий");
            return;
        }
        QJsonObject root = doc.object();
        QJsonArray versions = root["versions"].toArray();

        QList<MCVersion> list;
        for (const auto &v : versions) {
            QJsonObject vo = v.toObject();
            MCVersion mv;
            mv.id = vo["id"].toString();
            mv.type = vo["type"].toString();
            mv.releaseTime = vo["releaseTime"].toString();
            // downloadUrl будем получать отдельно через version manifest v2
            mv.downloadUrl = QString("https://launcher.mojang.com/v1/objects/%1/client.jar").arg(vo["id"].toString());
            mv.isVanilla = true;
            list.append(mv);
        }
        emit vanillaVersionsFetched(list);
    }
    else if (url.toString().contains("fabricmc")) {
        QString urlStr = QString::fromUtf8(data).trimmed();
        emit fabricInstalled(urlStr.isEmpty() ? "unknown" : "fabric", instanceDir("fabric-unknown"));
    }
    else if (url.toString().contains("forge")) {
        emit forgeInstalled("unknown", instanceDir("forge-unknown"));
    }
}
