// SPDX-License-Identifier: GPL-3.0-only
#include "MatronixAccentTheme.h"

#include <QObject>

namespace {
struct Colors {
    const char* id;
    const char* name;
    const char* description;
    const char* background;
    const char* surface;
    const char* raised;
    const char* foreground;
    const char* muted;
    const char* accent;
    const char* accentBright;
    const char* accentDark;
    const char* border;
};

const Colors& colors(MatronixAccentTheme::Variant variant)
{
    static const Colors aurora{ "aurora", "Aurora", "Midnight blue with flowing teal and violet aurora light.", "#08121e", "#0f1c2b", "#16283b", "#e7f3fb", "#97acbf", "#42d6c5", "#ae95ff", "#153c42", "#2c4558" };
    static const Colors crimson{ "crimson", "Crimson", "Polished graphite with restrained, vivid crimson accents.", "#111013", "#1b191d", "#262227", "#f5edf0", "#afa3aa", "#e84862", "#ff8294", "#3d202a", "#41323a" };
    static const Colors frost{ "frost", "Frost", "Quiet ice-blue surfaces with crisp, frosted highlights.", "#081321", "#101e2d", "#172a3f", "#eaf6ff", "#a4bad0", "#78cfff", "#c5f0ff", "#1d3b51", "#31516b" };
    static const Colors christmas{ "christmas", "Christmas", "Evergreen, cranberry, and warm champagne for the winter season.", "#0b1511", "#122019", "#1b2c22", "#f2f5ec", "#a9baaa", "#c84f59", "#e7c77e", "#38262a", "#354c3d" };
    switch (variant) {
        case MatronixAccentTheme::Variant::Aurora: return aurora;
        case MatronixAccentTheme::Variant::Crimson: return crimson;
        case MatronixAccentTheme::Variant::Frost: return frost;
        case MatronixAccentTheme::Variant::Christmas: return christmas;
    }
    return aurora;
}

QString centralGlow(MatronixAccentTheme::Variant variant)
{
    switch (variant) {
        case MatronixAccentTheme::Variant::Aurora:
            return QStringLiteral("qradialgradient(cx:0.16, cy:0.04, radius:0.96, fx:0.16, fy:0.04, stop:0 rgba(48, 205, 186, 38), stop:0.48 rgba(22, 48, 67, 24), stop:1 #08121e)");
        case MatronixAccentTheme::Variant::Crimson:
            return QStringLiteral("qradialgradient(cx:0.88, cy:0.08, radius:1.0, fx:0.88, fy:0.08, stop:0 rgba(192, 42, 68, 34), stop:0.5 rgba(48, 29, 37, 18), stop:1 #111013)");
        case MatronixAccentTheme::Variant::Frost:
            return QStringLiteral("qradialgradient(cx:0.18, cy:0.06, radius:0.95, fx:0.18, fy:0.06, stop:0 rgba(89, 178, 235, 28), stop:0.52 rgba(26, 53, 78, 16), stop:1 #081321)");
        case MatronixAccentTheme::Variant::Christmas:
            return QStringLiteral("qradialgradient(cx:0.88, cy:0.02, radius:0.92, fx:0.88, fy:0.02, stop:0 rgba(181, 78, 76, 28), stop:0.5 rgba(31, 58, 41, 18), stop:1 #0b1511)");
    }
    return QStringLiteral("#08121e");
}
}

QString MatronixAccentTheme::id() { return QString::fromLatin1(colors(m_variant).id); }
QString MatronixAccentTheme::name() { return QObject::tr(colors(m_variant).name); }
QString MatronixAccentTheme::tooltip() { return QObject::tr(colors(m_variant).description); }

