// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDialog>
#include <QMap>
#include <QStringList>
#include <QVector>

class QLabel;
class QListWidget;
class QHBoxLayout;

class ChordEditorDialog final : public QDialog {
    Q_OBJECT
public:
    explicit ChordEditorDialog(const QString &content, QWidget *parent = nullptr);

    QString content() const;
    int currentWordCount() const;
    bool setChordForCurrentWord(int wordIndex, const QString &chord);

private:
    struct LinePair {
        int lyricLine = -1;
        int chordLine = -1;
        QString lyric;
        QVector<int> wordStarts;
        QStringList words;
        QMap<int, QString> chordsByWord;
    };

    void selectPair(int row);
    void rebuildWordButtons();
    QString chordLine(const LinePair &pair) const;

    QStringList m_lines;
    QString m_originalContent;
    QVector<LinePair> m_pairs;
    QListWidget *m_lineList = nullptr;
    QLabel *m_selectedLyric = nullptr;
    QWidget *m_wordContainer = nullptr;
    QHBoxLayout *m_wordLayout = nullptr;
    int m_currentPair = -1;
    bool m_modified = false;
};
