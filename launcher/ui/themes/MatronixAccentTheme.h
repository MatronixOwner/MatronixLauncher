// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "FusionTheme.h"

class MatronixAccentTheme : public FusionTheme {
   public:
    enum class Variant { Aurora, Crimson, Frost, Christmas };
    explicit MatronixAccentTheme(Variant variant) : m_variant(variant) {}

    QString id() override;
    QString name() override;
    QString tooltip() override;
    bool hasStyleSheet() override { return true; }
    QString appStyleSheet() override;
    QPalette colorScheme() override;
    double fadeAmount() override { return 0.45; }
    QColor fadeColor() override;

   private:
    Variant m_variant;
};
