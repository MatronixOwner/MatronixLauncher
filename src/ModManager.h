#ifndef MOD_MANAGER_H
#define MOD_MANAGER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>

struct ModInfo {
    QString id;
    QString name;
    QString description;
    QString version;
    QString fileUrl;
    QString iconUrl;
    QString source; // "modrinth" или "curseforge"
    QStringList gameVersions;
    QStringList loaders; // "fabric", "forge"
};

class ModManager : public QObject
{
    Q_OBJECT
public:
    explicit ModManager(QObject *parent = nullptr);
    ~ModManager();

    // Поиск модов на Modrinth
    void searchModrinth(const QString &query, const QString &gameVersion = QString(),
                        const QString &loader = QString());

    // Поиск модов на CurseForge (требует API ключ — для MVP используется публичный поиск без ключа)
    void searchCurseForge(const QString &query);

signals:
    void modrinthResults(const QList<ModInfo> &mods);
    void modrinthFailed(const QString &error);
    void curseforgeResults(const QList<ModInfo> &mods);
    void curseforgeFailed(const QString &error);

private:
    QNetworkAccessManager *m_network;
};

#endif // MOD_MANAGER_H
