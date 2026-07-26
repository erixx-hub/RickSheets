// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chordparser.h"
#include "previewwidget.h"
#include "renderer.h"
#include "song.h"

#include <QFileInfo>
#include <QAbstractTextDocumentLayout>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextCursor>
#include <QTextTable>
#include <QtTest>

class CoreTest : public QObject {
    Q_OBJECT
private slots:
    void jsonRoundTrip();
    void detectsChordLines();
    void importsHeadingAndMetadata();
    void removesWebsiteNoiseAndChordDiagrams();
    void reportsConcreteImportWarnings();
    void importsPdfStyleHeaderWithHint();
    void restoresPdfColumnReadingOrder();
    void preservesChordPositionsInPdfColumns();
    void normalizesIncompleteSectionBrackets();
    void transposesChords();
    void rendersPages();
    void scalesShortSongs();
    void anchorsChordToExactLyricPosition();
    void usesColumnsForLongSongs();
    void rejectsColumnsThatWrapGridLines();
    void multiPageColumnsFollowPhysicalReadingOrder();
    void exportsPdf();
    void loadsAllExampleSongs();
};

void CoreTest::jsonRoundTrip()
{
    Song original;
    original.title = "Red Light";
    original.artist = "Number Nine";
    original.key = "Am";
    original.bpm = 123;
    original.layoutScale = 145;
    original.content = "[Verse 1]\nAm F C G";
    QString error;
    const Song restored = Song::fromJson(original.toJson(), &error);
    QCOMPARE(error, QString());
    QCOMPARE(restored.title, original.title);
    QCOMPARE(restored.content, original.content);
    QCOMPARE(restored.bpm, 123);
    QCOMPARE(restored.layoutScale, 145);
}

void CoreTest::detectsChordLines()
{
    QVERIFY(ChordParser::isChordLine("    Am       F       C  G"));
    QVERIFY(ChordParser::isChordLine("| A | D E | A |"));
    QVERIFY(!ChordParser::isChordLine("And now the danger sign is on"));
}

void CoreTest::importsHeadingAndMetadata()
{
    const Song song = ChordParser::importText(
        "Red Light - Number Nine\n123 BPM\nCapo 2\n\n[Verse 1]\nAm F C G\nPage 1/2");
    QCOMPARE(song.title, QString("Red Light"));
    QCOMPARE(song.artist, QString("Number Nine"));
    QCOMPARE(song.bpm, 123);
    QCOMPARE(song.capo, 2);
    QVERIFY(song.content.contains("[Verse 1]"));
    QVERIFY(!song.content.contains("Page 1/2"));
}

void CoreTest::removesWebsiteNoiseAndChordDiagrams()
{
    const QString copiedPage =
        "WE CAN LEAVE THE WORLD BEHIND CHORDS by Sasha\n"
        "We Can Leave The World Behind - Sasha\n"
        "Difficulty: Intermediate\n"
        "Tuning: E A D G B E\n"
        "Key: A\n"
        "Capo: 3\n"
        "Author: Example User\n"
        "Tabbed by foma\n"
        "Transcribed by Another User\n"
        "Last edit: Jul 25, 2026\n"
        "https://tabs.example.test/song\n"
        "A\n"
        "x 0 2 2 2 0\n"
        "Dmaj7\n"
        "x\n0\n0\n2\n2\n2\n"
        "Download PDF\n"
        "Transpose -3\n"
        "\n"
        "[Intro]\n"
        "[ch]A[/ch] [ch]Dmaj7[/ch]\n"
        "\n"
        "[Verse 1]\n"
        "A                 Dmaj7\n"
        "When you feel in times of trouble\n"
        "\n"
        "[Chorus]\n"
        "A\n"
        "We can leave the world behind\n"
        "Page 1/2\n";

    const QString cleaned = ChordParser::cleanImportText(copiedPage);
    QVERIFY(!cleaned.contains("Difficulty"));
    QVERIFY(!cleaned.contains("Tuning"));
    QVERIFY(!cleaned.contains("Author"));
    QVERIFY(!cleaned.contains("Tabbed by"));
    QVERIFY(!cleaned.contains("Transcribed by"));
    QVERIFY(!cleaned.contains("Download PDF"));
    QVERIFY(!cleaned.contains("x 0 2 2 2 0"));
    QVERIFY(!cleaned.contains("\nx\n0\n0\n2\n2\n2"));
    QVERIFY(!cleaned.contains("[ch]"));
    QVERIFY(cleaned.contains("When you feel in times of trouble"));

    const Song song = ChordParser::importText(copiedPage);
    QCOMPARE(song.title, QString("We Can Leave The World Behind"));
    QCOMPARE(song.artist, QString("Sasha"));
    QCOMPARE(song.key, QString("A"));
    QCOMPARE(song.capo, 3);
    QVERIFY(song.content.contains("[Intro]\nA Dmaj7"));
    QVERIFY(song.content.contains("We can leave the world behind"));
    QVERIFY(!song.content.contains("Difficulty"));
    QVERIFY(!song.content.contains("Page 1/2"));
}

