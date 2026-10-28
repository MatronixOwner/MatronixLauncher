#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

#include "MainWindow.h"
#include "AccountManager.h"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    QApplication app(argc, argv);
    app.setApplicationName("MatronixLauncher");
    app.setOrganizationName("MatronixLauncher");
    app.setOrganizationDomain("matronixlauncher.org");
    app.setApplicationVersion(MATRONIX_VERSION);

    // Тёмная тема интерфейса (в стиле ElyPrismLauncher / Prism Launcher)
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(30, 30, 30));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(40, 40, 40));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(45, 45, 45));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(100, 150, 255));
    darkPalette.setColor(QPalette::Highlight, QColor(70, 130, 180));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    app.setPalette(darkPalette);

    QFont font("Segoe UI", 10);
    app.setFont(font);

    // Инициализация менеджера аккаунтов (ely.by)
    AccountManager accountManager;

    MainWindow mainWindow(&accountManager);
    mainWindow.setWindowTitle(QString("MatronixLauncher v%1").arg(MATRONIX_VERSION));
    mainWindow.resize(1100, 750);
    mainWindow.show();

    return app.exec();
}