QString MatronixAccentTheme::appStyleSheet()
{
    const auto& c = colors(m_variant);
    QString style = QStringLiteral(
        "QWidget { color: %1; }"
        "QMainWindow, QDialog { background-color: %2; }"
        "QWidget#centralWidget { background: %12; }"
        "QToolBar { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 %3,stop:1 %2); border:0; border-bottom:1px solid %11; spacing:7px; padding:4px 6px; }"
        "QPushButton, QToolButton, QComboBox, QSpinBox, QDoubleSpinBox, QLineEdit, QTextEdit, QPlainTextEdit {"
        " background-color:%4; color:%1; border:1px solid %11; border-radius:5px; padding:5px 8px; }"
        "QPushButton:hover, QToolButton:hover, QComboBox:hover { background-color:%5; border-color:%6; }"
        "QPushButton:pressed, QToolButton:pressed, QToolButton:checked { background-color:%7; border-color:%8; }"
        "QPushButton:default { border:1px solid %6; }"
        "QPushButton:disabled, QToolButton:disabled { color:%9; border-color:%10; }"
        "QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus { border-color:%6; }"
        "QComboBox::drop-down { border:0; border-left:1px solid %11; width:22px; }"
        "QGroupBox { background-color:%3; border:1px solid %11; border-radius:6px; margin-top:12px; padding:12px 8px 8px; }"
        "QGroupBox::title { subcontrol-origin:margin; left:12px; padding:0 5px; color:%6; }"
        "QTabWidget::pane { border:1px solid %11; background-color:%3; }"
        "QTabBar::tab { background-color:%4; color:%9; border:1px solid %11; padding:7px 12px; margin-right:3px; }"
        "QTabBar::tab:selected { background-color:%3; color:%1; border-bottom-color:%6; }"
        "QAbstractItemView { selection-background-color:%7; selection-color:%1; alternate-background-color:%3; outline:0; }"
        "QHeaderView::section { background-color:%3; color:%9; border:0; border-bottom:1px solid %11; padding:6px 8px; }"
        "QMenu { background-color:%3; border:1px solid %11; padding:4px; }"
        "QMenu::item { padding:6px 24px 6px 10px; }"
        "QMenu::item:selected { background-color:%7; color:%1; }"
        "QScrollBar:vertical { background-color:%2; width:10px; margin:2px; }"
        "QScrollBar:horizontal { background-color:%2; height:10px; margin:2px; }"
        "QScrollBar::handle:vertical, QScrollBar::handle:horizontal { background-color:%11; border-radius:4px; min-height:26px; min-width:26px; }"
        "QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover { background-color:%6; }"
        "QScrollBar::add-line, QScrollBar::sub-line { width:0; height:0; }"
        "QCheckBox::indicator, QRadioButton::indicator { width:15px; height:15px; border:1px solid %11; background-color:%4; }"
        "QCheckBox::indicator:checked, QRadioButton::indicator:checked { background-color:%6; border-color:%8; }"
        "QToolTip { color:%1; background-color:%3; border:1px solid %6; padding:5px; }"
        "QProgressBar { background-color:%2; color:%1; border:1px solid %11; border-radius:4px; text-align:center; }"
        "QProgressBar::chunk { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %6,stop:1 %8); border-radius:3px; }")
        .arg(QString::fromLatin1(c.foreground))
        .arg(QString::fromLatin1(c.background))
        .arg(QString::fromLatin1(c.surface))
        .arg(QString::fromLatin1(c.raised))
        .arg(QString::fromLatin1(c.border))
        .arg(QString::fromLatin1(c.accent))
        .arg(QString::fromLatin1(c.accentDark))
        .arg(QString::fromLatin1(c.accentBright))
        .arg(QString::fromLatin1(c.muted))
        .arg(QString::fromLatin1(c.border))
        .arg(QString::fromLatin1(c.border))
        .arg(centralGlow(m_variant));

    if (m_variant == Variant::Frost) {
        style += QStringLiteral("QGroupBox { background-color: rgba(17, 34, 52, 218); }");
        style += QStringLiteral("QTabWidget::pane { background-color: rgba(16, 30, 45, 220); }");
    }
    return style;
}

QPalette MatronixAccentTheme::colorScheme()
{
    const auto& c = colors(m_variant);
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(QString::fromLatin1(c.background)));
    palette.setColor(QPalette::WindowText, QColor(QString::fromLatin1(c.foreground)));
    palette.setColor(QPalette::Base, QColor(QString::fromLatin1(c.surface)));
    palette.setColor(QPalette::AlternateBase, QColor(QString::fromLatin1(c.raised)));
    palette.setColor(QPalette::ToolTipBase, QColor(QString::fromLatin1(c.surface)));
    palette.setColor(QPalette::ToolTipText, QColor(QString::fromLatin1(c.foreground)));
    palette.setColor(QPalette::Text, QColor(QString::fromLatin1(c.foreground)));
    palette.setColor(QPalette::Button, QColor(QString::fromLatin1(c.raised)));
    palette.setColor(QPalette::ButtonText, QColor(QString::fromLatin1(c.foreground)));
    palette.setColor(QPalette::BrightText, QColor(QString::fromLatin1(c.accentBright)));
    palette.setColor(QPalette::Link, QColor(QString::fromLatin1(c.accent)));
    palette.setColor(QPalette::Highlight, QColor(QString::fromLatin1(c.accentDark)));
    palette.setColor(QPalette::HighlightedText, QColor(QString::fromLatin1(c.foreground)));
    palette.setColor(QPalette::PlaceholderText, QColor(QString::fromLatin1(c.muted)));
    palette.setColor(QPalette::Light, QColor(QString::fromLatin1(c.border)));
    palette.setColor(QPalette::Midlight, QColor(QString::fromLatin1(c.raised)));
    palette.setColor(QPalette::Mid, QColor(QString::fromLatin1(c.surface)));
    palette.setColor(QPalette::Dark, QColor(QString::fromLatin1(c.background)));
    palette.setColor(QPalette::Shadow, QColor(QStringLiteral("#05080d")));
    return fadeInactive(palette, fadeAmount(), fadeColor());
}

QColor MatronixAccentTheme::fadeColor()
{
    return QColor(QString::fromLatin1(colors(m_variant).background));
}
