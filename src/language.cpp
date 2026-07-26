// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#include "language.h"

#include <QCoreApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

#include <memory>

namespace {
std::unique_ptr<QTranslator> activeTranslator;
std::unique_ptr<QTranslator> activeQtTranslator;
}

QString rickSheetsLanguagePreference()
{
    QSettings settings;
    settings.sync();
    return settings.value("appearance/language", "system").toString();
}

QString rickSheetsEffectiveLanguage(const QString &preference)
{
    const QString selected =
        preference.isEmpty() ? rickSheetsLanguagePreference() : preference;
    if (selected == "de" || selected == "en")
        return selected;
    return QLocale::system().language() == QLocale::German ? "de" : "en";
}

bool applyRickSheetsLanguage(QCoreApplication &application,
                             const QString &preference)
{
    if (activeTranslator) {
        application.removeTranslator(activeTranslator.get());
        activeTranslator.reset();
    }
    if (activeQtTranslator) {
        application.removeTranslator(activeQtTranslator.get());
        activeQtTranslator.reset();
    }

    if (rickSheetsEffectiveLanguage(preference) == "de") {
        auto qtTranslator = std::make_unique<QTranslator>();
        const QString qtCatalog = QDir(
            QLibraryInfo::path(QLibraryInfo::TranslationsPath))
                                      .filePath("qtbase_de.qm");
        if (qtTranslator->load(qtCatalog)) {
            application.installTranslator(qtTranslator.get());
            activeQtTranslator = std::move(qtTranslator);
        }
        return true;
    }

    auto translator = std::make_unique<QTranslator>();
    if (!translator->load(":/i18n/ricksheets_en.qm")) {
        const QString adjacentCatalog =
            QDir(application.applicationDirPath()).filePath("ricksheets_en.qm");
        if (!translator->load(adjacentCatalog))
            return false;
    }
    application.installTranslator(translator.get());
    activeTranslator = std::move(translator);
    return true;
}

void storeRickSheetsLanguagePreference(const QString &preference)
{
    if (preference != "system" && preference != "de" && preference != "en")
        return;
    QSettings settings;
    settings.setValue("appearance/language", preference);
    settings.sync();
}