void CoreTest::reportsConcreteImportWarnings()
{
    Song song;
    song.title = "A";
    song.content =
        "[Verse 1]\n"
        "A                                      D\n"
        "Short lyric\n"
        "\n"
        "[Mystery]\n"
        "G\n"
        "Another lyric";
    const QList<ImportWarning> warnings = ChordParser::analyzeImport(song);
    QVERIFY(warnings.size() >= 4);
    bool foundUnknownSection = false;
    bool foundChordPosition = false;
    for (const ImportWarning &warning : warnings) {
        foundUnknownSection |= warning.message.contains("Unbekannter Abschnitt");
        foundChordPosition |= warning.message.contains("Akkord");
    }
    QVERIFY(foundUnknownSection);
    QVERIFY(foundChordPosition);
}

void CoreTest::importsPdfStyleHeaderWithHint()
{
    const Song song = ChordParser::importText(
        "Well, he heard every song                 C\n"
        "SLEEPING THROUGH THE PARTY\n"
        "BPM: 100\n"
        "[Strophe 1]\nC\nA lyric line",
        "Sleepin through the party.pdf");
    QCOMPARE(song.title, QString("SLEEPING THROUGH THE PARTY"));
    QCOMPARE(song.bpm, 100);
}

void CoreTest::restoresPdfColumnReadingOrder()
{
    const QString extracted =
        "Title - Artist                                      [Chorus]\n"
        "[Verse 1]                                           G\n"
        "C                                                   Right lyric one\n"
        "Left lyric one                                      D\n"
        "G                                                   Right lyric two\n"
        "Left lyric two                                      G\n";
    const QString normalized = ChordParser::normalizePdfColumns(extracted);
    const int verse = normalized.indexOf("[Verse 1]");
    const int leftLyric = normalized.indexOf("Left lyric two");
    const int chorus = normalized.indexOf("[Chorus]");
    const int rightLyric = normalized.indexOf("Right lyric one");
    QVERIFY(verse >= 0);
    QVERIFY(leftLyric > verse);
    QVERIFY(chorus > leftLyric);
    QVERIFY(rightLyric > chorus);
}

void CoreTest::preservesChordPositionsInPdfColumns()
{
    const QString extracted =
        "Just One Beer - Low Places                              G                              D\n"
        "BPM: 186 - Capo 2!                                     Just one beer - another line\n"
        "[Verse 1]                                              [Chorus]\n"
        "           Hm                                                G                             D\n"
        "I got no plans, I just needed some peace                Just one beer - I need some quiet\n"
        "                  A                              Hm                   A\n"
        "Found me a table, sittin' down with ease                Why is it so unclear\n";
    const QString normalized = ChordParser::normalizePdfColumns(extracted);
    const Song song = ChordParser::importText(normalized, "Just One Beer.pdf");
    QVERIFY2(
        song.content.contains(
            QRegularExpression(R"(^\s*G\s{8,}D\s*$)",
                               QRegularExpression::MultilineOption)),
        qPrintable(song.content));
    QVERIFY(song.content.indexOf("[Chorus]") >
            song.content.indexOf("Found me a table"));
}

