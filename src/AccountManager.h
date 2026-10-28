#ifndef ACCOUNT_MANAGER_H
#define ACCOUNT_MANAGER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSettings>
#include <QUuid>

struct ElyAccount {
    QString username;
    QString uuid;
    QString accessToken;
    QString clientToken;
    bool authenticated = false;
};

class AccountManager : public QObject
{
    Q_OBJECT
public:
    explicit AccountManager(QObject *parent = nullptr);
    ~AccountManager();

    bool isAuthenticated() const;
    ElyAccount currentAccount() const;
    QString clientToken() const;

    // Вход через ely.by (логин/пароль, опционально 2FA токен)
    void login(const QString &username, const QString &password, const QString &twoFactorToken = QString());

    // Выход
    void logout();

    // Загрузить сохранённый аккаунт из настроек
    void loadSavedAccount();

signals:
    void loginSucceeded(const ElyAccount &account);
    void loginFailed(const QString &errorMessage);
    void logoutCompleted();

private:
    QNetworkAccessManager *m_network;
    ElyAccount m_current;
    QString m_clientTokenFile;

    void saveAccount(const ElyAccount &acc);
    void clearSavedAccount();
    QString readClientToken();
};

#endif // ACCOUNT_MANAGER_H
