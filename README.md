# RickSheets

RickSheets ist ein plattformneutraler Chordsheet-Editor mit A4-Livevorschau. Der aktuelle Stand ist ein erster vertikaler Prototyp.

Copyright (C) 2026 Erik Heidenreich. RickSheets ist freie Software unter
der Lizenz GPL-3.0-or-later. Der vollständige Lizenztext steht in
[`LICENSE`](LICENSE).

## Bereits funktionsfähig

- unformatierten Chordsheet-Text einfügen
- textbasierte PDFs über `pdftotext` importieren
- Titel, Interpret, BPM und Capo grundlegend erkennen
- Abschnittsnamen normalisieren
- Songtext und Akkorde im Rohtexteditor korrigieren
- Akkorde als eigene Zeilen oder inline wie `[Am]Text` eingeben
- vollständigen Song um Halbtonschritte transponieren
- deutsche Akkordschreibweise mit `H` und `B`
- A4-Livevorschau mit sichtbaren Einzelseiten
- automatische Zweispaltenverteilung für längere Songs
- Erkennung und Rekonstruktion vorhandener PDF-Spalten in Lesereihenfolge
- aktuelle Seitenanzahl anzeigen
- Song als offene `.ricksheet`-JSON-Datei speichern und wieder öffnen
- lokale, durchsuchbare Songbibliothek mit Öffnen per Doppelklick
- automatische Bibliothekskopien für Text-, PDF- und externe Songimporte
- PDF mit derselben Qt-Rendering-Quelle wie die Vorschau exportieren
- Linux-AppImage als getestetes Paket

## Noch nicht im ersten Prototyp

- Prüfansicht für unsichere Importergebnisse
- visueller Block- und Akkordeditor
- erweiterte Bibliotheksfunktionen wie Tags, Setlisten und SQLite-Index
- Windows-Paket

Diese Funktionen sind im [Hauptkonzept](KONZEPT.md) und im [UI-Konzept](UI_KONZEPT.md) beschrieben.

## Linux-Build

Benötigt werden ein C++20-Compiler, CMake, Ninja, Poppler `pdftotext` und Qt 6 mit Core, Gui, Widgets, PrintSupport und Test.

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/ricksheets
```

## Windows-Build

Dieselbe Codebasis wird mit Qt 6 und einem C++20-Compiler gebaut. Empfohlen sind Qt 6 für MSVC 2022, CMake und Ninja:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
build\ricksheets.exe
```

Die spätere Standalone-Ausgabe wird mit `windeployqt` erzeugt. Der Windows-Build muss unter Windows gebaut und getestet werden.
Für den PDF-Import wird `pdftotext` aus Poppler mit dem fertigen Paket ausgeliefert.

## AppImage-Build

Ein neues AppImage wird aus einem bereits geprüften Seed-AppImage aufgebaut.
Das Skript entpackt die Basis in ein frisches temporäres Verzeichnis, setzt
ausschließlich das aktuelle Release-Binary ein, prüft Version und Qt 6 und
versiegelt das Ergebnis mit einem lokalen `appimagetool`:

```bash
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
ctest --test-dir build-release --output-on-failure
APPIMAGETOOL=/pfad/zu/appimagetool \
  packaging/linux/build-appimage.sh 0.1.6 \
  dist/RickSheets-0.1.5-x86_64.AppImage
```

Optional kann `APPIMAGE_RUNTIME_FILE` auf einen bereits geprüften
Runtime-Starter zeigen. Ohne diese Variable übernimmt das Skript den
Runtime-Starter bytegenau aus dem Seed-AppImage. Es lädt während des Builds
nichts aus dem Netz.

## Spaltenlogik

RickSheets verwendet zunächst ein einspaltiges A4-Layout. Nur wenn daraus mehr
Seiten entstehen, wird eine zweispaltige Variante berechnet. Sie wird nur
übernommen, wenn sie die Seitenanzahl tatsächlich reduziert.

Der Spaltenwechsel liegt bevorzugt zwischen vollständigen Songabschnitten.
Akkord- und zugehörige Textzeile werden als untrennbares Paar behandelt. Bei
bereits zweispaltigen PDFs werden die beiden Originalspalten vor der weiteren
Verarbeitung erkannt und in korrekte Lesereihenfolge gebracht.

Bei klassischen „Akkorde über Text“-Zeilen wird die Zeichenposition jedes
Akkords beim Rendern an das entsprechende Textsegment gebunden. Die Vorschau
verlässt sich damit nicht auf zufällig gleich breite Leerzeichen. Ein über dem
zweiten Vorkommen eines Wortes gesetzter Akkord bleibt auch bei proportionaler
Druckschrift und geänderter Spaltenbreite genau diesem Wort zugeordnet.

Bei mehrseitigen Songs werden die Spalten pro physischer Seite aufgebaut. Die
Lesereihenfolge ist immer:

```text
Seite 1 links → Seite 1 rechts → Seite 2 links → Seite 2 rechts
```

Die gelben Abschnittslabels verwenden explizit schwarzen Text und eine dunkle
Kontur. Dadurch bleiben Beschriftungen wie `[Chorus]` auch im Graustufen- oder
Schwarzweißdruck erkennbar.

Das RickSheets-Standardprofil orientiert sich an der Vorlage „Keeper of the
Stars“: zwei ausgewogene Spalten bei Bedarf, Abschnittstitel in vollständigen
eckigen Klammern, klare horizontale Trennlinien, Akkorde über dem Text und eine
kompakte, druckbare A4-Typografie. Fremde Formatierungen aus Word-, TextMaker-
oder PDF-Dateien werden nicht übernommen.

## Eingabesyntax

Abschnitte stehen in eckigen Klammern:

```text
[Intro]
| A | D E | A |

[Verse 1]
    Am              F
You took my heart and turned me on
```

Inline-Akkorde sind ebenfalls möglich:

```text
You took my [Am]heart and turned me [F]on
```

## Projektstruktur

```text
src/song.*          Songformat
src/chordparser.*   Importerkennung und Transposition
src/renderer.*      gemeinsames Chordsheet-Layout
src/previewwidget.* A4-Seitenansicht und PDF-Export
src/mainwindow.*    Desktop-Oberfläche
tests/              automatisierte Kerntests
docs/               vorhandene PDF-Vorlagen
examples/           synthetische Beispiel- und Seitentests
```
