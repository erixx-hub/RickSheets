// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPalette>
#include <QString>

class QApplication;

QString rickSheetsStyleSheet();
QPalette rickSheetsLightPalette();
QPalette rickSheetsDarkPalette();
QString rickSheetsEffectiveTheme(const QApplication &application,
                                 const QString &preference);
void applyRickSheetsTheme(QApplication &application,
                          const QString &preference);
