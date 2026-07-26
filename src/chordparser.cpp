// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chordparser.h"

#include <QRegularExpression>

namespace {
const QRegularExpression chordToken(
    R"(^([A-H](?:#|b)?)(m|maj|min|dim|aug|sus|add)?(\d{0,2})?([+\-]?\d*)?(?:/([A-H](?:#|b)?))?(\.|\(.*\))?$)",
    QRegularExpression::CaseInsensitiveOption);

int pitchForRoot(QString root)
{
    root = root.trimmed();
    const bool flat = root.endsWith('b');
    const bool sharp = root.endsWith('#');
    const QChar letter = root.at(0).toUpper();
    int pitch = 0;
    switch (letter.unicode()) {
    case 'C': pitch = 0; break;
    case 'D': pitch = 2; break;
    case 'E': pitch = 4; break;
    case 'F': pitch = 5; break;
    case 'G': pitch = 7; break;
    case 'A': pitch = 9; break;
    case 'H': pitch = 11; break;
    case 'B': pitch = 10; break; // German B = Bb
    }
    return (pitch + (sharp ? 1 : 0) - (flat ? 1 : 0) + 12) % 12;
}

QString normalizedRoot(int pitch, bool german)
{
    static const QStringList germanNames{"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "B", "H"};
    static const QStringList internationalNames{"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"};
    return (german ? germanNames : internationalNames).at((pitch % 12 + 12) % 12);
}

bool isSectionDescription(const QString &value)
{
    static const QRegularExpression keyword(
        R"(\b(intro|verse|strophe|chorus|refrain|pre[- ]?chorus|bridge|solo|instrumental|outro|interlude|ending|nur\s+(?:bass|gitarre|vocals)|alle)\b)",
        QRegularExpression::CaseInsensitiveOption);
    return keyword.match(value).hasMatch();
}

bool parseSectionLine(const QString &line, QString *label = nullptr, QString *remainder = nullptr)
{
    const QString trimmed = line.trimmed();
    static const QRegularExpression closedWrapper(
        R"(^[\[(]\s*([^\])]{2,48}?)\s*[\])]\s*:?\s*(.*)$)");
    const auto wrappedMatch = closedWrapper.match(trimmed);
    if (wrappedMatch.hasMatch() && isSectionDescription(wrappedMatch.captured(1))) {
        if (label)
            *label = wrappedMatch.captured(1).trimmed();
        if (remainder)
            *remainder = wrappedMatch.captured(2).trimmed();
        return true;
    }

    static const QRegularExpression danglingOpen(R"(^[\[(]\s*(.{2,48}?)\s*$)");
    const auto danglingMatch = danglingOpen.match(trimmed);
    if (danglingMatch.hasMatch() && isSectionDescription(danglingMatch.captured(1))) {
        if (label)
            *label = danglingMatch.captured(1).trimmed();
        if (remainder)
            remainder->clear();
        return true;
    }

    if (trimmed.startsWith('[') || trimmed.startsWith('('))
        return false;

    QString plain = trimmed;
    plain.remove(QRegularExpression(R"([\])]\s*:?\s*$)"));
    plain.remove(QRegularExpression(R"(:\s*$)"));
    if (isSectionDescription(plain) && plain.size() <= 52 &&
        !plain.contains(QRegularExpression(R"(\s{2,})"))) {
        if (label)
            *label = plain.trimmed();
        if (remainder)
            remainder->clear();
        return true;
    }
    return false;
}

bool looksLikeSection(const QString &line)
{
    QString remainder;
    return parseSectionLine(line, nullptr, &remainder) && remainder.isEmpty();
}
}

QStringList ChordParser::sectionNames()
{
    return {"Intro", "Strophe", "Pre-Chorus", "Chorus", "Bridge", "Solo", "Instrumental", "Outro"};
}

QString ChordParser::cleanImportText(const QString &rawText)
{
    QString text = rawText;
    text.replace("\r\n", "\n");
    text.replace('\r', '\n');
    text.replace('\f', '\n');
    text.replace('\t', "    ");
    text.replace(QRegularExpression(R"(\[/?(?:ch|tab)\])",
                                    QRegularExpression::CaseInsensitiveOption),
                 QString());

    const QStringList lines = text.split('\n');
    QStringList cleaned;
    static const QRegularExpression noiseLine(
        R"(^\s*(?:(?:Difficulty|Tuning|Author|Last\s+edit|Views?|Rating|Published)\s*:.*|(?:Tabbed|Chords?|Transcribed|Arranged)\s+by\b.*|View\s+official\s+tab.*|Check\s+out\s+the\s+tab.*|We\s+have\s+an\s+official.*|Listen\s+backing\s+track.*|Download\s+PDF.*|Print\s+this\s+page.*|Add\s+to\s+favorites.*|Transpose.*|Autoscroll.*|Simplify.*|Correct\s+tab.*|Improve\s+this\s+tab.*|There\s+is\s+no\s+strumming\s+pattern.*|Strumming\s+pattern.*|Ultimate[- ]Guitar(?:\.com)?.*|UG\s+professional\s+guitarists.*)\s*$)",
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression urlLine(
        R"(^\s*(?:https?://|www\.)\S+\s*$)",
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression pageCounter(
        R"(^\s*(?:Page|Seite)\s+\d+\s*(?:/|of|von)\s*\d+\s*$)",
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression fretRow(
        R"(^\s*(?:[xX0-9-]{1,4}\s+){5}[xX0-9-]{1,4}\s*$)");
    static const QRegularExpression stringNames(
        R"(^\s*[Ee]\s+[Aa]\s+[Dd]\s+[Gg]\s+[BbHh]\s+[Ee]\s*$)");

    for (int i = 0; i < lines.size(); ++i) {
        const QString line = lines.at(i);
        const QString trimmed = line.trimmed();
        if (noiseLine.match(trimmed).hasMatch() ||
            urlLine.match(trimmed).hasMatch() ||
            pageCounter.match(trimmed).hasMatch() ||
            stringNames.match(trimmed).hasMatch()) {
            continue;
        }

        if (fretRow.match(trimmed).hasMatch()) {
            if (!cleaned.isEmpty() && isChordLine(cleaned.last()) &&
                !looksLikeSection(cleaned.last()))
                cleaned.removeLast();
            continue;
        }

        // A print view may put the six fret values on separate lines directly
        // below a chord name. Remove only a complete six-value group.
        if (isChordLine(trimmed) && i + 6 < lines.size()) {
            bool sixSingleFretValues = true;
            for (int offset = 1; offset <= 6; ++offset) {
                if (!QRegularExpression(R"(^[xX0-9-]{1,4}$)")
                         .match(lines.at(i + offset).trimmed())
                         .hasMatch()) {
                    sixSingleFretValues = false;
                    break;
                }
            }
            if (sixSingleFretValues) {
                i += 6;
                continue;
            }
        }

        cleaned << line;
    }

    QStringList compact;
    bool previousBlank = false;
    for (const QString &line : cleaned) {
        const bool blank = line.trimmed().isEmpty();
        if (!blank || !previousBlank)
            compact << (blank ? QString() : line);
        previousBlank = blank;
    }
    while (!compact.isEmpty() && compact.first().trimmed().isEmpty())
        compact.removeFirst();
    while (!compact.isEmpty() && compact.last().trimmed().isEmpty())
        compact.removeLast();
    return compact.join('\n');
}

QString ChordParser::normalizePdfColumns(const QString &extractedText)
{
    QString normalized = extractedText;
    normalized.replace("\r\n", "\n");
    normalized.replace('\r', '\n');
    const QStringList pages = normalized.split('\f');
    QStringList orderedPages;

    for (const QString &page : pages) {
        const QStringList lines = page.split('\n');
        QMap<int, int> startHistogram;
        int pageWidth = 0;
        for (const QString &line : lines) {
            pageWidth = qMax(pageWidth, line.size());
            const int firstNonSpace = line.indexOf(QRegularExpression(R"(\S)"));
            if (firstNonSpace >= 35 && firstNonSpace <= 110)
                ++startHistogram[firstNonSpace];

            auto gaps = QRegularExpression(R"(\s{6,}(?=\S))").globalMatch(line);
            while (gaps.hasNext()) {
                const auto gap = gaps.next();
                const int rightStart = gap.capturedEnd();
                if (rightStart >= 35 && rightStart <= 110)
                    ++startHistogram[rightStart];
            }
        }

        int columnStart = -1;
        int bestCluster = 0;
        const int searchStart = qMax(35, qRound(pageWidth * 0.44));
        const int searchEnd = qMin(110, qRound(pageWidth * 0.58));
        const auto findBestCluster = [&](int first, int last) {
            for (auto it = startHistogram.cbegin(); it != startHistogram.cend();
                 ++it) {
                if (it.key() < first || it.key() > last)
                    continue;
                int cluster = 0;
                for (int position = it.key() - 2; position <= it.key() + 2;
                     ++position)
                    cluster += startHistogram.value(position);
                if (cluster > bestCluster) {
                    bestCluster = cluster;
                    columnStart = it.key();
                }
            }
        };
        findBestCluster(searchStart, searchEnd);
        const bool useWideSearch = bestCluster < 3;
        if (useWideSearch)
            findBestCluster(35, 110);
        // Chord rows often begin a few characters inside the right column and
        // can outvote its true left edge. Prefer the earliest strong cluster
        // near the page centre so that this indentation remains intact.
        const int strongCluster = qMax(3, qCeil(bestCluster * 0.55));
        for (auto it = startHistogram.cbegin(); it != startHistogram.cend(); ++it) {
            if (it.key() < (useWideSearch ? 35 : searchStart) ||
                it.key() > (useWideSearch ? 110 : searchEnd))
                continue;
            int cluster = 0;
            for (int position = it.key() - 2; position <= it.key() + 2; ++position)
                cluster += startHistogram.value(position);
            if (cluster >= strongCluster) {
                columnStart = it.key();
                break;
            }
        }

        if (columnStart < 0 || bestCluster < 3) {
            orderedPages << page.trimmed();
            continue;
        }

        QStringList left;
        QStringList right;
        int actualSplits = 0;
        for (const QString &line : lines) {
            if (line.size() <= columnStart - 2) {
                left << line;
                right << QString();
                continue;
            }

            const int firstNonSpace = line.indexOf(QRegularExpression(R"(\S)"));
            bool split = firstNonSpace >= columnStart - 2;
            if (!split) {
                const int gapStart = line.lastIndexOf(
                    QRegularExpression(R"(\S)"), columnStart - 1) + 1;
                const int nextText = line.indexOf(
                    QRegularExpression(R"(\S)"), qMax(columnStart - 2, gapStart));
                split = gapStart >= 0 && nextText >= columnStart - 2 &&
                        nextText - gapStart >= 4;
            }

            if (split) {
                QString leftLine = line.left(columnStart);
                leftLine.remove(QRegularExpression(R"(\s+$)"));
                QString rightLine = line.mid(columnStart);
                rightLine.remove(QRegularExpression(R"(\s+$)"));
                left << leftLine;
                right << rightLine;
                ++actualSplits;
            } else {
                left << line;
                right << QString();
            }
        }

        if (actualSplits < 3) {
            orderedPages << page.trimmed();
            continue;
        }

        auto trimBlankEdges = [](QStringList values) {
            while (!values.isEmpty() && values.first().trimmed().isEmpty())
                values.removeFirst();
            while (!values.isEmpty() && values.last().trimmed().isEmpty())
                values.removeLast();
            QStringList compact;
            bool previousBlank = false;
            for (const QString &value : values) {
                const bool blank = value.trimmed().isEmpty();
                if (!blank || !previousBlank)
                    compact << value;
                previousBlank = blank;
            }
            return compact;
        };
        left = trimBlankEdges(left);
        right = trimBlankEdges(right);
        orderedPages << (left.join('\n') + "\n\n" + right.join('\n')).trimmed();
    }
    return orderedPages.join("\n\n").trimmed();
}

bool ChordParser::isChordLine(const QString &line)
{
    QString simplified = line;
    simplified.replace('|', ' ');
    simplified.replace(':', ' ');
    simplified.replace(QRegularExpression(R"(\s+)"), " ");
    const auto tokens = simplified.trimmed().split(' ', Qt::SkipEmptyParts);
    if (tokens.isEmpty())
        return false;

    int chordCount = 0;
    for (QString token : tokens) {
        token.remove(',');
        if (token.compare("N.C.", Qt::CaseInsensitive) == 0 ||
            token.compare("N.C", Qt::CaseInsensitive) == 0 ||
            token == "/" || token == "x2" || token == "2x") {
            ++chordCount;
        } else if (chordToken.match(token).hasMatch()) {
            ++chordCount;
        }
    }
    return chordCount > 0 && chordCount * 4 >= tokens.size() * 3;
}

Song ChordParser::importText(const QString &rawText, const QString &sourceNameHint)
{
    Song song;
    const QString text = cleanImportText(rawText);
    QStringList lines = text.split('\n');

    while (!lines.isEmpty() && lines.first().trimmed().isEmpty())
        lines.removeFirst();
    while (!lines.isEmpty() && lines.last().trimmed().isEmpty())
        lines.removeLast();

    // PDF text extraction may emit the right column before the title. Inspect
    // several short column fragments instead of trusting only the first line.
    struct Candidate {
        QString text;
        int lineIndex;
    };
    QList<Candidate> candidates;
    const int headerScanLines = qMin(18, lines.size());
    for (int i = 0; i < headerScanLines; ++i) {
        const auto fragments = lines.at(i).split(QRegularExpression(R"(\s{7,})"), Qt::SkipEmptyParts);
        for (const QString &fragment : fragments) {
            const QString value = fragment.trimmed();
            if (value.isEmpty() || value.size() > 75 || isChordLine(value) ||
                looksLikeSection(value) || value.startsWith('[') || value.startsWith('(') ||
                value.contains(QRegularExpression(R"(^[-_]{5,})")) ||
                value.contains(QRegularExpression(R"(\b(BPM|Capo)\b)", QRegularExpression::CaseInsensitiveOption)))
                continue;
            candidates << Candidate{value, i};
        }
    }

    static const QRegularExpression titleArtist(
        R"(^(.{2,}?)\s+[–—-]\s+(.{2,})$)");
    for (int candidateIndex = 0; candidateIndex < candidates.size(); ++candidateIndex) {
        const Candidate &candidate = candidates.at(candidateIndex);
        const auto match = titleArtist.match(candidate.text);
        if (match.hasMatch()) {
            song.title = match.captured(1).trimmed();
            song.artist = match.captured(2).trimmed();
            // A long title can wrap immediately before "last words - artist".
            if (song.title.size() < 12 && candidateIndex > 0 &&
                candidates.at(candidateIndex - 1).lineIndex + 1 >= candidate.lineIndex &&
                candidates.at(candidateIndex - 1).text.size() < 50 &&
                !QRegularExpression(R"([.!?]$)").match(candidates.at(candidateIndex - 1).text).hasMatch()) {
                song.title = candidates.at(candidateIndex - 1).text + " " + song.title;
            }
            break;
        }
    }

    // Handle headers split over two lines, e.g. "Friends in Low Places -" /
    // "Garth Brooks".
    if (song.title.isEmpty()) {
        for (int i = 0; i + 1 < candidates.size(); ++i) {
            if (!candidates.at(i).text.endsWith('-'))
                continue;
            int artistIndex = -1;
            for (int j = i + 1; j < candidates.size() &&
                                candidates.at(j).lineIndex <= candidates.at(i).lineIndex + 3; ++j) {
                const QString possibleArtist = candidates.at(j).text;
                const auto words = possibleArtist.split(' ', Qt::SkipEmptyParts);
                if (possibleArtist.size() <= 32 && words.size() <= 5 &&
                    !possibleArtist.contains(QRegularExpression(R"([,.!?'])"))) {
                    artistIndex = j;
                    break;
                }
            }
            if (artistIndex >= 0) {
                song.title = candidates.at(i).text.left(candidates.at(i).text.size() - 1).trimmed();
                song.artist = candidates.at(artistIndex).text.trimmed();
                break;
            }
        }
    }

    if (song.title.isEmpty() && !candidates.isEmpty()) {
        int bestIndex = 0;
        int bestScore = -100;
        for (int i = 0; i < candidates.size(); ++i) {
            const QString value = candidates.at(i).text;
            int score = 0;
            if (value == value.toUpper() && value.contains(QRegularExpression("[A-Z]")))
                score += 8;
            if (value.size() <= 42)
                score += 3;
            if (value.split(' ', Qt::SkipEmptyParts).size() <= 7)
                score += 2;
            if (value.contains(QRegularExpression(R"(\b(I|you|he|she|we|they|my|your|the)\b)",
                                                  QRegularExpression::CaseInsensitiveOption)))
                score -= 4;
            score -= candidates.at(i).lineIndex;
            if (score > bestScore) {
                bestScore = score;
                bestIndex = i;
            }
        }
        song.title = candidates.at(bestIndex).text;
        if (bestIndex + 1 < candidates.size() &&
            candidates.at(bestIndex + 1).lineIndex <= candidates.at(bestIndex).lineIndex + 2)
            song.artist = candidates.at(bestIndex + 1).text;
    }

    if (!sourceNameHint.isEmpty()) {
        QString hint = sourceNameHint;
        hint.remove(QRegularExpression(R"(\.pdf$)", QRegularExpression::CaseInsensitiveOption));
        hint.remove(QRegularExpression(R"(\d+$)"));
        const auto hintMatch = titleArtist.match(hint);
        const bool suspiciousTitle =
            song.title.size() <= 4 || song.title.contains("---") ||
            song.title.contains(',') || song.title.split(' ', Qt::SkipEmptyParts).size() > 7 ||
            (song.title.size() < 12 && hint.size() > song.title.size()) ||
            song.title.contains(
                QRegularExpression(R"(\b(while|when|well|and|but|I|you|he|she|we|they)\b)",
                                   QRegularExpression::CaseInsensitiveOption));
        if (hintMatch.hasMatch()) {
            song.title = hintMatch.captured(1).trimmed();
            song.artist = hintMatch.captured(2).trimmed();
        } else if (suspiciousTitle || song.title.isEmpty()) {
            song.title = hint.trimmed();
        }
    }

    if (song.artist.split(' ', Qt::SkipEmptyParts).size() > 5 ||
        song.artist.contains(
            QRegularExpression(R"(^(and|while|when|I|you|he|she|we|they|my|your)\b)",
                               QRegularExpression::CaseInsensitiveOption))) {
        song.artist.clear();
    }

    // For ordinary pasted text, remove a clean header from the editable body.
    if (!lines.isEmpty()) {
        const QString first = lines.first().trimmed();
        const auto match = titleArtist.match(first);
        if (match.hasMatch() && match.captured(1).trimmed() == song.title) {
            lines.removeFirst();
        } else if (first == song.title) {
            lines.removeFirst();
            if (!lines.isEmpty() && lines.first().trimmed() == song.artist)
                lines.removeFirst();
        }
    }

    QStringList cleaned;
    static const QRegularExpression bpmExpression(
        R"(\b(?:(\d{2,3})\s*BPM|BPM\s*:?\s*(\d{2,3}))\b)",
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression capoExpression(R"(\bCapo\s*:?\s*(\d+)\b)", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression keyExpression(
        R"(^\s*(?:Key|Tonart)\s*:?\s*([A-H](?:#|b)?m?)\s*$)",
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression pageCounterExpression(
        R"(^\s*(?:Page|Seite)\s+\d+\s*(?:/|of|von)\s*\d+\s*$)",
        QRegularExpression::CaseInsensitiveOption);

    for (int lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
        QString line = lines.at(lineIndex);
        line = line.trimmed().isEmpty() ? QString() : line;
        const QString trimmedLine = line.trimmed();
        if (pageCounterExpression.match(trimmedLine).hasMatch())
            continue;
        QString headerComparable = trimmedLine;
        headerComparable.remove(QRegularExpression(R"(\s*[-–—]\s*$)"));
        if (lineIndex < 14 &&
            (headerComparable.compare(song.title, Qt::CaseInsensitive) == 0 ||
             (!song.artist.isEmpty() &&
              trimmedLine.compare(song.artist, Qt::CaseInsensitive) == 0))) {
            continue;
        }
        if (lineIndex < 14 && trimmedLine.size() > 5 &&
            trimmedLine == trimmedLine.toUpper() && !isChordLine(trimmedLine) &&
            !looksLikeSection(trimmedLine)) {
            const auto titleWords = song.title.toLower().split(' ', Qt::SkipEmptyParts);
            const auto lineWords = trimmedLine.toLower().split(' ', Qt::SkipEmptyParts);
            int commonWords = 0;
            for (const QString &word : titleWords) {
                if (word.size() >= 3 && lineWords.contains(word))
                    ++commonWords;
            }
            if (commonWords >= 2)
                continue;
        }
        const auto bpmMatch = bpmExpression.match(line);
        if (bpmMatch.hasMatch()) {
            const QString bpmValue = bpmMatch.captured(1).isEmpty()
                                         ? bpmMatch.captured(2)
                                         : bpmMatch.captured(1);
            song.bpm = bpmValue.toInt();
        }
        const auto capoMatch = capoExpression.match(line);
        if (capoMatch.hasMatch())
            song.capo = capoMatch.captured(1).toInt();
        const auto keyMatch = keyExpression.match(line);
        if (keyMatch.hasMatch())
            song.key = keyMatch.captured(1);
        if (lineIndex < 14 &&
            line.contains(QRegularExpression(R"(\b(BPM|Capo|Key|Tonart)\b)",
                                             QRegularExpression::CaseInsensitiveOption))) {
            continue;
        }
        if ((bpmMatch.hasMatch() || capoMatch.hasMatch() || keyMatch.hasMatch()) &&
            !line.contains(QRegularExpression(R"([A-Za-zÄÖÜäöüß]{4,}\s+[A-Za-zÄÖÜäöüß]{4,})")) &&
            !looksLikeSection(line)) {
            continue;
        }

        if (line.trimmed().contains(QRegularExpression(R"(^[-_]{5,}$)")))
            continue;

        QString section;
        QString sectionRemainder;
        if (parseSectionLine(line, &section, &sectionRemainder)) {
            cleaned << "[" + section + "]";
            if (!sectionRemainder.isEmpty())
                cleaned << sectionRemainder;
        } else {
            cleaned << line;
        }
    }

    song.content = cleaned.join('\n').trimmed();
    return song;
}

QString ChordParser::transposeChord(const QString &chord, int semitones, bool germanNotation)
{
    const auto match = chordToken.match(chord);
    if (!match.hasMatch())
        return chord;

    const QString root = normalizedRoot(pitchForRoot(match.captured(1)) + semitones, germanNotation);
    QString result = root + match.captured(2) + match.captured(3) + match.captured(4);
    if (!match.captured(5).isEmpty())
        result += "/" + normalizedRoot(pitchForRoot(match.captured(5)) + semitones, germanNotation);
    result += match.captured(6);
    return result;
}

QString ChordParser::transposeContent(const QString &content, int semitones, bool germanNotation)
{
    QStringList lines = content.split('\n');
    const QRegularExpression inlineChord(R"(\[([A-H](?:#|b)?[^\]\s]*)\])",
                                         QRegularExpression::CaseInsensitiveOption);

    for (QString &line : lines) {
        if (isChordLine(line)) {
            const QRegularExpression words(R"([A-H](?:#|b)?(?:m|maj|min|dim|aug|sus|add)?\d*(?:[+\-]?\d*)?(?:/[A-H](?:#|b)?)?(?:\([^)]+\))?)",
                                           QRegularExpression::CaseInsensitiveOption);
            qsizetype offset = 0;
            auto iterator = words.globalMatch(line);
            QList<QRegularExpressionMatch> matches;
            while (iterator.hasNext())
                matches << iterator.next();
            for (const auto &match : matches) {
                const QString replacement = transposeChord(match.captured(), semitones, germanNotation);
                line.replace(match.capturedStart() + offset, match.capturedLength(), replacement);
                offset += replacement.size() - match.capturedLength();
            }
        } else {
            qsizetype offset = 0;
            auto iterator = inlineChord.globalMatch(line);
            QList<QRegularExpressionMatch> matches;
            while (iterator.hasNext())
                matches << iterator.next();
            for (const auto &match : matches) {
                const QString replacement = "[" + transposeChord(match.captured(1), semitones, germanNotation) + "]";
                line.replace(match.capturedStart() + offset, match.capturedLength(), replacement);
                offset += replacement.size() - match.capturedLength();
            }
        }
    }
    return lines.join('\n');
}

QList<ImportWarning> ChordParser::analyzeImport(const Song &song)
{
    QList<ImportWarning> warnings;
    if (song.title.trimmed().isEmpty())
        warnings << ImportWarning{-1, QObject::tr("Kein eindeutiger Titel erkannt.")};
    else if (song.title.trimmed().size() <= 3)
        warnings << ImportWarning{-1, QObject::tr("Der erkannte Titel ist ungewöhnlich kurz.")};
    if (song.artist.trimmed().isEmpty())
        warnings << ImportWarning{-1, QObject::tr("Kein Interpret erkannt.")};

    static const QRegularExpression bracketedSection(
        R"(^\s*\[([^\]]+)\]\s*$)");
    static const QRegularExpression knownSection(
        R"(\b(intro|verse|strophe|chorus|refrain|pre[- ]?chorus|bridge|solo|instrumental|outro|interlude|ending|teil|part)\b)",
        QRegularExpression::CaseInsensitiveOption);
    const QStringList lines = song.content.split('\n');
    for (int index = 0; index < lines.size(); ++index) {
        const QString line = lines.at(index);
        const auto sectionMatch = bracketedSection.match(line);
        if (sectionMatch.hasMatch() &&
            !knownSection.match(sectionMatch.captured(1)).hasMatch()) {
            warnings << ImportWarning{
                index,
                QObject::tr("Unbekannter Abschnitt „%1“ – bitte prüfen.")
                    .arg(sectionMatch.captured(1).trimmed())};
            continue;
        }

        if (!isChordLine(line) || index + 1 >= lines.size())
            continue;
        const QString lyric = lines.at(index + 1);
        if (lyric.trimmed().isEmpty() || isChordLine(lyric) ||
            bracketedSection.match(lyric).hasMatch())
            continue;
        const int lastChordCharacter =
            line.lastIndexOf(QRegularExpression(R"(\S)"));
        if (lastChordCharacter > lyric.size() + 6) {
            warnings << ImportWarning{
                index,
                QObject::tr("Ein Akkord steht deutlich hinter dem Ende der folgenden Textzeile.")};
        }
    }
    return warnings;
}
