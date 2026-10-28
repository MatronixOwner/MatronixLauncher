#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#include <QDialog>
#include "AccountManager.h"

class QLineEdit;
class QPushButton;
class QCheckBox;
class QLabel;

class AuthDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AuthDialog(AccountManager *accountManager, QWidget *parent = nullptr);
    ~AuthDialog();

private slots:
    void onLoginClicked();
    void onLoginSucceeded(const ElyAccount &account);
    void onLoginFailed(const QString &error);

private:
    AccountManager *m_accountManager;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QLineEdit *m_otpEdit;
    QCheckBox *m_otpCheck;
    QPushButton *m_loginButton;
    QLabel *m_statusLabel;
};

#endif // AUTHDIALOG_H
