// SPDX-License-Identifier: GPL-3.0-only
#include "MatronixFrostTheme.h"

#include <QObject>

QString MatronixFrostTheme::id() { return QStringLiteral("matronix-frost"); }
QString MatronixFrostTheme::name() { return QObject::tr("Matronix Frost"); }
QString MatronixFrostTheme::tooltip() { return QObject::tr("A muted icy blue theme with gently falling snow."); }

QString MatronixFrostTheme::appStyleSheet()
{
    return QStringLiteral(
        "QWidget { color: #dcecf4; }"
        "QMainWindow, QDialog { background-color: #284657; }"
        "QWidget#centralWidget { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #365a6c,stop:0.55 #2c4d5e,stop:1 #213e4e); }"
        "QToolBar { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #395e70,stop:1 #2d4d5e); border:0; border-bottom:1px solid #65889a; spacing:6px; padding:4px; }"
        "QPushButton, QToolButton, QComboBox, QSpinBox, QDoubleSpinBox, QLineEdit, QTextEdit, QPlainTextEdit {"
        " background-color:#315567; color:#dcecf4; border:1px solid #668c9d; border-radius:5px; padding:5px; }"
        "QPushButton:hover, QToolButton:hover, QComboBox:hover { background-color:#406c80; border-color:#87bed4; }"
        "QPushButton:pressed, QToolButton:pressed, QToolButton:checked { background-color:#4b788d; border-color:#9bcde0; }"
        "QPushButton:disabled, QToolButton:disabled { color:#9db2bc; border-color:#527384; }"
        "QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QComboBox:focus { border-color:#87bed4; }"
        "QGroupBox { border:1px solid #668c9d; border-radius:6px; margin-top:12px; padding:12px 8px 8px; }"
        "QGroupBox::title { subcontrol-origin:margin; left:12px; padding:0 5px; color:#9bcde0; }"
        "QTabWidget::pane { border:1px solid #668c9d; background-color:#2c4e5e; }"
        "QTabBar::tab { background:#315567; color:#d1e5ed; border:1px solid #668c9d; padding:6px 12px; }"
        "QTabBar::tab:selected { background:#3c6578; color:#eef8fc; border-bottom-color:#87bed4; }"
        "QAbstractItemView { selection-background-color:#4b788d; selection-color:#f1fbff; alternate-background-color:#315365; outline:0; }"
        "QHeaderView::section { background-color:#395d6e; color:#d1e5ed; border:1px solid #668c9d; padding:4px; }"
        "QMenu { background-color:#294a5b; border:1px solid #7398a8; padding:3px; }"
        "QMenu::item:selected { background-color:#4b788d; color:#f1fbff; }"
        "QToolTip { color:#e5f4fa; background-color:#315567; border:1px solid #78b2c9; padding:4px; }"
        "QProgressBar { background-color:#284657; color:#e5f4fa; border:1px solid #668c9d; border-radius:4px; text-align:center; }"
        "QProgressBar::chunk { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #5893ac,stop:1 #9bcde0); border-radius:3px; }");
}

QPalette MatronixFrostTheme::colorScheme()
{
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(QStringLiteral("#284657")));
    palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#dcecf4")));
    palette.setColor(QPalette::Base, QColor(QStringLiteral("#233e4c")));
    palette.setColor(QPalette::AlternateBase, QColor(QStringLiteral("#315365")));
    palette.setColor(QPalette::ToolTipBase, QColor(QStringLiteral("#315567")));
    palette.setColor(QPalette::ToolTipText, QColor(QStringLiteral("#e5f4fa")));
    palette.setColor(QPalette::Text, QColor(QStringLiteral("#dcecf4")));
    palette.setColor(QPalette::Button, QColor(QStringLiteral("#315567")));
    palette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#dcecf4")));
    palette.setColor(QPalette::BrightText, QColor(QStringLiteral("#d3f1fc")));
    palette.setColor(QPalette::Link, QColor(QStringLiteral("#99cee4")));
    palette.setColor(QPalette::Highlight, QColor(QStringLiteral("#4b788d")));
    palette.setColor(QPalette::HighlightedText, QColor(QStringLiteral("#f1fbff")));
    palette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#93adb9")));
    palette.setColor(QPalette::Light, QColor(QStringLiteral("#7398a8")));
    palette.setColor(QPalette::Midlight, QColor(QStringLiteral("#4e7485")));
    palette.setColor(QPalette::Mid, QColor(QStringLiteral("#395d6e")));
    palette.setColor(QPalette::Dark, QColor(QStringLiteral("#233e4c")));
    palette.setColor(QPalette::Shadow, QColor(QStringLiteral("#142c39")));
    return fadeInactive(palette, fadeAmount(), fadeColor());
}

QColor MatronixFrostTheme::fadeColor() { return QColor(QStringLiteral("#284657")); }
