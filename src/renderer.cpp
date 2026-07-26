// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#include "renderer.h"
#include "chordparser.h"

#include <QAbstractTextDocumentLayout>
#include <QPageSize>
#include <QRegularExpression>
#include <QSet>
#include <QTextBlock>
#include <QTextLayout>

namespace {
struct SectionBlock {
    QStringList elements;
    int weight = 0;
};

QString escaped(QString value)
{
    return value.toHtmlEscaped();
}

QString renderInlineChords(const QString &line)
{
    QString result = escaped(line);
    static const QRegularExpression chord(R"(\[([^\]]+)\])");
    result.replace(chord, R"(<span class="inline-chord">\1</span>)");
    return result;
}

QString chordHtml(const QString &line)
{
    return QString("<div class=\"chords\">%1</div>")
        .arg(escaped(line).replace(" ", "&nbsp;"));
}

QString lyricHtml(const QString &line)
{
    return QString("<div class=\"lyrics\">%1</div>").arg(renderInlineChords(line));
}

QList<SectionBlock> buildBlocks(const Song &song)
{
    QList<SectionBlock> blocks;
    SectionBlock current;
    const QStringList lines = song.content.split('\n');
    static const QRegularExpression pageCounterExpression(
        R"(^\s*(?:Page|Seite)\s+\d+\s*(?:/|of|von)\s*\d+\s*$)",
        QRegularExpression::CaseInsensitiveOption);

    auto flush = [&] {
        if (!current.elements.isEmpty()) {
            blocks << current;
            current = {};
        }
    };

    for (int i = 0; i < lines.size(); ++i) {
        const QString line = lines.at(i);
        const QString trimmed = line.trimmed();
        if (pageCounterExpression.match(trimmed).hasMatch())
            continue;
        if (trimmed.startsWith('[') && trimmed.endsWith(']')) {
            flush();
            current.elements << QString("<hr class=\"section-rule\"><div class=\"section\"><span>[%1]</span></div>")
                                    .arg(escaped(trimmed.mid(1, trimmed.size() - 2)));
            current.weight += 2;
        } else if (trimmed.isEmpty()) {
            current.elements << "<div class=\"space\"></div>";
            current.weight += 1;
        } else if (ChordParser::isChordLine(line) && i + 1 < lines.size() &&
                   !lines.at(i + 1).trimmed().isEmpty() &&
                   !ChordParser::isChordLine(lines.at(i + 1)) &&
                   !(lines.at(i + 1).trimmed().startsWith('[') &&
                     lines.at(i + 1).trimmed().endsWith(']'))) {
            current.elements << QString("<div class=\"line-pair\">%1%2</div>")
                                    .arg(chordHtml(line), lyricHtml(lines.at(++i)));
            current.weight += 2;
        } else if (ChordParser::isChordLine(line)) {
            current.elements << chordHtml(line);
            current.weight += 1;
        } else {
            current.elements << lyricHtml(line);
            current.weight += 1;
        }
    }
    flush();
    return blocks;
}

QString stylesheet(bool compact, qreal compactPointSize = 6.8,
                   bool tightOneColumn = false, int layoutScale = 100)
{
    const qreal factor = qBound(80, layoutScale, 160) / 100.0;
    const qreal baseBodySize =
        compact ? compactPointSize : (tightOneColumn ? 8.8 : 9.3);
    const QString bodySize =
        QString::number(baseBodySize * factor, 'f', 1) + "pt";
    const QString lineHeight = compact ? "1.0" : (tightOneColumn ? "1.06" : "1.14");
    const QString sectionMargin =
        QString::number((compact ? 0.4 : (tightOneColumn ? 0.8 : 1.2)) *
                            factor,
                        'f', 1) +
        "mm";
    const QString rowHeight =
        QString::number((compact ? 2.1 : (tightOneColumn ? 3.2 : 3.6)) *
                            factor,
                        'f', 1) +
        "mm";
    const QString titleSize =
        QString::number((compact ? 12.0 : (tightOneColumn ? 14.0 : 15.0)) *
                            factor,
                        'f', 1) +
        "pt";
    const QString artistSize =
        QString::number((compact ? 8.0 : (tightOneColumn ? 9.5 : 10.0)) *
                            factor,
                        'f', 1) +
        "pt";
    const QString metaSize = QString::number(8.3 * factor, 'f', 1) + "pt";
    const QString inlineChordSize =
        QString::number(7.3 * factor, 'f', 1) + "pt";
    return QString(R"(
@page { size: A4; margin: 8mm; }
body { font-family:"DejaVu Sans","Arial",sans-serif; color:#161616; font-size:%1; line-height:%2; }
h1 { font-size:%5; margin:0 0 0.7mm; font-weight:800; text-transform:uppercase; }
.artist { font-size:%6; font-weight:700; border-bottom:1px solid #777; padding-bottom:1mm; margin-bottom:0.7mm; }
.meta { color:#555; font-size:%7; margin-bottom:2mm; }
.section-rule { border:0; border-top:1px solid #999; height:0; margin:%3 0 0.5mm; }
.section { margin-top:0; padding-top:0; font-weight:700; page-break-after:avoid; }
.section span { background:#f2dd32; color:#000; border:0.35pt solid #555; padding:0.25mm 1mm; }
.section-block { page-break-inside:avoid; }
.line-pair { page-break-inside:avoid; font-family:"DejaVu Sans Mono","Consolas",monospace; }
.line-pair .chords, .line-pair .lyrics { white-space:pre; }
.chords { color:#8b6900; font-weight:800; white-space:pre-wrap; margin-top:0.5mm; min-height:%4; page-break-after:avoid; }
.lyrics { min-height:%4; white-space:pre-wrap; }
.inline-chord { color:#8b6900; font-weight:800; vertical-align:super; font-size:%8; padding-right:0.8mm; }
.space { height:0.7mm; }
.columns { width:100%; border:0; table-layout:fixed; }
.columns td { width:48%; vertical-align:top; }
.columns td.gutter { width:4%; }
.page-break { page-break-before:always; height:0; margin:0; padding:0; font-size:1px; }
)").arg(bodySize, lineHeight, sectionMargin, rowHeight, titleSize, artistSize,
        metaSize, inlineChordSize);
}

QList<QStringList> distributeBlocks(const QList<SectionBlock> &blocks, int groupCount)
{
    QList<QStringList> groups;
    if (groupCount <= 0)
        return groups;

    int remainingWeight = 0;
    for (const auto &block : blocks)
        remainingWeight += block.weight;

    // The first physical page also contains title, artist and metadata. Give
    // both of its columns noticeably less content than later pages.
    const int headerReserve = groupCount > 2 ? 12 : 0;
    const int baseTarget = qMax(
        1, qCeil(qreal(remainingWeight + (groupCount > 2 ? 2 * headerReserve : 0)) /
                 groupCount));

    int blockIndex = 0;
    for (int groupIndex = 0; groupIndex < groupCount; ++groupIndex) {
        QStringList group;
        int groupWeight = 0;
        const int groupsLeft = groupCount - groupIndex;
        const int evenTarget = qMax(1, qCeil(qreal(remainingWeight) / groupsLeft));
        const int pageTarget = groupIndex < 2 ? qMax(1, baseTarget - headerReserve)
                                              : baseTarget;
        const int target = qMin(evenTarget, pageTarget);

        while (blockIndex < blocks.size()) {
            const auto &block = blocks.at(blockIndex);
            const int blocksAfter = blocks.size() - blockIndex - 1;
            const int groupsAfter = groupsLeft - 1;
            const bool mustLeaveBlocks = blocksAfter < groupsAfter;
            if (!group.isEmpty() && !mustLeaveBlocks &&
                groupWeight + block.weight > target)
                break;
            group << QString("<div class=\"section-block\">%1</div>")
                         .arg(block.elements.join('\n'));
            groupWeight += block.weight;
            remainingWeight -= block.weight;
            ++blockIndex;
            if (mustLeaveBlocks)
                break;
        }
        groups << group;
    }

    while (blockIndex < blocks.size()) {
        groups.last() << QString("<div class=\"section-block\">%1</div>")
                             .arg(blocks.at(blockIndex).elements.join('\n'));
        ++blockIndex;
    }
    return groups;
}

int longestGridLine(const Song &song)
{
    int longestLine = 0;
    for (const QString &line : song.content.split('\n')) {
        if (!(line.trimmed().startsWith('[') && line.trimmed().endsWith(']')))
            longestLine = qMax(longestLine, line.size());
    }
    return longestLine;
}

qreal rawCompactPointSizeForSong(const Song &song)
{
    const int longestLine = longestGridLine(song);
    if (longestLine <= 0)
        return 6.8;
    // Keep a safety margin for table and font-metric differences between
    // preview/PDF and platforms. About 48 grid characters fit reliably.
    return 6.8 * 48.0 / longestLine;
}

qreal compactPointSizeForSong(const Song &song)
{
    // Compact only moderately. If 6 pt is still too wide, retaining the
    // chord grid on an additional page is preferable to tiny print.
    return qBound(6.0, rawCompactPointSizeForSong(song), 6.8);
}

QString documentHtml(const Song &song, int columnPages = 0, bool compact = false,
                     bool tightOneColumn = false)
{
    const QList<SectionBlock> blocks = buildBlocks(song);
    QString renderedBody;

    if (columnPages > 0 && blocks.size() >= 2) {
        const auto groups = distributeBlocks(blocks, columnPages * 2);
        QStringList pageTables;
        for (int page = 0; page < columnPages; ++page) {
            const QString left = groups.value(page * 2).join('\n');
            const QString right = groups.value(page * 2 + 1).join('\n');
            pageTables << QString(
                "<table class=\"columns\" width=\"100%\" cellspacing=\"0\" cellpadding=\"0\"><tr>"
                "<td width=\"48%\">%1</td><td class=\"gutter\" width=\"4%\"></td>"
                "<td width=\"48%\">%2</td></tr></table>")
                              .arg(left, right);
        }
        renderedBody = pageTables.join("<div class=\"page-break\">&#8203;</div>");
    } else {
        QStringList elements;
        for (const auto &block : blocks)
            elements << block.elements.join('\n');
        renderedBody = elements.join('\n');
    }

    QStringList meta;
    if (song.bpm > 0)
        meta << QString::number(song.bpm) + " BPM";
    if (!song.key.isEmpty())
        meta << "Tonart " + escaped(song.key);
    if (song.capo > 0)
        meta << "Capo " + QString::number(song.capo);

    return QString(
        "<!doctype html><html><head><meta charset=\"utf-8\"><style>%1</style></head>"
        "<body><h1>%2</h1><div class=\"artist\">%3</div><div class=\"meta\">%4</div>%5"
        "</body></html>")
        .arg(stylesheet(compact, compactPointSizeForSong(song), tightOneColumn,
                        song.layoutScale),
             escaped(song.title.isEmpty() ? "Unbenannter Song" : song.title),
             escaped(song.artist),
             meta.join(" · "),
             renderedBody);
}

void setDocumentHtml(QTextDocument &document, const QString &html)
{
    document.setHtml(html);
    // Force layout before reading document.size().
    document.documentLayout()->documentSize();
}

bool hasWrappedGridLines(const QTextDocument &document, const Song &song)
{
    QSet<QString> sourceLines;
    for (const QString &line : song.content.split('\n')) {
        const QString trimmed = line.trimmed();
        if (!trimmed.isEmpty() &&
            !(trimmed.startsWith('[') && trimmed.endsWith(']')))
            sourceLines.insert(line);
    }

    for (QTextBlock block = document.begin(); block.isValid(); block = block.next()) {
        if (!sourceLines.contains(block.text()))
            continue;
        const QTextLayout *layout = block.layout();
        if (layout && layout->lineCount() > 1)
            return true;
    }
    return false;
}
}

QString SongRenderer::toHtml(const Song &song)
{
    return documentHtml(song);
}

void SongRenderer::configureDocument(QTextDocument &document, const Song &song)
{
    const QPageSize page(QPageSize::A4);
    document.setPageSize(page.size(QPageSize::Point));
    document.setDocumentMargin(23.0);

    const QString oneColumnHtml = documentHtml(song);
    setDocumentHtml(document, oneColumnHtml);
    const int oneColumnPages = pageCount(document);
    if (oneColumnPages <= 1)
        return;

    // A near-empty second page should not be accepted before trying a very
    // small one-column tightening. This keeps the readable single-column
    // layout and never invokes the much smaller two-column font.
    const QString tightOneColumnHtml = documentHtml(song, 0, false, true);
    setDocumentHtml(document, tightOneColumnHtml);
    if (pageCount(document) == 1 && !hasWrappedGridLines(document, song))
        return;
    setDocumentHtml(document, oneColumnHtml);

    // Try complete physical pages in ascending order. Each generated page owns
    // its left and right column, so reading order never jumps back to page 1.
    for (int targetPages = 1; targetPages <= oneColumnPages; ++targetPages) {
        const bool normalWidthFits = longestGridLine(song) <= 44;
        if (normalWidthFits) {
            const QString normalColumns = documentHtml(song, targetPages, false);
            setDocumentHtml(document, normalColumns);
            if (pageCount(document) == targetPages &&
                !hasWrappedGridLines(document, song))
                return;
        }

        const QString compactColumns = documentHtml(song, targetPages, true);
        setDocumentHtml(document, compactColumns);
        if (pageCount(document) == targetPages &&
            !hasWrappedGridLines(document, song))
            return;
    }
    setDocumentHtml(document, oneColumnHtml);
}

int SongRenderer::pageCount(const QTextDocument &document)
{
    const qreal pageHeight = document.pageSize().height();
    if (pageHeight <= 0)
        return 1;
    return qMax(1, qCeil(document.size().height() / pageHeight));
}
