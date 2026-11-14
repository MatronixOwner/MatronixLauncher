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
        "QMainWindow, QDialog { background-color: #172b3a; }"
        "QWidget#centralWidget { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #1c3547,stop:0.55 #192e3d,stop:1 #122432); }"
        "QToolBar { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #223e50,stop:1 #1a3040); border:0; border-bottom:1px solid #426478; spacing:6px; padding:4px; }"
        "QPushButton, QToolButton, QComboBox, QSpinBox, QDoubleSpinBox, QLineEdit, QTextEdit, QPlainTextEdit {"
        " background-color:#243e4e; color:#dcecf4; border:1px solid #4a6d7e; border-radius:5px; padding:5px; }"
        "QPushButton:hover, QToolButton:hover, QComboBox:hover { background-color:#315468; border-color:#87bed4; }"
        "QPushButton:pressed, QToolButton:pressed, QToolButton:checked { background-color:#416c80; border-color:#9bcde0; }"
        "QPushButton:disabled, QToolButton:disabled { color:#829aa6; border-color:#405b68; }"
        "QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QComboBox:focus { border-color:#87bed4; }"
        "QGroupBox { border:1px solid #496d7e; border-radius:6px; margin-top:12px; padding:12px 8px 8px; }"
        "QGroupBox::title { subcontrol-origin:margin; left:12px; padding:0 5px; color:#9bcde0; }"
        "QTabWidget::pane { border:1px solid #496d7e; background-color:#1e3544; }"
        "QTabBar::tab { background:#243e4e; color:#a9c4d0; border:1px solid #496d7e; padding:6px 12px; }"
        "QTabBar::tab:selected { background:#315062; color:#eef8fc; border-bottom-color:#87bed4; }"
        "QAbstractItemView { selection-background-color:#416c80; selection-color:#f1fbff; alternate-background-color:#203847; outline:0; }"
        "QHeaderView::section { background-color:#294454; color:#d1e5ed; border:1px solid #496d7e; padding:4px; }"
        "QMenu { background-color:#1d3443; border:1px solid #527b8d; padding:3px; }"
        "QMenu::item:selected { background-color:#416c80; color:#f1fbff; }"
        "QToolTip { color:#e5f4fa; background-color:#213b4b; border:1px solid #78b2c9; padding:4px; }"
        "QProgressBar { background-color:#172b3a; color:#e5f4fa; border:1px solid #496d7e; border-radius:4px; text-align:center; }"
        "QProgressBar::chunk { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #5893ac,stop:1 #9bcde0); border-radius:3px; }");
}

QPalette MatronixFrostTheme::colorScheme()
{
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(QStringLiteral("#172b3a")));
    palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#dcecf4")));
    palette.setColor(QPalette::Base, QColor(QStringLiteral("#142735")));
    palette.setColor(QPalette::AlternateBase, QColor(QStringLiteral("#203847")));
    palette.setColor(QPalette::ToolTipBase, QColor(QStringLiteral("#213b4b")));
    palette.setColor(QPalette::ToolTipText, QColor(QStringLiteral("#e5f4fa")));
    palette.setColor(QPalette::Text, QColor(QStringLiteral("#dcecf4")));
    palette.setColor(QPalette::Button, QColor(QStringLiteral("#243e4e")));
    palette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#dcecf4")));
    palette.setColor(QPalette::BrightText, QColor(QStringLiteral("#d3f1fc")));
    palette.setColor(QPalette::Link, QColor(QStringLiteral("#99cee4")));
    palette.setColor(QPalette::Highlight, QColor(QStringLiteral("#416c80")));
    palette.setColor(QPalette::HighlightedText, QColor(QStringLiteral("#f1fbff")));
    palette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#93adb9")));
    palette.setColor(QPalette::Light, QColor(QStringLiteral("#527b8d")));
    palette.setColor(QPalette::Midlight, QColor(QStringLiteral("#395c6d")));
    palette.setColor(QPalette::Mid, QColor(QStringLiteral("#294454")));
    palette.setColor(QPalette::Dark, QColor(QStringLiteral("#142735")));
    palette.setColor(QPalette::Shadow, QColor(QStringLiteral("#091821")));
    return fadeInactive(palette, fadeAmount(), fadeColor());
}

QColor MatronixFrostTheme::fadeColor() { return QColor(QStringLiteral("#172b3a")); }
