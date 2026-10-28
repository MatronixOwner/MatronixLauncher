#include "AuthDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QMessageBox>

AuthDialog::AuthDialog(AccountManager *accountManager, QWidget *parent)
    : QDialog(parent)
    , m_accountManager(accountManager)
{
    setWindowTitle("Войти через Ely.by");
    setFixedSize(400, 300);
    setModal(true);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(12);
    layout->setContentsMargins(24, 24, 24, 24);

    auto *title = new QLabel("Матроникс Лаунчер — вход в Ely.by");
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: #ffffff;");
    layout->addWidget(title);

    auto *sub = new QLabel("Введите логин и пароль вашего аккаунта на ely.by");
    sub->setStyleSheet("color: #aaaaaa; font-size: 12px;");
    layout->addWidget(sub);

    m_usernameEdit = new QLineEdit;
    m_usernameEdit->setPlaceholderText("Логин или e-mail (ely.by)");
    m_usernameEdit->setMinimumHeight(36);
    layout->addWidget(m_usernameEdit);

    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setPlaceholderText("Пароль");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(36);
    layout->addWidget(m_passwordEdit);

    m_otpCheck = new QCheckBox("У меня включена двухфакторная аутентификация (2FA)");
    layout->addWidget(m_otpCheck);

    m_otpEdit = new QLineEdit;
    m_otpEdit->setPlaceholderText("Код из приложения аутентификации");
    m_otpEdit->setMinimumHeight(36);
    m_otpEdit->setEnabled(false);
    layout->addWidget(m_otpEdit);

    connect(m_otpCheck, &QCheckBox::toggled, m_otpEdit, &QLineEdit::setEnabled);

    m_loginButton = new QPushButton("Войти");
    m_loginButton->setMinimumHeight(40);
    m_loginButton->setStyleSheet(
        "QPushButton { background-color: #4a90e2; color: white; border: none; border-radius: 6px; font-weight: bold; }"
        "QPushButton:hover { background-color: #357abd; }"
        "QPushButton:pressed { background-color: #2c5f9e; }"
        "QPushButton:disabled { background-color: #555555; }");
    layout->addWidget(m_loginButton);

    m_statusLabel = new QLabel("");
    m_statusLabel->setStyleSheet("color: #ff6b6b; font-size: 12px;");
    m_statusLabel->setWordWrap(true);
    layout->addWidget(m_statusLabel);

    layout->addStretch();

    connect(m_loginButton, &QPushButton::clicked, this, &AuthDialog::onLoginClicked);
    connect(m_accountManager, &AccountManager::loginSucceeded, this, &AuthDialog::onLoginSucceeded);
    connect(m_accountManager, &AccountManager::loginFailed, this, &AuthDialog::onLoginFailed);
}

AuthDialog::~AuthDialog() = default;

void AuthDialog::onLoginClicked()
{
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    QString otp = m_otpCheck->isChecked() ? m_otpEdit->text().trimmed() : QString();

    if (username.isEmpty() || password.isEmpty()) {
        m_statusLabel->setText("Заполните логин и пароль");
        return;
    }

    m_loginButton->setEnabled(false);
    m_statusLabel->setText("Вход в ely.by...");
    m_accountManager->login(username, password, otp);
}

void AuthDialog::onLoginSucceeded(const ElyAccount &account)
{
    Q_UNUSED(account)
    accept();
}

void AuthDialog::onLoginFailed(const QString &error)
{
    m_loginButton->setEnabled(true);
    m_statusLabel->setText(error);
}
