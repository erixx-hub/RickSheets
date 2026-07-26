# RickSheets – Konzept für einen geführten Chordsheet-Editor

Das detaillierte Erscheinungs- und Bedienkonzept befindet sich ergänzend in [`UI_KONZEPT.md`](UI_KONZEPT.md).

## 1. Ziel

RickSheets soll aus strukturierten Songdaten druckfertige Chordsheets erzeugen, ohne dass Titel, Akkorde, Trennlinien, Spalten und Seitenumbrüche in einer Textverarbeitung von Hand gesetzt werden müssen.

Der Name **RickSheets** ist eine bewusste Verbindung aus „Rick“ und „Chordsheets“ und wird als Produktname, Fenstertitel und Export-Metadatum verwendet.

Die Anwendung läuft lokal und wird von Grund auf plattformneutral entwickelt. Linux ist die primäre Entwicklungsumgebung; Windows wird bei Architektur, Dateiformaten, Oberfläche und Abhängigkeiten von Anfang an berücksichtigt. RickSheets speichert Songs in einem offenen, lesbaren Format und exportiert reproduzierbar nach PDF.

Der Editor ist kein allgemeines Schreibprogramm. Er führt durch die musikalisch sinnvollen Bausteine eines Chordsheets und zeigt jederzeit eine echte A4-Vorschau.

Neben der manuellen Eingabe muss RickSheets weitgehend unformatierten, kopierten Chordsheet-Text übernehmen können, beispielsweise aus einer Webseite wie Ultimate Guitar. Der Inhalt wird analysiert, in Songblöcke und Akkordpositionen umgewandelt und anschließend im RickSheets-Layout ausgegeben. Das automatisch erkannte Ergebnis bleibt vollständig bearbeitbar.

## 2. Erkenntnisse aus den Vorlagen

Untersucht wurden 22 PDF-Dateien in `docs/`.

### Dominantes Layout

- A4-Hochformat
- in der Regel eine Seite
- häufig zwei gleich breite Spalten
- kompakte serifenlose Schrift
- Songtitel und Interpret als Kopf
- optionale Angaben wie BPM und Capo direkt darunter
- Songteile wie Intro, Strophe, Chorus, Bridge, Solo, Instrumental und Outro
- gelbe Hervorhebung vieler Abschnittsüberschriften
- horizontale Trennlinien zwischen Abschnitten
- Akkorde über den zugehörigen Textstellen
- Taktfolgen in einer kompakten Schreibweise mit Taktstrichen
- Spielanweisungen wie `N.C.`, „nur Gitarre“, „alle“, Wiederholungen und Ausklingen

Als visuelle Referenz für das RickSheets-Standardprofil dient insbesondere
„Keeper of the Stars“. Importierte Dokumente liefern Inhalt und Struktur, nicht
ihre ursprüngliche Formatierung.

### Relevante Varianten

- einspaltige Leadsheets mit Akkorden am rechten Zeilenrand
- klassische Akkord-über-Text-Darstellung in zwei Spalten
- parallele Tonarten in getrennten Akkordspalten
- Medleys aus mehreren Songs
- sehr volle Blätter mit automatischer Schriftverkleinerung
- ausnahmsweise zwei Seiten
- Chorus nur als Verweis statt vollständig wiederholt

Diese Varianten sollten nicht als frei formatierte Dokumente modelliert werden. Sie sind besser als wenige auswählbare Layoutprofile und strukturierte Songblöcke abbildbar.

## 3. Empfohlener Bedienablauf

### Schritt 1: Eingabeweg wählen

Beim Start eines neuen Songs stehen zwei Wege zur Auswahl:

1. **Smart Import** – ein vorhandenes Chordsheet als Text einfügen
2. **PDF importieren** – ein vorhandenes textbasiertes PDF in ein bearbeitbares Songprojekt umwandeln
3. **Geführte Eingabe** – einen Song Abschnitt für Abschnitt neu erfassen

Der Smart Import ist der bevorzugte schnelle Arbeitsweg, der geführte Editor bleibt für Korrekturen und neue Arrangements verfügbar.

### Schritt 2: Song anlegen

Pflichtfelder:

- Titel
- Interpret

Optionale Felder:

