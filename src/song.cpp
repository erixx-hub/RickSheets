// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#include "song.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

QByteArray Song::toJson() const
{
    QJsonObject object{
        {"formatVersion", formatVersion},
        {"title", title},
        {"artist", artist},
        {"key", key},
        {"bpm", bpm},
        {"capo", capo},
        {"content", content},
    };
    return QJsonDocument(object).toJson(QJsonDocument::Indented);
}

Song Song::fromJson(const QByteArray &data, QString *error)
{
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error)
            *error = parseError.errorString();
        return {};
    }

    const auto object = document.object();
    Song song;
    song.formatVersion = object.value("formatVersion").toInt(1);
    song.title = object.value("title").toString();
    song.artist = object.value("artist").toString();
    song.key = object.value("key").toString();
    song.bpm = object.value("bpm").toInt();
    song.capo = object.value("capo").toInt();
    song.content = object.value("content").toString();
    return song;
}
