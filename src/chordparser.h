// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "song.h"

#include <QStringList>

struct ImportWarning {
    int line = -1;
    QString message;
};

class ChordParser {
public:
    static Song importText(const QString &rawText, const QString &sourceNameHint = {});
    static QString cleanImportText(const QString &rawText);
    static QString normalizePdfColumns(const QString &extractedText);
    static bool isChordLine(const QString &line);
    static QString transposeContent(const QString &content, int semitones, bool germanNotation = true);
    static QString transposeChord(const QString &chord, int semitones, bool germanNotation = true);
    static QStringList sectionNames();
    static QList<ImportWarning> analyzeImport(const Song &song);
};