- BPM
- Taktart
- Originaltonart
- gespielte Tonart
- Capo
- Stimmung
- freie Kopfnotiz

Beim Smart Import versucht RickSheets Titel, Interpret, Tonart, Capo, Taktart und Tempo aus dem eingefügten Inhalt vorzubelegen. Fehlende oder unsichere Werte werden zur Bestätigung gezeigt.

### Schritt 3: Arrangement aus Bausteinen erstellen

Der Song besteht aus einer sortierbaren Liste von Blöcken:

- Intro
- Strophe
- Pre-Chorus
- Chorus
- Bridge
- Solo
- Instrumental
- Outro
- freie Anweisung
- Seiten-/Spaltenumbruch
- Medley-Wechsel
- Wiederholungsverweis

Jeder Block kann dupliziert, verschoben, ausgeblendet oder als Kurzverweis ausgegeben werden, zum Beispiel „Chorus 2x“.

### Schritt 4: Text und Akkorde eingeben

Für Liedtext eignet sich eine Inline-Schreibweise:

```text
You took my [Am]heart and turned me on
And now the [F]danger sign is on
```

In der Vorschau erscheinen die Akkorde automatisch über den passenden Silben. Dadurch bleiben Akkord und Text auch bei Schrift- oder Layoutänderungen miteinander verbunden.

Zusätzlich gibt es eine visuelle Eingabe:

1. Textzeile eingeben.
2. Cursor vor die gewünschte Silbe setzen.
3. Akkord über Auswahlfeld oder Tastatur eingeben.
4. Die Vorschau zeigt sofort die gesetzte Position.

Für Instrumentalteile gibt es einen Takt-Editor:

```text
| A | D E | A | D E |
```

Wiederholungen, Klammern, `N.C.`, Pausen und Freitext bleiben möglich.

### Schritt 5: Layout wählen

Vorgesehene Profile:

1. **RickSheets Standard**
   Zwei Spalten, gelbe Abschnittslabels, Trennlinien, Akkorde über Text.

2. **Kompakt einspaltig**
   Liedtext links, Akkordfolge am rechten Rand; geeignet für einfache Songs.

3. **Zwei Tonarten**
   Text mit zwei parallelen Akkordspalten, wie in „Stand By Me“.

4. **Medley**
   mehrere Songköpfe bzw. klar markierte Songwechsel innerhalb eines Blatts.

Layoutoptionen:

- eine oder zwei Spalten
- Abschnittsfarbe
- Schriftgröße und Zeilenabstand
- Akkordfarbe
- Trennlinien ein/aus
- Kopfzeile/Logo ein/aus
- Seitenzahl ein/aus
- Abschnitt in neuer Spalte beginnen

Die Anwendung sollte sinnvolle Standardwerte liefern und Detailoptionen in einem aufklappbaren Bereich verstecken.

### Schritt 6: A4-Seitenansicht prüfen und exportieren

Die rechte Bildschirmhälfte zeigt dauerhaft eine seitengetreue A4-Vorschau. Sie ist die verbindliche Darstellung für den späteren PDF-Export und zeigt:

- tatsächliche A4-Seiten mit sichtbaren Seitenrändern
- eine oder zwei Spalten entsprechend dem Layout
- alle automatischen und manuellen Seiten- und Spaltenumbrüche
- mehrere Seiten untereinander, sobald der Inhalt nicht auf eine Seite passt
- eine gut sichtbare Seitenzahl-Anzeige wie „1 Seite“ oder „2 Seiten“
- Zoomstufen „Ganze Seite“, „Seitenbreite“ und freie Vergrößerung

Hinweise erscheinen bei:

- Überlauf auf eine zweite Seite
- getrenntem Akkord und zugehöriger Textzeile
- sehr kleiner automatisch gewählter Schrift
- leerer BPM-/Capo-Angabe, wenn das Feld sichtbar wäre
- unbekannten Akkorden
- überlangen Zeilen

Das Ziel ist standardmäßig ein gut lesbares einseitiges Chordsheet. Wenn der Song auf zwei Seiten fällt, bietet RickSheets kontrollierte Optimierungen an:

1. Abstände zwischen Abschnitten geringfügig reduzieren
2. Zeilenabstand reduzieren
3. Spaltenverteilung verbessern
4. Schriftgröße innerhalb einer festgelegten lesbaren Untergrenze verkleinern
5. Chorus-Wiederholungen optional durch einen Verweis ersetzen

