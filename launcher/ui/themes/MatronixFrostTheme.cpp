// SPDX-License-Identifier: GPL-3.0-only
#include "MatronixFrostTheme.h"

#include <QObject>

QString MatronixFrostTheme::id() { return QStringLiteral("matronix-frost"); }
QString MatronixFrostTheme::name() { return QObject::tr("Matronix Frost"); }
QString MatronixFrostTheme::tooltip() { return QObject::tr("A muted icy blue theme with gently falling snow."); }

QString MatronixFrostTheme::appStyleSheet()
{
    return QStringLiteral(
        "QWidget { color: #edf8fc; }"
        "QMainWindow, QDialog { background-color: #375d70; }"
        "QWidget#centralWidget { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #4c788b,stop:0.55 #3d6577,stop:1 #304f61); }"
        "QToolBar { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #486f82,stop:1 #3c6274); border:0; border-bottom:1px solid #65889a; spacing:6px; padding:4px; }"
        "QPushButton, QToolButton, QComboBox, QSpinBox, QDoubleSpinBox, QLineEdit, QTextEdit, QPlainTextEdit {"
        " background-color:#41687a; color:#edf8fc; border:1px solid #80a4b3; border-radius:5px; padding:5px; }"
        "QPushButton:hover, QToolButton:hover, QComboBox:hover { background-color:#4a7184; border-color:#87bed4; }"
        "QPushButton:pressed, QToolButton:pressed, QToolButton:checked { background-color:#4d7486; border-color:#9bcde0; }"
        "QPushButton:disabled, QToolButton:disabled { color:#aebfc7; border-color:#68899a; }"
        "QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QComboBox:focus { border-color:#87bed4; }"
        "QGroupBox { border:1px solid #80a4b3; border-radius:6px; margin-top:12px; padding:12px 8px 8px; }"
        "QGroupBox::title { subcontrol-origin:margin; left:12px; padding:0 5px; color:#9bcde0; }"
        "QTabWidget::pane { border:1px solid #80a4b3; background-color:#3c6274; }"
        "QTabBar::tab { background:#41687a; color:#e1f0f5; border:1px solid #80a4b3; padding:6px 12px; }"
        "QTabBar::tab:selected { background:#4a7184; color:#eef8fc; border-bottom-color:#87bed4; }"
        "QAbstractItemView { selection-background-color:#4d7486; selection-color:#f1fbff; alternate-background-color:#416477; outline:0; }"
        "QHeaderView::section { background-color:#496d7f; color:#e1f0f5; border:1px solid #80a4b3; padding:4px; }"
        "QMenu { background-color:#375b6c; border:1px solid #8caebc; padding:3px; }"
        "QMenu::item:selected { background-color:#4d7486; color:#f1fbff; }"
        "QToolTip { color:#f0faff; background-color:#41687a; border:1px solid #9acbdd; padding:4px; }"
        "QProgressBar { background-color:#375d70; color:#f0faff; border:1px solid #80a4b3; border-radius:4px; text-align:center; }"
        "QProgressBar::chunk { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #5893ac,stop:1 #9bcde0); border-radius:3px; }");
}

QPalette MatronixFrostTheme::colorScheme()
{
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(QStringLiteral("#375d70")));
    palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#edf8fc")));
    palette.setColor(QPalette::Base, QColor(QStringLiteral("#304e5d")));
    palette.setColor(QPalette::AlternateBase, QColor(QStringLiteral("#416477")));
    palette.setColor(QPalette::ToolTipBase, QColor(QStringLiteral("#41687a")));
    palette.setColor(QPalette::ToolTipText, QColor(QStringLiteral("#f0faff")));
    palette.setColor(QPalette::Text, QColor(QStringLiteral("#edf8fc")));
    palette.setColor(QPalette::Button, QColor(QStringLiteral("#41687a")));
    palette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#edf8fc")));
    palette.setColor(QPalette::BrightText, QColor(QStringLiteral("#d3f1fc")));
    palette.setColor(QPalette::Link, QColor(QStringLiteral("#99cee4")));
    palette.setColor(QPalette::Highlight, QColor(QStringLiteral("#4d7486")));
    palette.setColor(QPalette::HighlightedText, QColor(QStringLiteral("#f1fbff")));
    palette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#bbd0d9")));
    palette.setColor(QPalette::Light, QColor(QStringLiteral("#8caebc")));
    palette.setColor(QPalette::Midlight, QColor(QStringLiteral("#60889a")));
    palette.setColor(QPalette::Mid, QColor(QStringLiteral("#496d7f")));
    palette.setColor(QPalette::Dark, QColor(QStringLiteral("#304e5d")));
    palette.setColor(QPalette::Shadow, QColor(QStringLiteral("#213c4a")));
    return fadeInactive(palette, fadeAmount(), fadeColor());
}

QColor MatronixFrostTheme::fadeColor() { return QColor(QStringLiteral("#375d70")); }
