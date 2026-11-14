// SPDX-License-Identifier: GPL-3.0-only
#include "MatronixPurpleTheme.h"

#include <QObject>

QString MatronixPurpleTheme::id()
{
    return QStringLiteral("matronix-purple");
}

QString MatronixPurpleTheme::name()
{
    return QObject::tr("Matronix Purple");
}

QString MatronixPurpleTheme::tooltip()
{
    return QObject::tr("Deep violet surfaces with a restrained purple gradient.");
}

bool MatronixPurpleTheme::hasStyleSheet()
{
    return true;
}

QString MatronixPurpleTheme::appStyleSheet()
{
    return QStringLiteral(
        "QWidget { color: #f2edfc; }"
        "QMainWindow, QDialog {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #100d18, stop:0.58 #171124, stop:1 #241637);"
        "}"
        "QWidget#centralWidget {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #110e1b, stop:0.62 #191329, stop:1 #241638);"
        "}"
        "QToolBar {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #191426, stop:1 #14111f);"
        "  border: 0; border-bottom: 1px solid #38284e; spacing: 5px; padding: 3px;"
        "}"
        "QPushButton, QToolButton, QComboBox, QSpinBox, QDoubleSpinBox, QLineEdit, QTextEdit, QPlainTextEdit {"
        "  background-color: #1a1525; color: #f2edfc; border: 1px solid #433453; border-radius: 4px; padding: 4px;"
        "}"
        "QPushButton:hover, QToolButton:hover, QComboBox:hover { background-color: #251b35; border-color: #7652a1; }"
        "QPushButton:pressed, QToolButton:pressed, QToolButton:checked { background-color: #38244f; border-color: #9a69d2; }"
        "QPushButton:disabled, QToolButton:disabled { color: #887d96; border-color: #30283a; }"
        "QMenu { background-color: #171321; border: 1px solid #49365e; padding: 3px; }"
        "QMenu::item:selected { background-color: #3a2852; color: #ffffff; }"
        "QHeaderView::section { background-color: #1d1729; color: #eee8f8; border: 1px solid #382b49; padding: 4px; }"
        "QToolTip { color: #f5efff; background-color: #1b1526; border: 1px solid #735493; padding: 4px; }"
        "QProgressBar { background-color: #15111e; border: 1px solid #49365e; text-align: center; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #7650bb, stop:1 #b37be2); }");
}

QPalette MatronixPurpleTheme::colorScheme()
{
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(QStringLiteral("#100d18")));
    palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#f2edfc")));
    palette.setColor(QPalette::Base, QColor(QStringLiteral("#14111e")));
    palette.setColor(QPalette::AlternateBase, QColor(QStringLiteral("#1b1628")));
    palette.setColor(QPalette::ToolTipBase, QColor(QStringLiteral("#211a30")));
    palette.setColor(QPalette::ToolTipText, QColor(QStringLiteral("#f5efff")));
    palette.setColor(QPalette::Text, QColor(QStringLiteral("#eee8f7")));
    palette.setColor(QPalette::Button, QColor(QStringLiteral("#211a30")));
    palette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#f2edfc")));
    palette.setColor(QPalette::BrightText, QColor(QStringLiteral("#ff718a")));
    palette.setColor(QPalette::Link, QColor(QStringLiteral("#c7a4ff")));
    palette.setColor(QPalette::Highlight, QColor(QStringLiteral("#7547ad")));
    palette.setColor(QPalette::HighlightedText, QColor(QStringLiteral("#ffffff")));
    palette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#a397b6")));
    palette.setColor(QPalette::Light, QColor(QStringLiteral("#49375e")));
    palette.setColor(QPalette::Midlight, QColor(QStringLiteral("#342842")));
    palette.setColor(QPalette::Mid, QColor(QStringLiteral("#2a2038")));
    palette.setColor(QPalette::Dark, QColor(QStringLiteral("#171222")));
    palette.setColor(QPalette::Shadow, QColor(QStringLiteral("#08060d")));
    return fadeInactive(palette, fadeAmount(), fadeColor());
}

double MatronixPurpleTheme::fadeAmount()
{
    return 0.45;
}

QColor MatronixPurpleTheme::fadeColor()
{
    return QColor(QStringLiteral("#130d1c"));
}