Keine Optimierung wird unbemerkt angewendet. Die Seitenansicht aktualisiert sich sofort und zeigt, welche Einstellung geändert wurde. Wenn eine gut lesbare Einseitenfassung nicht möglich ist, bleibt das Dokument bewusst zweiseitig.

Export:

- PDF als verpflichtend unterstütztes Hauptformat
- Drucken
- Songprojekt als JSON oder YAML
- optional ChordPro als Austauschformat

Der PDF-Export verwendet das gewählte RickSheets-Layout, eingebettete Schriften und das A4-Seitenformat. Damit soll das Ergebnis auf anderen Rechnern und beim Drucken genauso aussehen wie in der Vorschau.

## 4. Smart Import aus kopiertem Text

Der Importdialog besteht aus drei Phasen:

### 1. Einfügen

Ein großes Textfeld nimmt den kopierten Inhalt möglichst unverändert entgegen. Zusätzliche Webseitenbestandteile wie Menüs, Akkordlisten, Bewertungen oder Leerzeilen dürfen enthalten sein.

### 2. Erkennen und prüfen

RickSheets erkennt nach Möglichkeit:

- Titel und Interpret
- Capo, Tonart und Tempo
- Abschnittsüberschriften wie Verse, Chorus, Bridge, Solo oder Outro
- reine Akkordzeilen über Textzeilen
- Inline-Akkorde in eckigen Klammern
- Taktfolgen mit `|` oder Wiederholungszeichen
- Spielanweisungen und freie Notizen
- Wiederholungen und Chorus-Verweise
- offensichtlichen Webseiten-Ballast

Das Ergebnis wird vor der Übernahme als zweigeteilte Ansicht gezeigt:

```text
┌──────────────────────────┬──────────────────────────┐
│ Eingefügter Originaltext │ Erkannte Songstruktur   │
│                          │                          │
│ Zeilen markieren         │ Metadaten               │
│ oder ignorieren          │ Verse 1                 │
│                          │ Chorus                  │
│                          │ Solo                    │
└──────────────────────────┴──────────────────────────┘
```

Unsichere Erkennungen werden farblich markiert. Der Nutzer kann:

- eine Zeile als Akkordzeile, Text, Überschrift oder Notiz einstufen
- einen erkannten Abschnitt umbenennen
- unerwünschte Zeilen ignorieren
- falsch gekoppelte Akkord- und Textzeilen neu verbinden
- Titel, Interpret und musikalische Angaben korrigieren

### 3. Übernehmen

Nach der Bestätigung wird der Inhalt in das normale strukturierte Songmodell übernommen. Ab diesem Punkt gibt es keinen Unterschied mehr zwischen einem importierten und einem manuell erfassten Song.

### Technische Importregeln

Bei klassischen Akkordzeilen wird die horizontale Akkordposition auf eine Textposition abgebildet:

```text
       Am                 F
You took my heart and turned me on
```

wird intern sinngemäß zu:

```text
You took my [Am]heart and turned me [F]on
```

Tabs werden vor der Analyse kontrolliert in Spaltenpositionen umgerechnet. Eine Monospace-Ansicht bewahrt während des Imports die ursprünglichen Abstände.

Die Erkennung arbeitet zunächst regelbasiert und nachvollziehbar. Sie benötigt keine Internetverbindung und sendet den eingefügten Songtext nicht an einen externen Dienst. Fehlerhafte Erkennung muss immer manuell korrigierbar sein.

### PDF-Import

Ein vorhandenes PDF wird nicht direkt wie in einer Textverarbeitung verändert. RickSheets extrahiert den Inhalt und rekonstruiert daraus ein strukturiertes Songprojekt, das anschließend normal bearbeitet, transponiert und erneut als PDF exportiert werden kann.

Der Ablauf:

1. PDF-Datei auswählen.
2. Text, Positionen, Seiten und Spalten auslesen.
3. Überschriften, Metadaten, Akkordzeilen, Textzeilen, Abschnitte und Trennlinien erkennen.
4. Rekonstruiertes Ergebnis neben einer Ansicht des Original-PDFs anzeigen.
5. Unsichere Stellen korrigieren und den Song übernehmen.

