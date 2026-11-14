// SPDX-License-Identifier: GPL-3.0-only
#include "FrostTitleBar.h"
#include <QWidget>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <dwmapi.h>
#endif

void updateFrostTitleBar(QWidget* window, bool enabled)
{
#ifdef Q_OS_WIN
    // Windows 11 supports explicit caption colors; older versions ignore them.
    const COLORREF caption = enabled ? RGB(31, 58, 75) : 0xffffffffu;
    const COLORREF text = enabled ? RGB(220, 236, 244) : 0xffffffffu;
    const HWND handle = reinterpret_cast<HWND>(window->winId());
    DwmSetWindowAttribute(handle, static_cast<DWMWINDOWATTRIBUTE>(35), &caption, sizeof(caption));
    DwmSetWindowAttribute(handle, static_cast<DWMWINDOWATTRIBUTE>(36), &text, sizeof(text));
#else
    Q_UNUSED(window);
    Q_UNUSED(enabled);
#endif
}
