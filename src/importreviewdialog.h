// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "song.h"

#include <QDialog>

class QLineEdit;
class QPlainTextEdit;
class QSpinBox;

class ImportReviewDialog final : public QDialog {
    Q_OBJECT
public:
    ImportReviewDialog(const QString &sourceText, const Song &detectedSong,
                       const QString &sourceDescription, QWidget *parent = nullptr,
                       const QString &pdfFileName = {});

    Song reviewedSong() const;

private:
    QLineEdit *m_titleEdit = nullptr;
    QLineEdit *m_artistEdit = nullptr;
    QLineEdit *m_keyEdit = nullptr;
    QSpinBox *m_bpmEdit = nullptr;
    QSpinBox *m_capoEdit = nullptr;
    QPlainTextEdit *m_resultEdit = nullptr;
};
