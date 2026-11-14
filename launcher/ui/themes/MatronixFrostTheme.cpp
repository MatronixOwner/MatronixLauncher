// SPDX-License-Identifier: GPL-3.0-only
#include "MatronixFrostTheme.h"

#include <QObject>

QString MatronixFrostTheme::id() { return QStringLiteral("matronix-frost"); }
QString MatronixFrostTheme::name() { return QObject::tr("Matronix Frost"); }
QString MatronixFrostTheme::tooltip() { return QObject::tr("An icy blue theme with gently falling snow."); }

QString MatronixFrostTheme::appStyleSheet()
{
    return QStringLiteral(
        "QWidget { color: #18384e; }"
        "QMainWindow, QDialog { background-color: #c6e5f3; }"
        "QWidget#centralWidget { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #dff5fc,stop:0.55 #bedfed,stop:1 #9ccbe2); }"
        "QToolBar { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #e3f6fc,stop:1 #b9dfef); border:0; border-bottom:1px solid #86b8d0; spacing:6px; padding:4px; }"
        "QPushButton, QToolButton, QComboBox, QSpinBox, QDoubleSpinBox, QLineEdit, QTextEdit, QPlainTextEdit {"
        " background-color:#e9f8fd; color:#173c52; border:1px solid #8dbdd2; border-radius:5px; padding:5px; }"
        "QPushButton:hover, QToolButton:hover, QComboBox:hover { background-color:#d0effb; border-color:#358db8; }"
        "QPushButton:pressed, QToolButton:pressed, QToolButton:checked { background-color:#a4d7ee; border-color:#267da8; }"
        "QPushButton:disabled, QToolButton:disabled { color:#7696a5; border-color:#b5d3df; }"
        "QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QComboBox:focus { border-color:#2789b7; }"
        "QGroupBox { border:1px solid #91c2d7; border-radius:6px; margin-top:12px; padding:12px 8px 8px; }"
        "QGroupBox::title { subcontrol-origin:margin; left:12px; padding:0 5px; color:#246b8b; }"
        "QTabWidget::pane { border:1px solid #91c2d7; background-color:#d7eff9; }"
        "QTabBar::tab { background:#c3e3f1; color:#285b72; border:1px solid #91c2d7; padding:6px 12px; }"
        "QTabBar::tab:selected { background:#e9f8fd; color:#173c52; border-bottom-color:#2789b7; }"
        "QAbstractItemView { selection-background-color:#88cce9; selection-color:#103a50; alternate-background-color:#d7eff9; outline:0; }"
        "QHeaderView::section { background-color:#c2e4f2; color:#285b72; border:1px solid #a3ccdd; padding:4px; }"
        "QMenu { background-color:#e4f5fc; border:1px solid #83b9d1; padding:3px; }"
        "QMenu::item:selected { background-color:#a6d9ee; color:#173c52; }"
        "QToolTip { color:#173c52; background-color:#e4f5fc; border:1px solid #58a7c9; padding:4px; }"
        "QProgressBar { background-color:#dbf1f9; color:#173c52; border:1px solid #91c2d7; border-radius:4px; text-align:center; }"
        "QProgressBar::chunk { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #48a9d0,stop:1 #9ce6f6); border-radius:3px; }");
}

QPalette MatronixFrostTheme::colorScheme()
{
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(QStringLiteral("#c6e5f3")));
    palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#18384e")));
    palette.setColor(QPalette::Base, QColor(QStringLiteral("#e7f7fc")));
    palette.setColor(QPalette::AlternateBase, QColor(QStringLiteral("#d7eff9")));
    palette.setColor(QPalette::ToolTipBase, QColor(QStringLiteral("#e4f5fc")));
    palette.setColor(QPalette::ToolTipText, QColor(QStringLiteral("#173c52")));
    palette.setColor(QPalette::Text, QColor(QStringLiteral("#18384e")));
    palette.setColor(QPalette::Button, QColor(QStringLiteral("#d3eef9")));
    palette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#18384e")));
    palette.setColor(QPalette::BrightText, QColor(QStringLiteral("#185d7c")));
    palette.setColor(QPalette::Link, QColor(QStringLiteral("#176a96")));
    palette.setColor(QPalette::Highlight, QColor(QStringLiteral("#88cce9")));
    palette.setColor(QPalette::HighlightedText, QColor(QStringLiteral("#103a50")));
    palette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#668a9d")));
    palette.setColor(QPalette::Light, QColor(QStringLiteral("#f1fbff")));
    palette.setColor(QPalette::Midlight, QColor(QStringLiteral("#c6e5f3")));
    palette.setColor(QPalette::Mid, QColor(QStringLiteral("#9bc9dc")));
    palette.setColor(QPalette::Dark, QColor(QStringLiteral("#78a9bf")));
    palette.setColor(QPalette::Shadow, QColor(QStringLiteral("#4e7e96")));
    return fadeInactive(palette, fadeAmount(), fadeColor());
}

QColor MatronixFrostTheme::fadeColor() { return QColor(QStringLiteral("#c6e5f3")); }
