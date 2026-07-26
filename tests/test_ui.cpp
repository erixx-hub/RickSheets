// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include "blockeditordialog.h"
#include "chordparser.h"
#include "chordeditordialog.h"
#include "importreviewdialog.h"
#include "language.h"
#include "previewwidget.h"
#include "song.h"

#include <QDir>
#include <QAction>
#include <QFile>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QtTest>

#include <algorithm>

class UiTest : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void createsAndRendersMainWindow();
    void restoresMaximizedWindowState();
    void translatesCompleteInterfaceToEnglish();
    void reviewsImportedSongBeforeAcceptance();
    void rendersPdfPageInImportReview();
    void editsAndReordersSongBlocks();
    void assignsChordsToWordsVisually();
    void roundTripsAllImportedPdfSongs();
};

void UiTest::initTestCase()
{
    QCoreApplication::setOrganizationName("RickSheets");
    QCoreApplication::setApplicationName("RickSheetsUiTests");
    const QString settingsDirectory =
        QDir::temp().filePath("ricksheets-ui-test-settings");
    QVERIFY(QDir().mkpath(settingsDirectory));
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope,
                       settingsDirectory);
}

void UiTest::init()
{
    const QString language =
        qEnvironmentVariable("RICKSHEETS_TEST_LANGUAGE", "de");
    QVERIFY(applyRickSheetsLanguage(*QCoreApplication::instance(), language));
}