Für die vorhandenen Vorlagen ist dieser Ansatz gut geeignet, weil die untersuchten Dateien echten, extrahierbaren Text enthalten. Die Unterschiede zwischen Word- und TextMaker-PDFs sowie die verschiedenen Spaltenlayouts erfordern trotzdem eine Prüfansicht.

Unterstützungsstufen:

- **Textbasierte PDFs:** regulär unterstützt
- **PDFs mit ungewöhnlicher Zeichenreihenfolge oder eingebetteten Spezialschriften:** bestmöglicher Import mit Warnungen
- **gescannte PDFs ohne Textschicht:** später optional über lokale OCR
- **handschriftliche Chordsheets:** nicht Bestandteil des ersten PDF-Imports

Ziel ist die inhaltliche Rekonstruktion im RickSheets-Standardlayout, nicht die pixelgenaue Wiederherstellung der ursprünglichen Word- oder TextMaker-Datei. Das Original-PDF bleibt unverändert.

## 5. UI-Aufbau

Empfohlen wird eine Desktop-Oberfläche mit drei Bereichen:

```text
┌───────────────┬──────────────────────────┬──────────────────────────┐
│ Songbibliothek│ Arrangement / Eingabe    │ A4-Livevorschau          │
│               │                          │                          │
│ Suche         │ Metadaten                │ Seite 1                  │
│ Titel         │ Intro                    │                          │
│ Sets/Tags     │ Strophe 1                │ klickbarer Block         │
│               │ Chorus                   │ Seite 2 (bei Überlauf)   │
│ + Neuer Song  │ + Abschnitt              │ 2 Seiten · Zoom          │
└───────────────┴──────────────────────────┴──────────────────────────┘
```

Auf kleinen Bildschirmen kann die Vorschau als eigener Tab erscheinen.

Ein Klick auf einen Abschnitt im Editor hebt ihn in der A4-Vorschau hervor. Umgekehrt springt ein Klick in der Vorschau zum zugehörigen bearbeitbaren Abschnitt.

Wichtige Komfortfunktionen:

- Autosave
- Undo/Redo
- ständig erreichbare Aktion „Tonart ändern“
- Drag-and-drop für Abschnitte
- Tastaturkürzel zum Einfügen eines Akkords
- Akkordvorschläge aus der gewählten Tonart
- Duplizieren von Strophen-/Chorus-Strukturen
- Volltextsuche in der Songbibliothek
- Tags und Setlisten
- Warnung vor ungespeicherten Änderungen

## 6. Transponieren

Transponieren sollte eine Kernfunktion sein:

- Transposition des vollständigen Songs mit einer Aktion
- Zieltonart oder Anzahl Halbtöne auswählen
- Akkorde im ganzen Song oder nur in ausgewählten Blöcken ändern
- deutsche/internationale Notation wählbar (`H`/`B`)
- Vorzeichenpräferenz wählbar (`F#` oder `Gb`)
- Slash-Chords, Erweiterungen und verminderte Akkorde erhalten
- Capo-Vorschlag optional
- Originaltonart bleibt in den Metadaten erhalten

Freitext darf beim Transponieren nie blind ersetzt werden. Nur als Akkord gespeicherte Tokens werden geändert.

Vor dem Anwenden zeigt ein Dialog die bisherige und die neue Tonart sowie einige Beispielumwandlungen. Die Änderung ist über Undo vollständig rückgängig zu machen.

## 7. Datenmodell

Ein Songprojekt sollte inhaltliche Daten und Layout strikt trennen.

```yaml
version: 1
title: Friends in Low Places
artist: Garth Brooks
tempo_bpm: 115
key: A
capo: null
layout: ricksheets-standard
sections:
  - type: intro
    bars: ["A", "Bbdim7", "Bm", "E"]
  - type: verse
    label: Verse 1
    lines:
      - text: "Blame it all on my roots, I showed up in boots"
        chords:
          - { at: 0, chord: A }
          - { at: 29, chord: Bbdim7 }
  - type: chorus
    label: Chorus
    lines: []
```

Intern sollten Akkordpositionen an Textstellen gebunden werden, nicht durch Leerzeichen simuliert werden. Das verhindert das Verrutschen bei Änderungen.

