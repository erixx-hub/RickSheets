// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

struct Song {
    int formatVersion = 1;
    QString title;
    QString artist;
    QString key;
    int bpm = 0;
    int capo = 0;
    int layoutScale = 100;
    QString content;

    QByteArray toJson() const;
    static Song fromJson(const QByteArray &data, QString *error = nullptr);
};