void CoreTest::normalizesIncompleteSectionBrackets()
{
    const Song song = ChordParser::importText(
        "Demo - Band\n"
        "(Intro) | A | D |\n"
        "(Verse 1)\nA\nA lyric\n"
        "Chorus]\nD\nAnother lyric\n"
        "[Bridge\nE\nLast lyric\n"
        "------------------------------------------------\n");
    QVERIFY2(song.content.contains("[Intro]\n| A | D |"), qPrintable(song.content));
    QVERIFY(song.content.contains("[Verse 1]"));
    QVERIFY(song.content.contains("[Chorus]"));
    QVERIFY(song.content.contains("[Bridge]"));
    QVERIFY(!song.content.contains("-----"));
    const QString html = SongRenderer::toHtml(song);
    QVERIFY(html.contains("<span>[Verse 1]</span>"));
    QVERIFY(html.contains("<hr class=\"section-rule\">"));
}

void CoreTest::transposesChords()
{
    QCOMPARE(ChordParser::transposeChord("Am", 3), QString("Cm"));
    QCOMPARE(ChordParser::transposeChord("E7/G#", 1), QString("F7/A"));
    const QString result = ChordParser::transposeContent(
        "[Verse]\nA D E\nThis is [Am]text", 2);
    QVERIFY(result.contains("H E F#"));
    QVERIFY(result.contains("[Hm]text"));
}

void CoreTest::rendersPages()
{
    Song song;
    song.title = "Test";
    song.content = "[Verse]\nA D E\nA line";
    QTextDocument document;
    SongRenderer::configureDocument(document, song);
    QCOMPARE(SongRenderer::pageCount(document), 1);
    QVERIFY(SongRenderer::toHtml(song).contains("Test"));
}

void CoreTest::scalesShortSongs()
{
    Song song;
    song.title = "Short Song";
    song.artist = "Test";
    song.content = "[Verse]\nA\nA short line";
    song.layoutScale = 150;
    const QString enlarged = SongRenderer::toHtml(song);
    song.layoutScale = 100;
    const QString normal = SongRenderer::toHtml(song);
    QVERIFY(enlarged.contains("font-size:14.0pt"));
    QVERIFY(normal.contains("font-size:9.3pt"));
    QVERIFY(enlarged != normal);
}

void CoreTest::anchorsChordToExactLyricPosition()
{
    const QString lyric = "When you feel in times of trouble when you're feeling";
    const int secondFeeling = lyric.lastIndexOf("feeling");
    Song song;
    song.title = "Anchor Test";
    song.content = "[Verse 1]\n" + QString(secondFeeling, ' ') + "Dmaj7\n" + lyric;
    const QString html = SongRenderer::toHtml(song);
    QVERIFY(html.contains(QString("&nbsp;").repeated(secondFeeling) + "Dmaj7"));
    QVERIFY(html.contains("DejaVu Sans Mono"));
    QVERIFY(html.contains("line-pair .chords, .line-pair .lyrics { white-space:pre; }"));
}

void CoreTest::usesColumnsForLongSongs()
{
    Song song;
    song.title = "Long Test";
    QStringList sections;
    for (int section = 1; section <= 7; ++section) {
        sections << QString("[Verse %1]").arg(section);
        for (int line = 0; line < 4; ++line)
            sections << "C G Am F" << "A readable lyric line for the layout";
    }
    song.content = sections.join('\n');
    QTextDocument document;
    SongRenderer::configureDocument(document, song);
    QCOMPARE(SongRenderer::pageCount(document), 1);
    QVERIFY(document.toHtml().contains("<table"));
    const QTextCursor verseThree = document.find("Verse 3");
    const QTextCursor verseFour = document.find("Verse 4");
    QVERIFY(verseThree.currentTable());
    QVERIFY(verseFour.currentTable());
    QCOMPARE(verseThree.currentTable()->cellAt(verseThree).column(), 0);
    QCOMPARE(verseFour.currentTable()->cellAt(verseFour).column(), 2);
}

