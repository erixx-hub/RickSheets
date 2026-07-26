// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "song.h"

#include <QTextDocument>

class SongRenderer {
public:
    static QString toHtml(const Song &song);
    static void configureDocument(QTextDocument &document, const Song &song);
    static int pageCount(const QTextDocument &document);
};