void UiTest::translatesCompleteInterfaceToEnglish()
{
    QVERIFY(applyRickSheetsLanguage(*QCoreApplication::instance(), "de"));
    storeRickSheetsLanguagePreference("de");

    MainWindow window;
    const auto actions = window.findChildren<QAction *>();
    auto hasAction = [&](const QString &text) {
        return std::any_of(actions.cbegin(), actions.cend(),
                           [&](const QAction *action) {
                               return action->text() == text;
                           });
    };
    QVERIFY(hasAction("&Datei"));
    QVERIFY(hasAction("&Ansicht"));
    auto *arrangement =
        window.findChild<QPlainTextEdit *>("arrangementEditor");
    QVERIFY(arrangement);
    const QString originalContent = "[Strophe]\nA\nEine deutsche Zeile";
    arrangement->setPlainText(originalContent);

    QVERIFY(QMetaObject::invokeMethod(&window, "setEnglishLanguage"));
    QCOMPARE(rickSheetsLanguagePreference(), QString("en"));
    QCOMPARE(arrangement->toPlainText(), originalContent);
    QCOMPARE(QCoreApplication::translate("MainWindow", "BIBLIOTHEK"),
             QString("LIBRARY"));
    QCOMPARE(QCoreApplication::translate("MainWindow", "PDF exportieren"),
             QString("Export PDF"));
    QCOMPARE(QCoreApplication::translate(
                 "QObject", "Kein eindeutiger Titel erkannt."),
             QString("No unambiguous title detected."));
    QVERIFY(hasAction("&File"));
    QVERIFY(hasAction("&View"));
    QVERIFY(hasAction("Language"));
    QVERIFY(hasAction("System language"));
    QVERIFY(hasAction("German"));
    QVERIFY(hasAction("English"));
    const QString switchScreenshot =
        qEnvironmentVariable("RICKSHEETS_LANGUAGE_SWITCH_SCREENSHOT");
    if (!switchScreenshot.isEmpty()) {
        window.resize(1280, 760);
        window.show();
        QTest::qWait(30);
        QImage image(window.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        window.render(&painter);
        painter.end();
        QVERIFY(image.save(switchScreenshot));
    }

    BlockEditorDialog blockDialog("[Verse]\nA\nA line");
    QCOMPARE(blockDialog.windowTitle(), QString("Edit sections"));
    ChordEditorDialog chordDialog("[Verse]\nA\nA line");
    QCOMPARE(chordDialog.windowTitle(), QString("Edit chords visually"));

    Song detected;
    detected.title = "Test";
    detected.content = "[Strophe]\nA\nEine Zeile";
    ImportReviewDialog importDialog("Quelle", detected, "Original");
    QCOMPARE(importDialog.windowTitle(), QString("Review import"));
    QCOMPARE(importDialog.reviewedSong().content, detected.content);

    QVERIFY(QMetaObject::invokeMethod(&window, "setGermanLanguage"));
    QCOMPARE(rickSheetsLanguagePreference(), QString("de"));
    QCOMPARE(arrangement->toPlainText(), originalContent);
    QVERIFY(hasAction("&Datei"));
    QVERIFY(hasAction("&Ansicht"));
    QCOMPARE(rickSheetsEffectiveLanguage(), QString("de"));
    storeRickSheetsLanguagePreference("system");

    QCOMPARE(QCoreApplication::translate("MainWindow", "BIBLIOTHEK"),
             QString("BIBLIOTHEK"));
}

void UiTest::restoresMaximizedWindowState()
{
    QSettings settings;
    settings.remove("window");

    {
        MainWindow window;
        window.showMaximized();
        QTest::qWait(30);
        QVERIFY(window.isMaximized());
        QVERIFY(window.close());
    }

    settings.sync();
    QVERIFY(settings.value("window/maximized").toBool());
    QVERIFY(!settings.value("window/geometry").toByteArray().isEmpty());

    {
        MainWindow restoredWindow;
        restoredWindow.show();
        QTest::qWait(30);
        QVERIFY(restoredWindow.isMaximized());
        restoredWindow.showNormal();
        QTest::qWait(30);
        QVERIFY(restoredWindow.close());
    }

    settings.sync();
    QVERIFY(!settings.value("window/maximized").toBool());
    settings.remove("window");
}

void UiTest::createsAndRendersMainWindow()
{
    QTemporaryDir libraryDirectory;
    QTemporaryDir externalDirectory;
    QVERIFY(libraryDirectory.isValid());
    QVERIFY(externalDirectory.isValid());
    qputenv("RICKSHEETS_LIBRARY_DIR", libraryDirectory.path().toUtf8());
    QSettings().remove("library/recentFiles");

    Song firstSong;
    firstSong.title = "Library Alpha";
    firstSong.artist = "Test Band";
    firstSong.content = "[Verse]\nA\nA line";
    QFile firstFile(libraryDirectory.filePath("alpha.ricksheet"));
    QVERIFY(firstFile.open(QIODevice::WriteOnly));
    QCOMPARE(firstFile.write(firstSong.toJson()), firstSong.toJson().size());
    firstFile.close();

    Song secondSong;
    secondSong.title = "Library Beta";
    secondSong.artist = "Other Artist";
    secondSong.content = "[Chorus]\nD\nAnother line";
    QFile secondFile(libraryDirectory.filePath("beta.ricksheet"));
    QVERIFY(secondFile.open(QIODevice::WriteOnly));
    QCOMPARE(secondFile.write(secondSong.toJson()), secondSong.toJson().size());
    secondFile.close();

    Song externalSong;
    externalSong.title = "External Gamma";
    externalSong.artist = "Guest";
    externalSong.content = "[Verse]\nG\nImported line";
    const QString externalPath = externalDirectory.filePath("gamma.ricksheet");
    QFile externalFile(externalPath);
    QVERIFY(externalFile.open(QIODevice::WriteOnly));
    const QByteArray externalJson = externalSong.toJson();
    QCOMPARE(externalFile.write(externalJson), externalJson.size());
    externalFile.close();
    QSettings settings;
    settings.setValue("library/recentFiles", QStringList{externalPath});
    settings.sync();

    MainWindow window;
    window.resize(1280, 760);
    window.show();
    QTest::qWait(30);
    QVERIFY(window.isVisible());
    QVERIFY(window.windowTitle().contains("RickSheets"));
    QVERIFY(!window.windowIcon().isNull());
    auto *wordmark = window.findChild<QLabel *>("brandWordmark");
    QVERIFY(wordmark);
    QVERIFY(!wordmark->pixmap().isNull());
    auto *library = window.findChild<QListWidget *>("songLibrary");
    auto *search = window.findChild<QLineEdit *>("librarySearch");
    auto *arrangement = window.findChild<QPlainTextEdit *>("arrangementEditor");
    auto *deleteButton = window.findChild<QPushButton *>("deleteLibrarySong");
    QVERIFY(library);
    QVERIFY(search);
    QVERIFY(arrangement);
    QVERIFY(deleteButton);
    QVERIFY(!deleteButton->isEnabled());
    QCOMPARE(arrangement->lineWrapMode(), QPlainTextEdit::NoWrap);
    QCOMPARE(library->count(), 3);
    search->setText("Beta");
    QCOMPARE(library->count(), 1);
    QVERIFY(library->item(0)->text().contains("Library Beta"));
    search->clear();
    QListWidgetItem *externalItem = nullptr;
    for (int index = 0; index < library->count(); ++index) {
        if (library->item(index)->text().contains("External Gamma")) {
            externalItem = library->item(index);
            break;
        }
    }
    QVERIFY(externalItem);
    library->setCurrentItem(externalItem);
    QVERIFY(deleteButton->isEnabled());
    library->itemActivated(externalItem);
    QCOMPARE(
        QDir(libraryDirectory.path())
            .entryList({"*.ricksheet"}, QDir::Files)
            .size(),
        3);
    QVERIFY(window.windowTitle().contains("External Gamma"));

    QVERIFY(QMetaObject::invokeMethod(&window, "setLightTheme"));
    QVERIFY(QApplication::palette().color(QPalette::Window).lightness() >= 128);
    QVERIFY(QMetaObject::invokeMethod(&window, "setDarkTheme"));
    QVERIFY(QApplication::palette().color(QPalette::Window).lightness() < 128);
    qApp->setProperty("ricksheetsSystemDark", true);
    QVERIFY(QMetaObject::invokeMethod(&window, "setSystemTheme"));
    QVERIFY(QApplication::palette().color(QPalette::Window).lightness() < 128);
    qApp->setProperty("ricksheetsSystemDark", false);
    QVERIFY(QMetaObject::invokeMethod(&window, "setSystemTheme"));
    QVERIFY(QApplication::palette().color(QPalette::Window).lightness() >= 128);
    if (qEnvironmentVariable("RICKSHEETS_SCREENSHOT_THEME").compare("light", Qt::CaseInsensitive) == 0)
        QVERIFY(QMetaObject::invokeMethod(&window, "setLightTheme"));

    arrangement->setPlainText("[Verse]\nA       D\nA lyric");
    QTextCursor chordCursor = arrangement->textCursor();
    chordCursor.setPosition(QString("[Verse]\nA ").size());
    arrangement->setTextCursor(chordCursor);
    QCoreApplication::processEvents();
    QVERIFY(arrangement->overwriteMode());
    QTest::keyClicks(arrangement, "G");
    QCOMPARE(arrangement->toPlainText().split('\n').at(1), QString("A G     D"));

    const QString screenshotPath = qEnvironmentVariable("RICKSHEETS_SCREENSHOT");
    if (!screenshotPath.isEmpty()) {
        window.setMinimumSize(1000, 680);
        window.resize(1200, 750);
        QCoreApplication::processEvents();
    }
    QImage image(window.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    window.render(&painter);
    painter.end();
    QVERIFY(!image.isNull());
    if (!screenshotPath.isEmpty())
        QVERIFY(image.save(screenshotPath));

    qunsetenv("RICKSHEETS_LIBRARY_DIR");
}

void UiTest::reviewsImportedSongBeforeAcceptance()
{
    Song detected;
    detected.title = "Falsch erkannt";
    detected.artist = "Unbekannt";
    detected.key = "A";
    detected.bpm = 100;
    detected.capo = 2;
    detected.content = "[Mystery]\nA\nOriginal line";

    ImportReviewDialog dialog(
        "RAW TITLE\n\nVerse\nA\nOriginal line", detected,
        "EINGEFÜGTER ORIGINALTEXT");
    auto *source = dialog.findChild<QPlainTextEdit *>("importSourceText");
    auto *result = dialog.findChild<QPlainTextEdit *>("importReviewResult");
    auto *title = dialog.findChild<QLineEdit *>("importReviewTitle");
    auto *artist = dialog.findChild<QLineEdit *>("importReviewArtist");
    QVERIFY(source);
    QVERIFY(result);
    QVERIFY(title);
    QVERIFY(artist);
    QVERIFY(source->isReadOnly());
    QVERIFY(source->toPlainText().contains("RAW TITLE"));
    auto *warnings = dialog.findChild<QListWidget *>("importWarnings");
    QVERIFY(warnings);
    QVERIFY(warnings->count() >= 1);
    QVERIFY(warnings->item(0)->flags() != Qt::NoItemFlags);
    warnings->itemActivated(warnings->item(0));
    QVERIFY(result->textCursor().selectedText().contains("[Mystery]"));

    title->setText("Korrigierter Titel");
    artist->setText("Test Band");
    result->setPlainText("[Chorus]\nD\nCorrected line");
    const Song reviewed = dialog.reviewedSong();
    QCOMPARE(reviewed.title, QString("Korrigierter Titel"));
    QCOMPARE(reviewed.artist, QString("Test Band"));
    QCOMPARE(reviewed.key, QString("A"));
    QCOMPARE(reviewed.bpm, 100);
    QCOMPARE(reviewed.capo, 2);
    QCOMPARE(reviewed.content, QString("[Chorus]\nD\nCorrected line"));
}

void UiTest::editsAndReordersSongBlocks()
{
    BlockEditorDialog dialog(
        "[Verse 1]\nA\nFirst line\n\n"
        "[Chorus]\nD\nSecond line");
    auto *sections = dialog.findChild<QListWidget *>("blockEditorSections");
    auto *name = dialog.findChild<QLineEdit *>("blockEditorName");
    auto *body = dialog.findChild<QPlainTextEdit *>("blockEditorBody");
    auto *add = dialog.findChild<QPushButton *>("blockEditorAdd");
    QVERIFY(sections);
    QVERIFY(name);
    QVERIFY(body);
    QVERIFY(add);
    QCOMPARE(sections->count(), 2);
    QCOMPARE(sections->item(0)->text(), QString("Verse 1"));
    QCOMPARE(sections->item(1)->text(), QString("Chorus"));

    sections->setCurrentRow(0);
    name->setText("Verse A");
    body->setPlainText("Am\nChanged line");
    QCOMPARE(sections->item(0)->text(), QString("Verse A"));

    QListWidgetItem *chorus = sections->takeItem(1);
    sections->insertItem(0, chorus);
    QString result = dialog.content();
    QVERIFY(result.startsWith("[Chorus]\nD\nSecond line"));
    QVERIFY(result.contains("[Verse A]\nAm\nChanged line"));

    QTest::mouseClick(add, Qt::LeftButton);
    QCOMPARE(sections->count(), 3);
    name->setText("Outro");
    body->setPlainText("E\nLast line");
    result = dialog.content();
    QVERIFY(result.endsWith("[Outro]\nE\nLast line"));

    const QString screenshotPath =
        qEnvironmentVariable("RICKSHEETS_BLOCK_EDITOR_SCREENSHOT");
    if (!screenshotPath.isEmpty()) {
        dialog.resize(900, 650);
        dialog.show();
        QCoreApplication::processEvents();
        QImage screenshot(dialog.size(), QImage::Format_ARGB32_Premultiplied);
        screenshot.fill(Qt::transparent);
        QPainter painter(&screenshot);
        dialog.render(&painter);
        painter.end();
        QVERIFY(screenshot.save(screenshotPath));
    }
}

void UiTest::assignsChordsToWordsVisually()
{
    ChordEditorDialog dialog(
        "[Verse]\n"
        "    Am    F\n"
        "This is a lyric\n"
        "No chords here\n"
        "[Chorus]\n"
        "You [G]inline chord");
    auto *lines = dialog.findChild<QListWidget *>("chordEditorLines");
    QVERIFY(lines);
    QCOMPARE(lines->count(), 2);
    QCOMPARE(dialog.currentWordCount(), 4);

    QVERIFY(dialog.setChordForCurrentWord(0, "C"));
    QVERIFY(dialog.setChordForCurrentWord(1, ""));
    QVERIFY(!dialog.setChordForCurrentWord(2, "not a chord"));
    QString content = dialog.content();
    QVERIFY2(content.contains("C         F\nThis is a lyric"),
             qPrintable(content));
    QVERIFY(content.contains("You [G]inline chord"));

    lines->setCurrentRow(1);
    QCOMPARE(dialog.currentWordCount(), 3);
    QVERIFY(dialog.setChordForCurrentWord(1, "Dm7"));
    content = dialog.content();
    QVERIFY2(content.contains("   Dm7\nNo chords here"), qPrintable(content));

    const QString screenshotPath =
        qEnvironmentVariable("RICKSHEETS_CHORD_EDITOR_SCREENSHOT");
    if (!screenshotPath.isEmpty()) {
        lines->setCurrentRow(0);
        dialog.resize(1100, 680);
        dialog.show();
        QCoreApplication::processEvents();
        QImage screenshot(dialog.size(), QImage::Format_ARGB32_Premultiplied);
        screenshot.fill(Qt::transparent);
        QPainter painter(&screenshot);
        dialog.render(&painter);
        painter.end();
        QVERIFY(screenshot.save(screenshotPath));
    }
}

void UiTest::roundTripsAllImportedPdfSongs()
{
    const QString importedDirectory =
        qEnvironmentVariable("RICKSHEETS_IMPORTED_SONG_DIR");
    if (importedDirectory.isEmpty())
        QSKIP("RICKSHEETS_IMPORTED_SONG_DIR is not set");

    const QFileInfoList files =
        QDir(importedDirectory)
            .entryInfoList({"*.ricksheet"}, QDir::Files, QDir::Name);
    QVERIFY(files.size() >= 1);
    QStringList failures;
    int warningCount = 0;
    int blockEdits = 0;
    int chordEdits = 0;
    QStringList noVisualChordLines;
    for (const QFileInfo &info : files) {
        QFile file(info.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            failures << info.fileName() + ": unreadable";
            continue;
        }
        QString error;
        const Song song = Song::fromJson(file.readAll(), &error);
        if (!error.isEmpty()) {
            failures << info.fileName() + ": " + error;
            continue;
        }

        warningCount += ChordParser::analyzeImport(song).size();
        BlockEditorDialog blockEditor(song.content);
        if (blockEditor.content() != song.content)
            failures << info.fileName() + ": block editor changed untouched content";
        auto *addSection =
            blockEditor.findChild<QPushButton *>("blockEditorAdd");
        if (!addSection) {
            failures << info.fileName() + ": block editor add button missing";
        } else {
            QTest::mouseClick(addSection, Qt::LeftButton);
            if (blockEditor.content() == song.content)
                failures << info.fileName() + ": block edit had no effect";
            else
                ++blockEdits;
        }

        ChordEditorDialog chordEditor(song.content);
        if (chordEditor.content() != song.content)
            failures << info.fileName() + ": chord editor changed untouched content";
        if (chordEditor.currentWordCount() > 0) {
            QString modifiedContent;
            bool changed = chordEditor.setChordForCurrentWord(0, "C") &&
                           chordEditor.content() != song.content;
            if (changed)
                modifiedContent = chordEditor.content();
            if (!changed) {
                ChordEditorDialog alternateChordEditor(song.content);
                changed =
                    alternateChordEditor.setChordForCurrentWord(0, "F#7") &&
                    alternateChordEditor.content() != song.content;
                if (changed)
                    modifiedContent = alternateChordEditor.content();
            }
            if (!changed) {
                failures << info.fileName() + ": chord edit had no effect";
            } else {
                Song edited = song;
                edited.content = modifiedContent;
                PreviewWidget preview;
                preview.setSong(edited);
                if (preview.pageCount() < 1)
                    failures << info.fileName() + ": edited song did not render";
                else
                    ++chordEdits;
            }
        } else {
            noVisualChordLines << info.fileName();
        }
    }
    qInfo().noquote()
        << QString("PDF editor regression: %1 songs, %2 import warnings, "
                   "%3 block edits, %4 chord edits")
               .arg(files.size())
               .arg(warningCount)
               .arg(blockEdits)
               .arg(chordEdits);
    if (!noVisualChordLines.isEmpty())
        qInfo().noquote()
            << "No classic lyric line for visual chord editing:"
            << noVisualChordLines.join(", ");
    QCOMPARE(blockEdits, files.size());
    QCOMPARE(chordEdits + noVisualChordLines.size(), files.size());
    QVERIFY2(failures.isEmpty(), qPrintable(failures.join('\n')));
}

void UiTest::rendersPdfPageInImportReview()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    Song song;
    song.title = "PDF Review";
    song.artist = "Test Band";
    song.content = "[Verse]\nA D\nA lyric line";

    const QString pdfFile = directory.filePath("source.pdf");
    PreviewWidget preview;
    preview.setSong(song);
    QString error;
    QVERIFY2(preview.exportPdf(pdfFile, &error), qPrintable(error));

    ImportReviewDialog dialog(
        "PDF Review\nTest Band\n[Verse]\nA D\nA lyric line", song,
        "ORIGINALTEXT AUS PDF", nullptr, pdfFile);
    auto *tabs = dialog.findChild<QTabWidget *>("importSourceTabs");
    auto *image = dialog.findChild<QLabel *>("pdfPageImage");
    auto *pageNumber = dialog.findChild<QLabel *>("pdfPageNumber");
    auto *zoomIn = dialog.findChild<QPushButton *>("pdfZoomIn");
    QVERIFY(tabs);
    QCOMPARE(tabs->count(), 2);
    QVERIFY(image);
    QVERIFY(!image->pixmap().isNull());
    QVERIFY(pageNumber);
    QVERIFY(pageNumber->text().contains("Seite 1 von 1"));
    QVERIFY(zoomIn);
    const QSize originalSize = image->pixmap().size();
    QTest::mouseClick(zoomIn, Qt::LeftButton);
    QVERIFY(image->pixmap().size().width() > originalSize.width());

    const QString screenshotPath =
        qEnvironmentVariable("RICKSHEETS_IMPORT_REVIEW_SCREENSHOT");
    if (!screenshotPath.isEmpty()) {
        dialog.resize(1180, 760);
        dialog.show();
        QCoreApplication::processEvents();
        QImage screenshot(dialog.size(), QImage::Format_ARGB32_Premultiplied);
        screenshot.fill(Qt::transparent);
        QPainter painter(&screenshot);
        dialog.render(&painter);
        painter.end();
        QVERIFY(screenshot.save(screenshotPath));
    }
}

QTEST_MAIN(UiTest)
#include "test_ui.moc"