void CoreTest::rejectsColumnsThatWrapGridLines()
{
    Song song;
    song.title = "Long lines";
    QStringList content;
    for (int section = 1; section <= 6; ++section) {
        content << QString("[Verse %1]").arg(section);
        for (int line = 0; line < 4; ++line) {
            content << "C                         G";
            content << "This intentionally long lyric line must remain on one visual grid line";
        }
    }
    song.content = content.join('\n');

    QTextDocument document;
    SongRenderer::configureDocument(document, song);
    for (QTextBlock block = document.begin(); block.isValid(); block = block.next()) {
        if (!block.text().startsWith("This intentionally"))
            continue;
        QVERIFY(block.layout());
        QCOMPARE(block.layout()->lineCount(), 1);
    }
}

void CoreTest::multiPageColumnsFollowPhysicalReadingOrder()
{
    Song song;
    song.title = "Page Flow";
    QStringList content;
    for (int section = 1; section <= 16; ++section) {
        content << QString("[Section %1 Chorus]").arg(section);
        for (int line = 0; line < 5; ++line)
            content << "Am F C G" << QString("Unique lyric %1.%2").arg(section).arg(line);
    }
    song.content = content.join('\n');

    QTextDocument document;
    SongRenderer::configureDocument(document, song);
    QVERIFY(SongRenderer::pageCount(document) >= 2);

    int previousReadingGroup = -1;
    qreal previousY = -1;
    for (int section = 1; section <= 16; ++section) {
        const QTextCursor cursor = document.find(QString("[Section %1 Chorus]").arg(section));
        QVERIFY(!cursor.isNull());
        const QRectF rect = document.documentLayout()->blockBoundingRect(cursor.block());
        const int page = qFloor(rect.top() / document.pageSize().height());
        const int column = rect.left() > document.pageSize().width() / 2 ? 1 : 0;
        const int readingGroup = page * 2 + column;
        QVERIFY(readingGroup >= previousReadingGroup);
        if (readingGroup == previousReadingGroup)
            QVERIFY(rect.top() >= previousY);
        previousReadingGroup = readingGroup;
        previousY = rect.top();
    }
}

void CoreTest::exportsPdf()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    Song song;
    song.title = "PDF Test";
    song.artist = "RickSheets";
    song.content = "[Verse]\nA D E\nA line";
    PreviewWidget preview;
    preview.setSong(song);
    const QString fileName = directory.filePath("test.pdf");
    QString error;
    QVERIFY2(preview.exportPdf(fileName, &error), qPrintable(error));
    QVERIFY(QFileInfo(fileName).size() > 500);
}

void CoreTest::loadsAllExampleSongs()
{
    const QString sourceDirectory = qEnvironmentVariable(
        "RICKSHEETS_EXAMPLE_SOURCE_DIR",
        QStringLiteral(RICKSHEETS_SOURCE_DIR) + "/examples");
    QDir examples(sourceDirectory);
    const auto files = examples.entryInfoList({"*.ricksheet"}, QDir::Files, QDir::Name);
    const bool customSource = qEnvironmentVariableIsSet("RICKSHEETS_EXAMPLE_SOURCE_DIR");
    QVERIFY(files.size() >= (customSource ? 1 : 4));
    for (const QFileInfo &info : files) {
        QFile file(info.absoluteFilePath());
        QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(info.fileName()));
        QString error;
        const Song song = Song::fromJson(file.readAll(), &error);
        QVERIFY2(error.isEmpty(), qPrintable(info.fileName() + ": " + error));
        QVERIFY2(!song.title.isEmpty(), qPrintable(info.fileName()));
        QVERIFY2(!song.content.isEmpty(), qPrintable(info.fileName()));
        QTextDocument document;
        SongRenderer::configureDocument(document, song);
        QVERIFY(SongRenderer::pageCount(document) >= 1);
        const QString pdfDirectory = qEnvironmentVariable("RICKSHEETS_EXAMPLE_PDF_DIR");
        if (!pdfDirectory.isEmpty()) {
            QDir().mkpath(pdfDirectory);
            PreviewWidget preview;
            preview.setSong(song);
            QString exportError;
            QVERIFY2(preview.exportPdf(
                         QDir(pdfDirectory).filePath(info.completeBaseName() + ".pdf"),
                         &exportError),
                     qPrintable(exportError));
        }
    }
}

QTEST_MAIN(CoreTest)
#include "test_core.moc"