## 8. Technische Empfehlung

### Desktop-Anwendung

- **Python 3**
- **PySide6 / Qt** für eine gemeinsame Oberfläche unter Linux und Windows
- **HTML/CSS als Layoutvorlage**
- **Qt QTextDocument** für A4-Vorschau und PDF-Export mit derselben Rendering-Engine
- **PyMuPDF** für den Import textbasierter PDFs
- **SQLite** für Bibliothek, Suche, Tags und Setlisten
- Songdateien zusätzlich als versioniertes JSON für Portabilität und Versionsverwaltung

Warum diese Kombination:

- gemeinsame Codebasis für Linux und Windows
- schnelle Entwicklung einer echten Desktop-Anwendung
- HTML/CSS eignet sich sehr gut für A4, Spalten, Farben und Drucklayout
- Vorschau und PDF-Export verwenden dieselbe HTML/CSS-Quelle und Rendering-Engine
- keine Abhängigkeit von Word oder TextMaker

Die Vorschau muss mit derselben Rendering-Logik wie der PDF-Export arbeiten. Andernfalls entstehen schwer nachvollziehbare Unterschiede zwischen Bildschirm und Ausdruck.

### Plattformneutrale Architektur

Alle fachlichen Funktionen bleiben betriebssystemunabhängig:

- Songmodell und Dateiformat
- Import von Text und PDF
- Akkordparser und Transposition
- Layout-Engine
- A4-Vorschau und PDF-Export
- Bibliothek und Suche

Betriebssystemspezifische Aufgaben werden in einer kleinen eigenen Schicht gekapselt:

- Speicherorte für Einstellungen und Bibliothek über Qt `QStandardPaths`
- native Dialoge zum Öffnen, Speichern und Drucken
- Verknüpfungen und Dateityp-Zuordnungen
- Ermittlung installierter Schriften und Drucker
- Öffnen von PDF-Dateien im jeweiligen Standardprogramm

Für ein identisches Druckbild werden die benötigten Schriften mit RickSheets ausgeliefert und in der Layout-Engine explizit verwendet. Das Songformat speichert keine absoluten Linux- oder Windows-Dateipfade.

### Entwicklung und Tests

Die Anwendung wird im Python-Virtualenv entwickelt. Automatisierte Tests prüfen auf beiden Plattformen:

- Laden und Speichern identischer Songdateien
- Akkorderkennung und Transposition
- Import repräsentativer Text- und PDF-Beispiele
- Seitenanzahl und Umbruchregeln
- PDF-Erzeugung
- Datenmigration bei neuen Formatversionen

Visuelle Referenztests vergleichen gerenderte A4-Seiten mit freigegebenen Beispielbildern. Kleine technische Rendering-Abweichungen zwischen Betriebssystemen dürfen das Layout nicht verschieben.

### Paketierung

Die Paketierung erfolgt erst aus einem stabilen gemeinsamen Quellstand:

- **Linux:** AppImage; Flatpak bleibt eine spätere Alternative
- **Windows:** eigenständige RickSheets-EXE, optional zusätzlich ein Installer

Qt und das für textbasierte PDF-Importe verwendete Poppler-Werkzeug werden vom jeweiligen Paketierungsprozess gebündelt. Ein Windows-Paket wird unter Windows gebaut und getestet, ein AppImage unter Linux. Beide Pakete verwenden dieselbe RickSheets-Version und dasselbe Songdateiformat.

## 9. Layout-Engine

Die Engine erhält strukturierte Songdaten und ein Layoutprofil und erzeugt daraus HTML/CSS.

Wichtige Regeln:

- ein Abschnitt bleibt möglichst vollständig zusammen
- Überschrift und erste Zeile werden nie getrennt
- Akkordzeile und Textzeile bilden eine untrennbare Einheit
- automatische Verteilung auf zwei Spalten
- explizite Spaltenumbrüche haben Vorrang
- falls nötig: erst Abstände reduzieren, dann geringfügig Schrift verkleinern, zuletzt zweite Seite erzeugen
- automatische Anpassungen werden sichtbar gemeldet

Die Anwendung sollte keine Garantie „immer genau eine Seite“ erzwingen. Lesbarkeit ist wichtiger als eine unbemerkt extrem kleine Schrift.

