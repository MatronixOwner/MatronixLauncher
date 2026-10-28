#include "ModManager.h"
#include <QJsonDocument>
#include <QUrlQuery>

ModManager::ModManager(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{
    connect(m_network, &QNetworkAccessManager::finished,
            this, &ModManager::onNetworkFinished);
}

ModManager::~ModManager() = default;

void ModManager::searchModrinth(const QString &query, const QString &gameVersion, const QString &loader)
{
    QUrl url("https://api.modrinth.com/v2/search");
    QUrlQuery queryParams;
    queryParams.addQueryItem("query", query);
    queryParams.addQueryItem("limit", "20");
    if (!gameVersion.isEmpty()) queryParams.addQueryItem("versions", gameVersion);
    if (!loader.isEmpty()) queryParams.addQueryItem("loaders", loader);
    url.setQuery(queryParams);

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "MatronixLauncher/1.0");
    request.setRawHeader("Accept", "application/json");
    m_network->get(request);
}

void ModManager::searchCurseForge(const QString &query)
{
    Q_UNUSED(query)
    // CurseForge API требует ключ (key). Для MVP возвращаем заглушку с предложением ввести ключ.
    emit curseforgeFailed("CurseForge API требует личный ключ API. Установите его в настройках лаунчера.");
}

void ModManager::onNetworkFinished(QNetworkReply *reply)
{
    QUrl url = reply->url();
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data = reply->readAll();
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError || httpStatus != 200) {
        if (url.host().contains("modrinth")) {
            emit modrinthFailed(reply->errorString());
        } else {
            emit curseforgeFailed(reply->errorString());
        }
        return;
    }

    if (url.host().contains("modrinth.com")) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isObject()) {
            emit modrinthFailed("Некорректный ответ Modrinth");
            return;
        }
        QJsonObject root = doc.object();
        QJsonArray hits = root["hits"].toArray();

        QList<ModInfo> mods;
        for (const auto &hit : hits) {
            QJsonObject h = hit.toObject();
            ModInfo mod;
            mod.id = h["project_id"].toString();
            mod.name = h["title"].toString();
            mod.description = h["description"].toString();
            mod.source = "modrinth";
            mod.version = h["latest_version"].toString();
            mod.gameVersions = h["game_versions"].toArray().toVariantList().toStringList();
            mod.loaders = h["loaders"].toArray().toVariantList().toStringList();
            mods.append(mod);
        }
        emit modrinthResults(mods);
    }
    else {
        emit curseforgeFailed("Неизвестный источник");
    }
}
