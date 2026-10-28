#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QFrame>
#include <QStackedWidget>
#include <QLineEdit>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QSpacerItem>
#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>

MainWindow::MainWindow(AccountManager *accountManager, QWidget *parent)
    : QMainWindow(parent)
    , m_accountManager(accountManager)
    , m_versionManager(new VersionManager(this))
    , m_modManager(new ModManager(this))
    , m_launchManager(new LaunchManager(this))
    , m_isAuthenticated(false)
{
    setWindowTitle("MatronixLauncher");
    setMinimumSize(1000, 700);

    setupUI();
    applyStyles();

    // Подключаем сигналы аккаунта
    connect(m_accountManager, &AccountManager::loginSucceeded, this, &MainWindow::onLoginSucceeded);
    connect(m_accountManager, &AccountManager::loginFailed, this, &MainWindow::onLoginFailed);
    connect(m_accountManager, &AccountManager::logoutCompleted, this, &MainWindow::onLogoutClicked);

    // Версии
    connect(m_versionManager, &VersionManager::vanillaVersionsFetched, this, &MainWindow::onVanillaVersionsFetched);
    connect(m_versionManager, &VersionManager::vanillaVersionsFailed, this, &MainWindow::onVanillaVersionsFailed);
    connect(m_versionManager, &VersionManager::fabricInstalled, this, &MainWindow::onFabricInstalled);
    connect(m_versionManager, &VersionManager::fabricInstallFailed, this, &MainWindow::onFabricInstallFailed);
    connect(m_versionManager, &VersionManager::forgeInstalled, this, &MainWindow::onForgeInstalled);
    connect(m_versionManager, &VersionManager::forgeInstallFailed, this, &MainWindow::onForgeInstallFailed);

    // Запуск
    connect(m_launchManager, &LaunchManager::gameStarted, this, &MainWindow::onGameStarted);
    connect(m_launchManager, &LaunchManager::gameExited, this, &MainWindow::onGameExited);
    connect(m_launchManager, &LaunchManager::launchFailed, this, &MainWindow::onLaunchFailed);

    // Моды
    connect(m_modManager, &ModManager::modrinthResults, this, &MainWindow::onModrinthResults);
    connect(m_modManager, &ModManager::modrinthFailed, this, &MainWindow::onModrinthFailed);

    // Если уже есть сохранённый аккаунт — показываем его
    if (m_accountManager->isAuthenticated()) {
        onLoginSucceeded(m_accountManager->currentAccount());
    }

    // Загружаем версии при старте
    refreshVersionList();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ---------- Боковая панель ----------
    m_sidebar = new QFrame;
    m_sidebar->setFixedWidth(220);
    m_sidebar->setObjectName("sidebar");

    QVBoxLayout *sidebarLayout = new QVBoxLayout(m_sidebar);
    sidebarLayout->setSpacing(8);
    sidebarLayout->setContentsMargins(12, 24, 12, 24);

    QLabel *logo = new QLabel("MATRONIX");
    logo->setAlignment(Qt::AlignCenter);
    logo->setStyleSheet("font-size: 22px; font-weight: bold; letter-spacing: 2px; color: #ffffff;");
    sidebarLayout->addWidget(logo);

    QLabel *sub = new QLabel("Launcher v1.0.0");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("font-size: 11px; color: #888888;");
    sidebarLayout->addWidget(sub);

    sidebarLayout->addSpacing(20);

    auto addNavButton = [&](const QString &text) -> QPushButton* {
        QPushButton *btn = new QPushButton(text);
        btn->setMinimumHeight(44);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton { text-align: left; padding-left: 16px; background: transparent; border: none; "
            "color: #cccccc; font-size: 14px; }"
            "QPushButton:hover { background: #2a2a2a; color: #ffffff; }"
            "QPushButton:checked { background: #2a2a2a; border-left: 4px solid #4a90e2; color: #ffffff; }"
        );
        btn->setCheckable(true);
        sidebarLayout->addWidget(btn);
        return btn;
    };

    QPushButton *btnVersions = addNavButton("Версии");
    QPushButton *btnMods = addNavButton("Моды");
    QPushButton *btnAccounts = addNavButton("Аккаунт");
    QPushButton *btnSettings = addNavButton("Настройки");

    sidebarLayout->addStretch();

    m_loginButton = new QPushButton("Войти в Ely.by");
    m_loginButton->setMinimumHeight(40);
    m_loginButton->setStyleSheet(
        "QPushButton { background-color: #4a90e2; color: white; border: none; border-radius: 6px; font-weight: bold; }"
        "QPushButton:hover { background-color: #357abd; }");
    sidebarLayout->addWidget(m_loginButton);

    m_logoutButton = new QPushButton("Выйти");
    m_logoutButton->setMinimumHeight(40);
    m_logoutButton->setStyleSheet(
        "QPushButton { background-color: #c44545; color: white; border: none; border-radius: 6px; font-weight: bold; }"
        "QPushButton:hover { background-color: #a33a3a; }");
    m_logoutButton->hide();
    sidebarLayout->addWidget(m_logoutButton);

    m_accountLabel = new QLabel("Не авторизован");
    m_accountLabel->setAlignment(Qt::AlignCenter);
    m_accountLabel->setWordWrap(true);
    m_accountLabel->setStyleSheet("color: #888888; font-size: 12px;");
    sidebarLayout->addWidget(m_accountLabel);

    // Подключаем навигацию
    connect(btnVersions, &QPushButton::toggled, this, [this](bool checked){ if(checked) showPageVersions(); });
    connect(btnMods, &QPushButton::toggled, this, [this](bool checked){ if(checked) showPageMods(); });
    connect(btnAccounts, &QPushButton::toggled, this, [this](bool checked){ if(checked) showPageAccounts(); });
    connect(btnSettings, &QPushButton::toggled, this, [this](bool checked){ if(checked) showPageSettings(); });
    connect(m_loginButton, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(m_logoutButton, &QPushButton::clicked, m_accountManager, &AccountManager::logout);

    mainLayout->addWidget(m_sidebar);

    // ---------- Центральный стек ----------
    m_stack = new QStackedWidget;
    mainLayout->addWidget(m_stack, 1);

    // Страница: Версии
    QWidget *pageVersions = new QWidget;
    QVBoxLayout *vLayout = new QVBoxLayout(pageVersions);
    vLayout->setContentsMargins(24, 24, 24, 24);
    vLayout->setSpacing(16);

    QLabel *verTitle = new QLabel("Версии Minecraft");
    verTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #ffffff;");
    vLayout->addWidget(verTitle);

    QHBoxLayout *verControls = new QHBoxLayout;
    QPushButton *btnRefresh = new QPushButton("Обновить список");
    btnRefresh->setStyleSheet(
        "QPushButton { background-color: #3a3a3a; color: #ffffff; border: 1px solid #555555; border-radius: 6px; padding: 8px 16px; }"
        "QPushButton:hover { background-color: #4a4a4a; }");
    verControls->addWidget(btnRefresh);

    m_installFabricButton = new QPushButton("Установить Fabric");
    m_installFabricButton->setEnabled(false);
    m_installFabricButton->setStyleSheet(
        "QPushButton { background-color: #4a90e2; color: white; border: none; border-radius: 6px; padding: 8px 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #357abd; } QPushButton:disabled { background: #555555; }");
    verControls->addWidget(m_installFabricButton);

    m_installForgeButton = new QPushButton("Установить Forge");
    m_installForgeButton->setEnabled(false);
    m_installForgeButton->setStyleSheet(
        "QPushButton { background-color: #e67e22; color: white; border: none; border-radius: 6px; padding: 8px 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #d35400; } QPushButton:disabled { background: #555555; }");
    verControls->addWidget(m_installForgeButton);

    verControls->addStretch();
    vLayout->addLayout(verControls);

    m_versionList = new QListWidget;
    m_versionList->setStyleSheet(
        "QListWidget { background: #1e1e1e; border: 1px solid #333333; border-radius: 8px; color: #ffffff; font-size: 13px; }"
        "QListWidget::item { padding: 12px; border-bottom: 1px solid #2a2a2a; }"
        "QListWidget::item:selected { background: #2a4a7a; color: #ffffff; }"
        "QListWidget::item:hover { background: #2a2a2a; }");
    vLayout->addWidget(m_versionList, 1);

    m_launchButton = new QPushButton("ИГРАТЬ");
    m_launchButton->setMinimumHeight(52);
    m_launchButton->setEnabled(false);
    m_launchButton->setStyleSheet(
        "QPushButton { background-color: #27ae60; color: white; border: none; border-radius: 8px; "
        "font-size: 18px; font-weight: bold; }"
        "QPushButton:hover { background-color: #219a52; }"
        "QPushButton:disabled { background-color: #555555; }");
    vLayout->addWidget(m_launchButton);

    connect(btnRefresh, &QPushButton::clicked, this, &MainWindow::refreshVersionList);
    connect(m_versionList, &QListWidget::currentItemChanged, this, [this](QListWidgetItem *cur, QListWidgetItem *){
        if (cur) {
            m_selectedVersionId = cur->data(Qt::UserRole).toString();
            m_launchButton->setEnabled(m_isAuthenticated && !m_selectedVersionId.isEmpty());
            m_installFabricButton->setEnabled(true);
            m_installForgeButton->setEnabled(true);
        }
    });
    connect(m_installFabricButton, &QPushButton::clicked, this, [this](){
        if (!m_selectedVersionId.isEmpty()) m_versionManager->installFabric(m_selectedVersionId);
    });
    connect(m_installForgeButton, &QPushButton::clicked, this, [this](){
        if (!m_selectedVersionId.isEmpty()) m_versionManager->installForge(m_selectedVersionId);
    });
    connect(m_launchButton, &QPushButton::clicked, this, &MainWindow::onLaunchClicked);

    m_stack->addWidget(pageVersions);

    // Страница: Моды
    QWidget *pageMods = new QWidget;
    QVBoxLayout *mLayout = new QVBoxLayout(pageMods);
    mLayout->setContentsMargins(24, 24, 24, 24);
    mLayout->setSpacing(16);

    QLabel *modTitle = new QLabel("Моды (Modrinth / CurseForge)");
    modTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #ffffff;");
    mLayout->addWidget(modTitle);

    QHBoxLayout *modSearchLayout = new QHBoxLayout;
    m_modSearchEdit = new QLineEdit;
    m_modSearchEdit->setPlaceholderText("Поиск модов...");
    m_modSearchEdit->setMinimumHeight(38);
    m_modSearchEdit->setStyleSheet(
        "QLineEdit { background: #1e1e1e; border: 1px solid #444444; border-radius: 6px; color: #ffffff; padding: 6px 12px; }"
        "QLineEdit:focus { border: 1px solid #4a90e2; }");
    modSearchLayout->addWidget(m_modSearchEdit);

    QPushButton *btnSearchMods = new QPushButton("Искать");
    btnSearchMods->setMinimumHeight(38);
    btnSearchMods->setStyleSheet(
        "QPushButton { background-color: #4a90e2; color: white; border: none; border-radius: 6px; padding: 0 20px; font-weight: bold; }"
        "QPushButton:hover { background-color: #357abd; }");
    modSearchLayout->addWidget(btnSearchMods);
    mLayout->addLayout(modSearchLayout);

    m_modList = new QListWidget;
    m_modList->setStyleSheet(
        "QListWidget { background: #1e1e1e; border: 1px solid #333333; border-radius: 8px; color: #ffffff; font-size: 13px; }"
        "QListWidget::item { padding: 12px; border-bottom: 1px solid #2a2a2a; }"
        "QListWidget::item:hover { background: #2a2a2a; }");
    mLayout->addWidget(m_modList, 1);

    connect(btnSearchMods, &QPushButton::clicked, this, &MainWindow::onSearchMods);
    connect(m_modSearchEdit, &QLineEdit::returnPressed, this, &MainWindow::onSearchMods);

    m_stack->addWidget(pageMods);

    // Страница: Аккаунт
    QWidget *pageAccounts = new QWidget;
    QVBoxLayout *aLayout = new QVBoxLayout(pageAccounts);
    aLayout->setContentsMargins(24, 24, 24, 24);
    aLayout->setSpacing(16);

    QLabel *accTitle = new QLabel("Аккаунт Ely.by");
    accTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #ffffff;");
    aLayout->addWidget(accTitle);

    QLabel *accInfo = new QLabel("Авторизация через серверы ely.by. Кожа и сессия работают на всех серверах с authlib-injector.");
    accInfo->setWordWrap(true);
    accInfo->setStyleSheet("color: #aaaaaa; font-size: 13px;");
    aLayout->addWidget(accInfo);

    aLayout->addStretch();
    m_stack->addWidget(pageAccounts);

    // Страница: Настройки
    QWidget *pageSettings = new QWidget;
    QVBoxLayout *sLayout = new QVBoxLayout(pageSettings);
    sLayout->setContentsMargins(24, 24, 24, 24);
    sLayout->setSpacing(16);

    QLabel *setTitle = new QLabel("Настройки");
    setTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #ffffff;");
    sLayout->addWidget(setTitle);

    sLayout->addStretch();
    m_stack->addWidget(pageSettings);
}

void MainWindow::applyStyles()
{
    setStyleSheet(
        "QMainWindow { background-color: #1a1a1a; }"
        "QFrame#sidebar { background-color: #1e1e1e; border-right: 1px solid #333333; }"
        "QLabel { color: #ffffff; }"
    );
}

void MainWindow::refreshVersionList()
{
    m_versionList->clear();
    m_versionManager->fetchVanillaVersions();
}

void MainWindow::animatePageSwitch()
{
    QPropertyAnimation *anim = new QPropertyAnimation(m_stack, "windowOpacity", this);
    anim->setDuration(300);
    anim->setStartValue(0.8);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::showPageVersions() { m_stack->setCurrentIndex(0); animatePageSwitch(); }
void MainWindow::showPageMods()     { m_stack->setCurrentIndex(1); animatePageSwitch(); }
void MainWindow::showPageAccounts() { m_stack->setCurrentIndex(2); animatePageSwitch(); }
void MainWindow::showPageSettings() { m_stack->setCurrentIndex(3); animatePageSwitch(); }

// ---------- Слоты авторизации ----------
void MainWindow::onLoginClicked()
{
    // Создаём диалог входа
    AuthDialog *dlg = new AuthDialog(m_accountManager, this);
    dlg->exec();
}

void MainWindow::onLoginSucceeded(const ElyAccount &account)
{
    m_isAuthenticated = true;
    m_loginButton->hide();
    m_logoutButton->show();
    m_accountLabel->setText("Вход: " + account.username);
    m_accountLabel->setStyleSheet("color: #27ae60; font-size: 12px; font-weight: bold;");
    m_launchButton->setEnabled(!m_selectedVersionId.isEmpty());
}

void MainWindow::onLoginFailed(const QString &error)
{
    QMessageBox::warning(this, "Ошибка входа ely.by", error);
}

// ---------- Слоты версий ----------
void MainWindow::onVanillaVersionsFetched(const QList<MCVersion> &versions)
{
    m_versionList->clear();
    for (const auto &v : versions) {
        if (v.type != "release") continue; // показываем только релизы
        QListWidgetItem *item = new QListWidgetItem(v.id + "  (Vanilla)");
        item->setData(Qt::UserRole, v.id);
        m_versionList->addItem(item);
    }
}

void MainWindow::onVanillaVersionsFailed(const QString &error)
{
    QMessageBox::warning(this, "Ошибка", "Не удалось загрузить версии Minecraft:\n" + error);
}

void MainWindow::onFabricInstalled(const QString &mcVersion, const QString &installPath)
{
    Q_UNUSED(installPath)
    QMessageBox::information(this, "Fabric", "Fabric установлен для версии " + mcVersion);
}

void MainWindow::onFabricInstallFailed(const QString &error)
{
    QMessageBox::warning(this, "Ошибка Fabric", error);
}

void MainWindow::onForgeInstalled(const QString &mcVersion, const QString &installPath)
{
    Q_UNUSED(installPath)
    QMessageBox::information(this, "Forge", "Forge установлен для версии " + mcVersion);
}

void MainWindow::onForgeInstallFailed(const QString &error)
{
    QMessageBox::warning(this, "Ошибка Forge", error);
}

// ---------- Запуск ----------
void MainWindow::onLaunchClicked()
{
    if (!m_isAuthenticated) {
        QMessageBox::warning(this, "Вход", "Сначала войдите через Ely.by");
        return;
    }
    if (m_selectedVersionId.isEmpty()) {
        QMessageBox::warning(this, "Версия", "Выберите версию Minecraft");
        return;
    }

    QString javaPath = "java"; // путь к java можно вынести в настройки
    QString instanceDir = m_versionManager->instanceDir(m_selectedVersionId);
    int memoryMB = 2048;

    m_launchManager->launch(m_selectedVersionId, instanceDir, javaPath, memoryMB,
                            m_accountManager->currentAccount());
}

void MainWindow::onGameStarted(int pid)
{
    Q_UNUSED(pid)
    m_launchButton->setText("ИГРА ЗАПУЩЕНА");
    m_launchButton->setEnabled(false);
}

void MainWindow::onGameExited(int code)
{
    Q_UNUSED(code)
    m_launchButton->setText("ИГРАТЬ");
    m_launchButton->setEnabled(m_isAuthenticated && !m_selectedVersionId.isEmpty());
}

void MainWindow::onLaunchFailed(const QString &error)
{
    QMessageBox::critical(this, "Ошибка запуска", error);
    m_launchButton->setText("ИГРАТЬ");
    m_launchButton->setEnabled(m_isAuthenticated && !m_selectedVersionId.isEmpty());
}

// ---------- Моды ----------
void MainWindow::onSearchMods()
{
    QString query = m_modSearchEdit->text().trimmed();
    if (query.isEmpty()) query = "optifine";
    m_modManager->searchModrinth(query);
}

void MainWindow::onModrinthResults(const QList<ModInfo> &mods)
{
    m_modList->clear();
    for (const auto &mod : mods) {
        QListWidgetItem *item = new QListWidgetItem(mod.name + "  [" + mod.source + "]");
        item->setData(Qt::UserRole, mod.fileUrl);
        m_modList->addItem(item);
    }
}

void MainWindow::onModrinthFailed(const QString &error)
{
    Q_UNUSED(error)
    // Тихо — можно показать в статусбаре
}

void MainWindow::onLogoutClicked()
{
    m_isAuthenticated = false;
    m_loginButton->show();
    m_logoutButton->hide();
    m_accountLabel->setText("Не авторизован");
    m_accountLabel->setStyleSheet("color: #888888; font-size: 12px;");
    m_launchButton->setEnabled(false);
}
