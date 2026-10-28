// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2024 Tayou <git@tayou.org>
 *  Copyright (C) 2024 TheKodeToad <TheKodeToad@proton.me>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */
#include "DarkTheme.h"

#include <QObject>

QString DarkTheme::id()
{
    return "dark";
}

QString DarkTheme::name()
{
    return QObject::tr("Matronix Black");
}

QPalette DarkTheme::colorScheme()
{
    // Keep the launcher genuinely black while using subtle grayscale surfaces
    // to distinguish controls and selection states.
    QPalette blackPalette;
    blackPalette.setColor(QPalette::Window, QColor("#000000"));
    blackPalette.setColor(QPalette::WindowText, QColor("#f2f2f2"));
    blackPalette.setColor(QPalette::Base, QColor("#000000"));
    blackPalette.setColor(QPalette::AlternateBase, QColor("#101010"));
    blackPalette.setColor(QPalette::ToolTipBase, QColor("#111111"));
    blackPalette.setColor(QPalette::ToolTipText, QColor("#f5f5f5"));
    blackPalette.setColor(QPalette::Text, QColor("#f2f2f2"));
    blackPalette.setColor(QPalette::Button, QColor("#111111"));
    blackPalette.setColor(QPalette::ButtonText, QColor("#f2f2f2"));
    blackPalette.setColor(QPalette::BrightText, QColor("#ff6666"));
    blackPalette.setColor(QPalette::Link, QColor("#d0d0d0"));
    blackPalette.setColor(QPalette::Highlight, QColor("#444444"));
    blackPalette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    blackPalette.setColor(QPalette::PlaceholderText, QColor("#858585"));
    blackPalette.setColor(QPalette::Light, QColor("#3a3a3a"));
    blackPalette.setColor(QPalette::Midlight, QColor("#252525"));
    blackPalette.setColor(QPalette::Mid, QColor("#1b1b1b"));
    blackPalette.setColor(QPalette::Dark, QColor("#0b0b0b"));
    blackPalette.setColor(QPalette::Shadow, QColor("#000000"));
    return fadeInactive(blackPalette, fadeAmount(), fadeColor());
}

double DarkTheme::fadeAmount()
{
    return 0.5;
}

QColor DarkTheme::fadeColor()
{
    return QColor("#000000");
}

bool DarkTheme::hasStyleSheet()
{
    return true;
}

QString DarkTheme::appStyleSheet()
{
    return QStringLiteral(
        "QWidget { background-color: #000000; color: #f2f2f2; }"
        "QPushButton, QToolButton, QComboBox, QSpinBox, QDoubleSpinBox, QLineEdit, QTextEdit, QPlainTextEdit {"
        "  background-color: #101010; border: 1px solid #333333; border-radius: 3px; padding: 3px; }"
        "QPushButton:hover, QToolButton:hover, QComboBox:hover { background-color: #1b1b1b; border-color: #555555; }"
        "QPushButton:pressed, QToolButton:pressed, QToolButton:checked { background-color: #292929; }"
        "QPushButton:disabled, QToolButton:disabled { color: #777777; border-color: #222222; }"
        "QMenu { background-color: #080808; border: 1px solid #333333; }"
        "QMenu::item:selected { background-color: #303030; color: #ffffff; }"
        "QHeaderView::section { background-color: #111111; color: #eeeeee; border: 1px solid #292929; padding: 4px; }"
        "QToolTip { color: #f5f5f5; background-color: #111111; border: 1px solid #555555; padding: 4px; }"
        "QProgressBar { background-color: #080808; border: 1px solid #333333; text-align: center; }"
        "QProgressBar::chunk { background-color: #777777; }");
}

QString DarkTheme::tooltip()
{
    return "";
}
