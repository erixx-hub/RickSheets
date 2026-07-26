// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chordeditordialog.h"

#include "appstyle.h"
#include "chordparser.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QVBoxLayout>

ChordEditorDialog::ChordEditorDialog(const QString &content, QWidget *parent)
    : QDialog(parent),
      m_lines(content.split('\n')),
      m_originalContent(content)
{
    qApp->setStyleSheet(rickSheetsStyleSheet());
    setWindowTitle(tr("Akkorde visuell bearbeiten"));
    resize(1100, 680);
    setMinimumSize(820, 520);

    static const QRegularExpression sectionLine(
        R"(^\s*\[[^\]]+\]\s*$)");
    static const QRegularExpression wordExpression(R"(\S+)");
    static const QRegularExpression chordToken(R"([^|\s]+)");

    for (int lineIndex = 0; lineIndex < m_lines.size(); ++lineIndex) {
        const QString lyric = m_lines.at(lineIndex);
        if (lyric.trimmed().isEmpty() ||
            sectionLine.match(lyric).hasMatch() ||
            ChordParser::isChordLine(lyric) ||
            lyric.contains(QRegularExpression(R"(\[[A-H](?:#|b)?)",
                                              QRegularExpression::CaseInsensitiveOption)))
            continue;

        LinePair pair;
        pair.lyricLine = lineIndex;
        pair.lyric = lyric;
        auto words = wordExpression.globalMatch(lyric);
        while (words.hasNext()) {
            const auto match = words.next();
            pair.wordStarts << match.capturedStart();
            pair.words << match.captured();
        }
        if (pair.words.isEmpty())
            continue;

        if (lineIndex > 0 &&
            ChordParser::isChordLine(m_lines.at(lineIndex - 1)) &&
            !m_lines.at(lineIndex - 1).contains('|')) {
            pair.chordLine = lineIndex - 1;
            auto chords = chordToken.globalMatch(m_lines.at(pair.chordLine));
            while (chords.hasNext()) {
                const auto match = chords.next();
                int closestWord = 0;
                int closestDistance =
                    qAbs(pair.wordStarts.at(0) - match.capturedStart());
                for (int word = 1; word < pair.wordStarts.size(); ++word) {
                    const int distance =
                        qAbs(pair.wordStarts.at(word) - match.capturedStart());
                    if (distance < closestDistance) {
                        closestWord = word;
                        closestDistance = distance;
                    }
                }
                const QString existing = pair.chordsByWord.value(closestWord);
                pair.chordsByWord[closestWord] =
                    existing.isEmpty() ? match.captured()
                                       : existing + " " + match.captured();
            }
        }
        m_pairs << pair;
    }

    auto *layout = new QVBoxLayout(this);
    auto *intro = new QLabel(
        tr("Wähle links eine Textzeile. Rechts ordnest du jedem Wort mit "
           "einem Klick einen Akkord zu. Leere Eingabe entfernt den Akkord."));
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto *columns = new QHBoxLayout;
    auto *left = new QVBoxLayout;
    auto *lineHeading = new QLabel(tr("TEXTZEILEN"));
    lineHeading->setObjectName("dialogHeading");
    QFont heading = lineHeading->font();
    heading.setBold(true);
    lineHeading->setFont(heading);
    m_lineList = new QListWidget;
    m_lineList->setObjectName("chordEditorLines");
    for (const LinePair &pair : m_pairs) {
        const QString text =
            QFontMetrics(m_lineList->font())
                .elidedText(pair.lyric.trimmed(), Qt::ElideRight, 320);
        m_lineList->addItem(text);
    }
    left->addWidget(lineHeading);
    left->addWidget(m_lineList, 1);
    columns->addLayout(left, 1);

    auto *right = new QVBoxLayout;
    auto *placementHeading = new QLabel(tr("AKKORDE ÜBER WÖRTERN"));
    placementHeading->setObjectName("dialogHeading");
    placementHeading->setFont(heading);
    m_selectedLyric = new QLabel;
    m_selectedLyric->setObjectName("lyricCard");
    m_selectedLyric->setWordWrap(true);
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    m_wordContainer = new QWidget;
    m_wordLayout = new QHBoxLayout(m_wordContainer);
    m_wordLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    scroll->setWidget(m_wordContainer);
    right->addWidget(placementHeading);
    right->addWidget(m_selectedLyric);
    right->addWidget(scroll, 1);
    columns->addLayout(right, 2);
    layout->addLayout(columns, 1);

    auto *note = new QLabel(
        tr("Inline-Akkorde wie [Am]Text und reine Taktzeilen bleiben "
           "unverändert und können weiterhin im Rohtext bearbeitet werden."));
    note->setWordWrap(true);
    layout->addWidget(note);

    auto *buttons =
        new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Akkorde übernehmen"));
    buttons->button(QDialogButtonBox::Ok)->setProperty("primary", true);
    buttons->button(QDialogButtonBox::Cancel)->setProperty("quiet", true);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    connect(m_lineList, &QListWidget::currentRowChanged,
            this, &ChordEditorDialog::selectPair);
    if (!m_pairs.isEmpty())
        m_lineList->setCurrentRow(0);
    else {
        m_selectedLyric->setText(
            tr("Keine geeigneten Textzeilen gefunden."));
        buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

void ChordEditorDialog::selectPair(int row)
{
    m_currentPair = row >= 0 && row < m_pairs.size() ? row : -1;
    m_selectedLyric->setText(
        m_currentPair >= 0 ? m_pairs.at(m_currentPair).lyric : QString());
    rebuildWordButtons();
}

void ChordEditorDialog::rebuildWordButtons()
{
    while (QLayoutItem *item = m_wordLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    if (m_currentPair < 0)
        return;

    const LinePair &pair = m_pairs.at(m_currentPair);
    for (int wordIndex = 0; wordIndex < pair.words.size(); ++wordIndex) {
        const QString chord = pair.chordsByWord.value(wordIndex);
        auto *button = new QPushButton(
            (chord.isEmpty() ? tr("+ Akkord") : chord) +
            "\n" + pair.words.at(wordIndex));
        button->setObjectName(QString("chordWord_%1").arg(wordIndex));
        button->setProperty("wordIndex", wordIndex);
        button->setMinimumHeight(58);
        if (!chord.isEmpty())
            button->setProperty("chordSet", true);
        connect(button, &QPushButton::clicked, this,
                [this, wordIndex, button] {
                    const QString current =
                        m_pairs.at(m_currentPair).chordsByWord.value(wordIndex);
                    bool ok = false;
                    const QString chord = QInputDialog::getText(
                        this, tr("Akkord setzen"),
                        tr("Akkord über „%1“ (leer = entfernen):")
                            .arg(m_pairs.at(m_currentPair).words.at(wordIndex)),
                        QLineEdit::Normal, current, &ok);
                    if (!ok)
                        return;
                    if (!setChordForCurrentWord(wordIndex, chord)) {
                        QMessageBox::warning(
                            this, tr("Ungültiger Akkord"),
                            tr("Bitte gib genau einen Akkord ein, zum Beispiel "
                               "Am, F#7 oder E/G#."));
                        return;
                    }
                    Q_UNUSED(button);
                    rebuildWordButtons();
                });
        m_wordLayout->addWidget(button);
    }
    m_wordLayout->addStretch();
}

int ChordEditorDialog::currentWordCount() const
{
    return m_currentPair >= 0
               ? m_pairs.at(m_currentPair).words.size()
               : 0;
}

bool ChordEditorDialog::setChordForCurrentWord(int wordIndex,
                                               const QString &chord)
{
    if (m_currentPair < 0 ||
        wordIndex < 0 ||
        wordIndex >= m_pairs.at(m_currentPair).words.size())
        return false;
    const QString value = chord.trimmed();
    if (!value.isEmpty() &&
        (value.contains(QRegularExpression(R"(\s)")) ||
         !ChordParser::isChordLine(value)))
        return false;
    if (value.isEmpty())
        m_pairs[m_currentPair].chordsByWord.remove(wordIndex);
    else
        m_pairs[m_currentPair].chordsByWord[wordIndex] = value;
    m_modified = true;
    return true;
}

QString ChordEditorDialog::chordLine(const LinePair &pair) const
{
    QString line(pair.lyric.size(), ' ');
    int previousEnd = -1;
    for (auto it = pair.chordsByWord.cbegin();
         it != pair.chordsByWord.cend(); ++it) {
        int position = pair.wordStarts.value(it.key());
        if (position <= previousEnd)
            position = previousEnd + 1;
        if (line.size() < position + it.value().size())
            line.resize(position + it.value().size(), ' ');
        line.replace(position, it.value().size(), it.value());
        previousEnd = position + it.value().size() - 1;
    }
    while (line.endsWith(' '))
        line.chop(1);
    return line;
}

QString ChordEditorDialog::content() const
{
    if (!m_modified)
        return m_originalContent;
    QMap<int, QString> replacements;
    QMap<int, QString> insertBefore;
    for (const LinePair &pair : m_pairs) {
        const QString chords = chordLine(pair);
        if (pair.chordLine >= 0)
            replacements[pair.chordLine] = chords;
        else if (!chords.isEmpty())
            insertBefore[pair.lyricLine] = chords;
    }

    QStringList result;
    for (int line = 0; line < m_lines.size(); ++line) {
        if (insertBefore.contains(line))
            result << insertBefore.value(line);
        if (replacements.contains(line)) {
            if (!replacements.value(line).isEmpty())
                result << replacements.value(line);
        } else {
            result << m_lines.at(line);
        }
    }
    return result.join('\n').trimmed();
}
