#include "AccountManager.h"
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>

AccountManager::AccountManager(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    m_clientTokenFile = configDir + "/client_token";

    // Автоматически загружаем сохранённый аккаунт при старте
    loadSavedAccount();

    connect(m_network, &QNetworkAccessManager::finished,
            this, &AccountManager::onNetworkFinished);
}

AccountManager::~AccountManager() = default;

bool AccountManager::isAuthenticated() const
{
    return m_current.authenticated && !m_current.accessToken.isEmpty();
}

ElyAccount AccountManager::currentAccount() const
{
    return m_current;
}

QString AccountManager::clientToken()
{
    return readClientToken();
}

void AccountManager::loadSavedAccount()
{
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/accounts.ini",
                       QSettings::IniFormat);
    settings.beginGroup("ElyAccount");
    m_current.username = settings.value("username").toString();
    m_current.uuid = settings.value("uuid").toString();
    m_current.accessToken = settings.value("accessToken").toString();
    m_current.clientToken = settings.value("clientToken").toString();
    m_current.authenticated = settings.value("authenticated", false).toBool();
    settings.endGroup();
}

void AccountManager::saveAccount(const ElyAccount &acc)
{
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/accounts.ini",
                       QSettings::IniFormat);
    settings.beginGroup("ElyAccount");
    settings.setValue("username", acc.username);
    settings.setValue("uuid", acc.uuid);
    settings.setValue("accessToken", acc.accessToken);
    settings.setValue("clientToken", acc.clientToken);
    settings.setValue("authenticated", true);
    settings.endGroup();
}

void AccountManager::clearSavedAccount()
{
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/accounts.ini",
                       QSettings::IniFormat);
    settings.remove("ElyAccount");
    m_current = ElyAccount();
}

QString AccountManager::readClientToken()
{
    QFile file(m_clientTokenFile);
    if (file.open(QIODevice::ReadOnly)) {
        return QString::fromUtf8(file.readAll()).trimmed();
    }
    // Генерируем новый clientToken
    QString token = QUuid::createUuid().toString(QUuid::WithoutBraces);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(token.toUtf8());
        file.close();
    }
    return token;
}

void AccountManager::login(const QString &username, const QString &password, const QString &twoFactorToken)
{
    QString clientToken = readClientToken();

    QJsonObject body;
    body["username"] = username;
    body["password"] = password;
    if (!twoFactorToken.isEmpty()) {
        body["password"] = password + ":" + twoFactorToken;
    }
    body["clientToken"] = clientToken;
    body["requestUser"] = true;

    QJsonDocument doc(body);
    QNetworkRequest request(QUrl(QString(MATRONIX_AUTH_SERVER) + "/auth/authenticate"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("User-Agent", "MatronixLauncher/1.0");

    m_network->post(request, doc.toJson());
}

void AccountManager::logout()
{
    clearSavedAccount();
    m_current = ElyAccount();
    emit logoutCompleted();
}

void AccountManager::onNetworkFinished(QNetworkReply *reply)
{
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data = reply->readAll();
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit loginFailed(QString("Сеть: %1").arg(reply->errorString()));
        return;
    }

    if (httpStatus != 200) {
        QString msg = QString("Ошибка сервера ely.by (HTTP %1)").arg(httpStatus);
        emit loginFailed(msg);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        emit loginFailed("Некорректный ответ сервера ely.by");
        return;
    }

    QJsonObject obj = doc.object();
    if (obj.contains("error")) {
        emit loginFailed(obj["errorMessage"].toString(obj["error"].toString()));
        return;
    }

    ElyAccount acc;
    acc.username = obj["selectedProfile"].toObject()["name"].toString();
    acc.uuid = obj["selectedProfile"].toObject()["id"].toString();
    acc.accessToken = obj["accessToken"].toString();
    acc.clientToken = obj["clientToken"].toString();
    acc.authenticated = true;

    m_current = acc;
    saveAccount(acc);
    emit loginSucceeded(acc);
}
