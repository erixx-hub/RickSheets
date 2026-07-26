// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

class QCoreApplication;

QString rickSheetsLanguagePreference();
QString rickSheetsEffectiveLanguage(const QString &preference = {});
bool applyRickSheetsLanguage(QCoreApplication &application,
                             const QString &preference = {});
void storeRickSheetsLanguagePreference(const QString &preference);
