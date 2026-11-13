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
    const char* border;
};

const Colors& colors(MatronixAccentTheme::Variant variant)
{
    static const Colors aurora{ "aurora", "Aurora", "Deep navy with soft teal and violet aurora accents.", "#091321", "#101d2c", "#17283a", "#e8f5fb", "#9db1c2", "#42d6c5", "#9c83ff", "#294355" };
    static const Colors crimson{ "crimson", "Crimson", "Graphite surfaces with vivid crimson highlights.", "#151416", "#201d20", "#2b2529", "#f5edef", "#b8a8ad", "#e33c55", "#ff6478", "#493039" };
    static const Colors frost{ "frost", "Frost", "Cool midnight blue with icy highlights and translucent panels.", "#0b1421", "#122033", "#1a2d43", "#eaf5ff", "#a9bfd4", "#7acbff", "#b0e5ff", "#304b63" };
    switch (variant) {
        case MatronixAccentTheme::Variant::Aurora: return aurora;
        case MatronixAccentTheme::Variant::Crimson: return crimson;
        case MatronixAccentTheme::Variant::Frost: return frost;
    }
    return aurora;
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
        "QMainWindow, QDialog { background: %2; }"
        "QWidget#centralWidget { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %2,stop:1 %3); }"
        "QToolBar { background: %3; border: 0; border-bottom: 1px solid %11; spacing: 5px; padding: 3px; }"
        "QPushButton, QToolButton, QComboBox, QSpinBox, QDoubleSpinBox, QLineEdit, QTextEdit, QPlainTextEdit {"
        " background-color: %4; color: %1; border: 1px solid %11; border-radius: 4px; padding: 4px; }"
        "QPushButton:hover, QToolButton:hover, QComboBox:hover { background-color: %5; border-color: %6; }"
        "QPushButton:pressed, QToolButton:pressed, QToolButton:checked { background-color: %6; border-color: %7; }"
        "QMenu { background-color: %3; border: 1px solid %11; padding: 3px; }"
        "QMenu::item:selected { background-color: %6; color: %1; }"
        "QHeaderView::section { background-color: %4; color: %1; border: 1px solid %11; padding: 4px; }"
        "QToolTip { color: %1; background-color: %4; border: 1px solid %6; padding: 4px; }"
        "QProgressBar { background-color: %2; border: 1px solid %11; text-align: center; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %6,stop:1 %7); }")
        .arg(QString::fromLatin1(c.foreground))
        .arg(QString::fromLatin1(c.background))
        .arg(QString::fromLatin1(c.surface))
        .arg(QString::fromLatin1(c.raised))
        .arg(QString::fromLatin1(c.border))
        .arg(QString::fromLatin1(c.accent))
        .arg(QString::fromLatin1(c.accentBright))
        .arg(QString::fromLatin1(c.muted))
        .arg(QString::fromLatin1(c.accent))
        .arg(QString::fromLatin1(c.accentBright))
        .arg(QString::fromLatin1(c.border));
    if (m_variant == Variant::Frost) {
        style += QStringLiteral("QGroupBox { background-color: rgba(18, 32, 51, 210); border: 1px solid #304b63; }");
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
    palette.setColor(QPalette::ToolTipBase, QColor(QString::fromLatin1(c.raised)));
    palette.setColor(QPalette::ToolTipText, QColor(QString::fromLatin1(c.foreground)));
    palette.setColor(QPalette::Text, QColor(QString::fromLatin1(c.foreground)));
    palette.setColor(QPalette::Button, QColor(QString::fromLatin1(c.raised)));
    palette.setColor(QPalette::ButtonText, QColor(QString::fromLatin1(c.foreground)));
    palette.setColor(QPalette::BrightText, QColor(QString::fromLatin1(c.accentBright)));
    palette.setColor(QPalette::Link, QColor(QString::fromLatin1(c.accent)));
    palette.setColor(QPalette::Highlight, QColor(QString::fromLatin1(c.accent)));
    palette.setColor(QPalette::HighlightedText, QColor(QString::fromLatin1(c.foreground)));
    palette.setColor(QPalette::PlaceholderText, QColor(QString::fromLatin1(c.muted)));
    palette.setColor(QPalette::Light, QColor(QString::fromLatin1(c.border)));
    palette.setColor(QPalette::Midlight, QColor(QString::fromLatin1(c.raised)));
    palette.setColor(QPalette::Mid, QColor(QString::fromLatin1(c.surface)));
    palette.setColor(QPalette::Dark, QColor(QString::fromLatin1(c.background)));
    palette.setColor(QPalette::Shadow, QColor(QStringLiteral("#080b10")));
    return fadeInactive(palette, fadeAmount(), fadeColor());
}

QColor MatronixAccentTheme::fadeColor()
{
    return QColor(QString::fromLatin1(colors(m_variant).background));
}
