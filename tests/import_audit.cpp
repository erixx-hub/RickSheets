// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chordparser.h"
#include "renderer.h"

#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QGuiApplication>
#include <QProcess>
#include <QTextDocument>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QGuiApplication application(argc, argv);
    QTextStream out(stdout);
    if (application.arguments().size() < 2) {
        out << "Usage: ricksheets_import_audit DOCS_DIRECTORY\n";
        return 2;
    }

    QDir docs(application.arguments().at(1));
    const QString outputDirectory = application.arguments().size() >= 3
                                        ? application.arguments().at(2)
                                        : QString();
    if (!outputDirectory.isEmpty())
        QDir().mkpath(outputDirectory);
    const auto files = docs.entryInfoList({"*.pdf"}, QDir::Files, QDir::Name);
    int failures = 0;
    out << "file\ttitle\tartist\tchord-lines\tsections\tpages\tstatus\n";

    for (const QFileInfo &file : files) {
        QProcess process;
        process.start("pdftotext", {"-raw", file.absoluteFilePath(), "-"});
        if (!process.waitForFinished(15000) || process.exitCode() != 0) {
            out << file.fileName() << "\t-\t-\t0\t0\t0\tPDF extraction failed\n";
            ++failures;
            continue;
        }

        const QString extracted = QString::fromUtf8(process.readAllStandardOutput());
        const Song song = ChordParser::importText(extracted, file.fileName());
        int chordLines = 0;
        int sections = 0;
        for (const QString &line : song.content.split('\n')) {
            if (ChordParser::isChordLine(line))
                ++chordLines;
            if (line.trimmed().startsWith('[') && line.trimmed().endsWith(']'))
                ++sections;
        }

        QTextDocument document;
        SongRenderer::configureDocument(document, song);
        const int pages = SongRenderer::pageCount(document);
        bool valid = !song.title.isEmpty() && !song.content.isEmpty();
        if (file.fileName().startsWith("Follow Me"))
            valid = valid && sections >= 8;
        if (file.fileName().startsWith("Keeper of the stars"))
            valid = valid && sections >= 5;
        if (!valid)
            ++failures;
        out << file.fileName() << '\t' << song.title.left(28) << '\t'
            << song.artist.left(20) << '\t' << chordLines << '\t'
            << sections << '\t' << pages << '\t'
            << (valid ? "ok" : "invalid") << '\n';
        if (valid && !outputDirectory.isEmpty()) {
            QFile output(QDir(outputDirectory).filePath(file.completeBaseName() + ".ricksheet"));
            if (output.open(QIODevice::WriteOnly | QIODevice::Truncate))
                output.write(song.toJson());
        }
    }
    out.flush();
    return failures == 0 ? 0 : 1;
}
