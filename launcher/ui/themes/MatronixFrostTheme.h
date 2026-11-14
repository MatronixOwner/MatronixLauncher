// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "FusionTheme.h"

class MatronixFrostTheme : public FusionTheme {
   public:
    QString id() override;
    QString name() override;
    QString tooltip() override;
    bool hasStyleSheet() override { return true; }
    QString appStyleSheet() override;
    QPalette colorScheme() override;
    double fadeAmount() override { return 0.25; }
    QColor fadeColor() override;
};