## 10. MVP

Die erste sinnvoll nutzbare Version umfasst:

1. Song anlegen, öffnen, speichern und duplizieren
2. Smart Import aus kopiertem, weitgehend unformatiertem Chordsheet-Text
3. Import textbasierter PDFs
4. Prüfansicht mit Original-PDF und manueller Korrektur des Importergebnisses
5. Titel, Interpret, BPM, Tonart und Capo
6. sortierbare Songabschnitte
7. Text mit positionsgebundenen Akkorden
8. Taktfolgen und Spielanweisungen
9. Standardlayout mit einer oder zwei Spalten
10. verbindliche A4-Livevorschau mit Seitenanzahl und sichtbaren Umbrüchen
11. verbindlicher PDF-Export
12. Transponieren
13. einfache Bibliothek mit Suche

Nicht Teil des ersten MVP:

- OCR für reine Scan-PDFs ohne Textschicht
- gemeinsame Cloud-Bearbeitung
- Audioanalyse
- Notensatz oder Tabulatur
- mobile Apps
- freie pixelgenaue Gestaltung

## 11. Spätere Ausbaustufen

- Import von ChordPro
- direkter Import aus Dateien und Webseiten, sofern technisch und rechtlich sinnvoll
- Setlisten und Export eines kompletten Setlisten-PDFs
- verschiedene Instrumenten-/Sängeransichten desselben Songs
- individuelle Transposition pro Bandmitglied
- Akkorddiagramme
- automatische Seitenwendepunkte für Tablet-Nutzung
- Archivversionen und Änderungsverlauf
- eigene Layoutvorlagen über einen Template-Editor

## 12. Umsetzung in Etappen

### Etappe 1: Vertikaler Prototyp

Einen vorhandenen Song als Rohtext importieren, in eine strukturierte Beispieldatei umwandeln und daraus das Standardlayout als PDF erzeugen. Damit werden Importlogik, Typografie, Spaltenlogik und Akkordpositionierung geklärt, bevor die komplette Oberfläche entsteht.

### Etappe 2: Editor

Import-Prüfansicht, Metadatenformular, Blockliste, Inline-Akkordeditor und Livevorschau umsetzen.

### Etappe 3: Bibliothek und Transposition

SQLite-Bibliothek, Suche, Tags, Akkordparser und sichere Transposition ergänzen.

### Etappe 4: Paketierung und Praxisprüfung

AppImage erstellen und mehrere der vorhandenen Vorlagen nachbauen. Als Abnahmekriterien dienen Lesbarkeit, Seitentreue und die Zeit, die für das Erfassen eines neuen Songs benötigt wird.

## 13. Konkrete Abnahmekriterien

- Ein typischer Song kann in höchstens zehn Minuten erfasst werden.
- Ein übliches kopiertes Chordsheet wird automatisch in bearbeitbare Abschnitte, Akkord- und Textzeilen zerlegt.
- Die vorhandenen textbasierten PDF-Vorlagen lassen sich importieren und als strukturierte RickSheets-Songs weiterbearbeiten.
- Beim PDF-Import werden Original und rekonstruiertes Ergebnis zur Kontrolle nebeneinander angezeigt.
- Unsichere Importergebnisse sind sichtbar und vor der Übernahme korrigierbar.
- Akkorde bleiben bei Textkorrekturen an der beabsichtigten Stelle.
- PDF und Vorschau stimmen visuell überein.
- Jeder gespeicherte Song lässt sich als druckfähige A4-PDF exportieren.
- Die aktuelle Anzahl der A4-Seiten ist während der Bearbeitung jederzeit sichtbar.
- Der Wechsel von einer auf zwei Seiten wird unmittelbar angezeigt.
- Automatische Verdichtung unterschreitet niemals die festgelegte Mindestschriftgröße.
- Das Standardlayout bildet die Mehrheit der vorhandenen Vorlagen ohne manuelle Positionierung ab.
- Ein Song lässt sich mit einem Schritt vollständig transponieren.
- Eine zweite Seite wird bewusst angezeigt und nicht durch unlesbare Skalierung vermieden.
- Gespeicherte Songs bleiben ohne die Anwendung in einem offenen Format lesbar.
