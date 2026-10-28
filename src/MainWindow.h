#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "AccountManager.h"
#include "VersionManager.h"
#include "ModManager.h"
#include "LaunchManager.h"

class QStackedWidget;
class QListWidget;
class QPushButton;
class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QFrame;
class QPropertyAnimation;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(AccountManager *accountManager, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoginClicked();
    void onLoginSucceeded(const ElyAccount &account);
    void onLoginFailed(const QString &error);
    void onLogoutClicked();
    void onVanillaVersionsFetched(const QList<MCVersion> &versions);
    void onVanillaVersionsFailed(const QString &error);
    void onFabricInstalled(const QString &mcVersion, const QString &installPath);
    void onFabricInstallFailed(const QString &error);
    void onForgeInstalled(const QString &mcVersion, const QString &installPath);
    void onForgeInstallFailed(const QString &error);
    void onLaunchClicked();
    void onGameStarted(int pid);
    void onGameExited(int code);
    void onLaunchFailed(const QString &error);
    void onModrinthResults(const QList<ModInfo> &mods);
    void onModrinthFailed(const QString &error);
    void onSearchMods();

    // Навигация
    void showPageVersions();
    void showPageMods();
    void showPageAccounts();
    void showPageSettings();

private:
    void setupUI();
    void applyStyles();
    void refreshVersionList();
    void animatePageSwitch();

    AccountManager *m_accountManager;
    VersionManager *m_versionManager;
    ModManager *m_modManager;
    LaunchManager *m_launchManager;

    QStackedWidget *m_stack;
    QFrame *m_sidebar;
    QListWidget *m_versionList;
    QLabel *m_accountLabel;
    QPushButton *m_loginButton;
    QPushButton *m_logoutButton;
    QPushButton *m_launchButton;
    QPushButton *m_installFabricButton;
    QPushButton *m_installForgeButton;
    QLineEdit *m_modSearchEdit;
    QListWidget *m_modList;

    QString m_selectedVersionId;
    bool m_isAuthenticated = false;
};

#endif // MAINWINDOW_H
