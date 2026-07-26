# RickSheets – UI-Konzept

## Gestaltungsziel

RickSheets soll wie ein fokussiertes musikalisches Arbeitswerkzeug wirken: übersichtlich, schnell und druckorientiert. Die Oberfläche übernimmt die klare Schwarz-Weiß-Typografie der vorhandenen Chordsheets und verwendet Gelb als gezielte Akzentfarbe für aktive Songteile, Akkorde und Warnungen.

Die Anwendung soll nicht wie eine vollständige Textverarbeitung aussehen. Formatierungsleisten mit Schriftart, frei positionierten Textfeldern und Seitenlinealen werden vermieden. Stattdessen bearbeitet der Nutzer musikalische Inhalte; RickSheets übernimmt das Layout.

## Hauptfenster

Das Hauptfenster besteht aus drei veränderbaren Bereichen:

```text
┌──────────────────────────────────────────────────────────────────────────────┐
│ RickSheets  Datei  Bearbeiten  Song  Ansicht  Hilfe      1 Seite  Export PDF│
├───────────────┬──────────────────────────────┬───────────────────────────────┤
│ BIBLIOTHEK    │ FRIENDS IN LOW PLACES        │ A4-VORSCHAU                  │
│ [Suchen ...]  │ Garth Brooks                 │ ┌─────────────────────────┐   │
│               │ 115 BPM · A · ohne Capo      │ │ Titel                   │   │
│ ★ Favoriten   │ [Transponieren] [Layout]     │ │                         │   │
│ Setlisten     ├──────────────────────────────┤ │ Verse 1     Chorus      │   │
│ Alle Songs    │ ▾ Intro                      │ │                         │   │
│               │   | A | Bbdim7 | Bm | E |   │ │ Verse 2     Verse 3     │   │
│ Friends ...   │                              │ │                         │   │
│ Hound Dog     │ ▾ Verse 1                    │ └─────────────────────────┘   │
│ Red Light     │   [A]Blame it all ...        │                               │
│               │   And ruined your ...        │ Ganze Seite · 100 % · Seite 1│
│ + Neuer Song  │                              │                               │
│ Importieren ▾ │ + Abschnitt                  │                               │
└───────────────┴──────────────────────────────┴───────────────────────────────┘
```

### Linker Bereich: Bibliothek

- Suche über Titel, Interpret und Tags
- Filter für Favoriten, Setlisten und zuletzt bearbeitete Songs
- kompakte Songliste
- primäre Aktionen „Neuer Song“ und „Importieren“
- Importmenü mit „Text einfügen“ und „PDF auswählen“
- ein- und ausblendbar, damit auf kleinen Bildschirmen mehr Platz entsteht

### Mittlerer Bereich: Editor

- Songkopf mit Titel, Interpret und musikalischen Metadaten
- gut sichtbare Aktion „Transponieren“
- Abschnitte als aufklappbare Karten
- Drag-and-drop-Griff zum Sortieren
- Abschnittstyp und eigener Anzeigename
- Text- und Akkordeingabe ohne sichtbare technische Markup-Syntax, sofern gewünscht
- Kontextmenü für Duplizieren, Wiederholen, Verschieben und Löschen
- Schaltfläche „+ Abschnitt“ am Ende

### Rechter Bereich: A4-Vorschau

- echte weiße A4-Seiten auf neutralgrauem Hintergrund
- sichtbare Seitenränder und Seitenschatten
- Seiten untereinander, nicht als endlose Textfläche
- feste Seitenanzeige in der Kopf- und Fußleiste
- Ansichten „Ganze Seite“, „Seitenbreite“ und frei wählbarer Zoom
- anklickbare Vorschau: Auswahl springt zum entsprechenden Editorblock
- aktive Editorsektion wird in der Vorschau kurz gelb umrandet
- roter Überlaufhinweis, wenn Inhalt außerhalb des druckbaren Bereichs liegt

Die Trennlinien zwischen den drei Bereichen lassen sich verschieben. Die gewählten Breiten werden gespeichert.

## Kopfzeile

Die Kopfzeile zeigt nur globale Aktionen:

- Rückgängig / Wiederholen
- Speichern
- aktuelle Seitenanzahl
- Warnstatus
- hervorgehobene Aktion „PDF exportieren“

„PDF exportieren“ bleibt rechts oben jederzeit erreichbar. Vor dem Export wird keine zusätzliche Layoutkopie erzeugt; exportiert wird exakt der Stand der A4-Vorschau.

## Neuer Song und Import

Nach „Neuer Song“ erscheint ein einfacher Startdialog:

```text
┌──────────────────────────────────────────┐
│ Was möchtest du tun?                    │
│                                          │
│ [ Chordsheet-Text einfügen ]             │
│   Akkorde und Songteile automatisch      │
│   erkennen                               │
│                                          │
│ [ PDF importieren ]                      │
│   Vorhandenes Chordsheet rekonstruieren  │
│                                          │
│ [ Leeren Song anlegen ]                  │
└──────────────────────────────────────────┘
```

Die drei Wege führen in dasselbe Songmodell und danach in denselben Editor.

## Smart-Import-Ansicht

Der Import verwendet einen geführten Drei-Schritt-Dialog:

1. Inhalt einfügen oder PDF auswählen
2. Erkennung kontrollieren
3. Metadaten und Layout bestätigen

In der Kontrolle stehen Original und erkanntes Ergebnis nebeneinander:

```text
┌────────────────────────────┬─────────────────────────────┐
│ ORIGINAL                   │ ERKANNT                     │
│                            │                             │
│ [Verse 1]                  │ Verse 1              sicher│
│     Am       F             │ Akkord + Text         sicher│
│ You took my heart ...      │ Akkord + Text         sicher│
│                            │                             │
│ random website text        │ Ignoriert            prüfen│
├────────────────────────────┴─────────────────────────────┤
│ Zurück                  [Unsichere Stellen: 1] [Übernehmen]│
└──────────────────────────────────────────────────────────┘
```

Erkennungsstatus:

- Grün: sicher erkannt
- Gelb: bitte prüfen
- Rot: nicht zugeordnet
- Grau: als Webseiten-Ballast ignoriert

Die Farben werden zusätzlich durch Symbole und Text vermittelt, damit die Bedienung nicht allein von Farbwahrnehmung abhängt.

Beim PDF-Import erscheint links die Originalseite und rechts die rekonstruierte RickSheets-Seite. Ein Klick auf eine Stelle markiert die jeweils zugehörigen Inhalte.

## Songabschnitt im Editor

Ein Abschnitt ist eine kompakte Karte:

```text
┌─ ⠿  Verse 1 ─────────────────────────────────── ⋯ ┐
│        Am                         F               │
│ You took my heart and turned me on               │
│                                                  │
│ And now the danger sign is on                    │
│                                                  │
│ + Zeile                         + Spielanweisung │
└──────────────────────────────────────────────────┘
```

### Akkorde setzen

Es gibt zwei gleichwertige Eingabearten:

- Akkord direkt als `[Am]` im Text schreiben
- Cursor vor eine Silbe setzen und über `Strg+K` einen Akkord einfügen

Im normalen visuellen Modus zeigt der Editor den Akkord über dem Text. Ein optionaler Rohtextmodus zeigt die Inline-Schreibweise und eignet sich besonders für schnelles Einfügen oder umfangreiche Korrekturen.

Beim Schreiben schlägt RickSheets passende Akkorde vor, ohne Eingaben automatisch zu verändern.

## Transponierdialog

```text
┌─ Song transponieren ──────────────────────────┐
│ Aktuelle Tonart       A                       │
│ Zieltonart            [ C ▾ ]                 │
│                                                │
│ Schreibweise          ○ B international       │
│                      ● H deutsch              │
│ Vorzeichen            ● automatisch ○ ♯ ○ ♭  │
│                                                │
│ Vorschau: A → C · Bm → Dm · E7 → G7           │
│                                                │
│                    Abbrechen  [Transponieren]  │
└───────────────────────────────────────────────┘
```

Die Aktion betrifft standardmäßig den ganzen Song. Eine erweiterte Option erlaubt ausgewählte Abschnitte. Nach dem Anwenden kann die Änderung vollständig rückgängig gemacht werden.

## Seitenstatus und Einseiten-Assistent

Die Seitenanzahl ist immer sichtbar:

- Grün: „1 Seite“
- Gelb: „2 Seiten“
- Rot: Überlauf oder ungültiger Umbruch

Gelb bedeutet nicht automatisch einen Fehler. Es signalisiert nur, dass das Ideal einer Einseitenfassung nicht erreicht ist.

Bei zwei Seiten bietet ein Klick auf den Status einen Assistenten:

```text
Auf eine Seite optimieren

[ ] Abschnittsabstände leicht reduzieren      reicht nicht
[ ] Zeilenabstand leicht reduzieren           Vorschau: 1 Seite
[ ] Schrift von 10,0 auf 9,5 pt reduzieren
[ ] wiederholten Chorus als Verweis ausgeben

Mindestschriftgröße: 9 pt
```

Jede Option aktualisiert die Vorschau unmittelbar. Änderungen werden erst nach Bestätigung dauerhaft übernommen.

## Layout-Seitenleiste

„Layout“ öffnet rechts neben dem Editor eine schmale Einstellungsleiste:

- Layoutprofil
- ein oder zwei Spalten
- Schriftgröße
- Zeilen- und Abschnittsabstand
- Abschnittsmarkierung
- Trennlinien
- Kopfzeile bzw. Logo
- feste Spalten- und Seitenumbrüche

Die Einstellungen zeigen nur Parameter, die das gewählte Profil erlaubt. Eine Aktion „Standard wiederherstellen“ verhindert, dass ein Song dauerhaft in einen schwer nachvollziehbaren Zustand gerät.

## Visuelles System

### Farben

- Hintergrund der Anwendung: warmes Hellgrau
- Papier: Weiß
- Haupttext: nahezu Schwarz
- RickSheets-Gelb: Abschnittslabels, Auswahl und primäre Akzente
- Blau: neutrale interaktive Elemente
- Grün: valide Einseitenansicht und sichere Imports
- Rot: echte Fehler und Überlauf

Gelb wird sparsam eingesetzt, damit es mit den vorhandenen Chordsheets verbunden bleibt und nicht die gesamte Oberfläche dominiert.

### Typografie

- Oberfläche: mitgelieferte, gut lesbare Sans-Serif-Schrift
- Chordsheet: eigenes druckoptimiertes Schriftprofil
- Akkorde: halbfett
- Abschnittsnamen: fett und kompakt
- Monospace-Schrift nur für Rohtext und Importkontrolle

Die für Chordsheets verwendeten Schriften werden mit der Anwendung ausgeliefert, damit Linux- und Windows-PDFs gleich umbrechen.

### Symbole

Symbole werden nur für gelernte Standardaktionen verwendet und fast immer mit Text oder Tooltip kombiniert. Musikalische Sonderfunktionen wie Transponieren erhalten eine Textbeschriftung statt eines schwer verständlichen Icons.

## Fenstergrößen

- optimale Arbeitsbreite: ab 1440 Pixel
- sinnvoll nutzbar: ab 1100 Pixel
- bei wenig Breite wird die Bibliothek eingeklappt
- unterhalb einer definierten Breite wechseln Editor und Vorschau in Tabs
- minimale Fenstergröße verhindert unbedienbar schmale Bereiche

Das Layout reagiert identisch unter Linux und Windows. Native Dateidialoge dürfen plattformspezifisch aussehen.

## Tastaturbedienung

- `Strg+N`: neuer Song
- `Strg+O`: Song oder Projekt öffnen
- `Strg+S`: speichern
- `Strg+Z` / `Strg+Umschalt+Z`: rückgängig / wiederholen
- `Strg+K`: Akkord an der Cursorposition einfügen
- `Strg+Umschalt+T`: transponieren
- `Strg+P`: PDF-/Druckansicht
- `Strg+E`: PDF exportieren
- `Alt+1`: Bibliothek
- `Alt+2`: Editor
- `Alt+3`: Vorschau

Alle Hauptfunktionen bleiben auch mit Maus erreichbar.

## Barrierefreiheit

- skalierbare UI-Schrift
- vollständige Tastaturbedienung
- sichtbarer Fokus
- ausreichende Kontraste
- Status nie ausschließlich über Farbe
- zugängliche Beschriftungen für Steuerelemente
- Hell- und Dunkelmodus für die Anwendung

Die A4-Seite bleibt im Dunkelmodus weiß, weil sie das spätere Druckergebnis darstellt.

## Erste Prototyp-Ansicht

Der erste UI-Prototyp sollte bewusst nur diesen Kernweg abbilden:

1. Rohtext einfügen
2. erkanntes Chordsheet übernehmen
3. Abschnitte und Akkorde bearbeiten
4. A4-Seitenanzahl sehen
5. gesamten Song transponieren
6. PDF exportieren

Bibliotheksdetails, Setlisten und ein umfangreicher Template-Editor folgen erst, nachdem dieser Ablauf in der Praxis schnell und verständlich funktioniert.

## UI-Abnahmekriterien

- Ein neuer Nutzer erkennt ohne Anleitung die drei Startwege.
- A4-Seitenanzahl und PDF-Export sind jederzeit auffindbar.
- Zwischen Einfügen eines Rohtexts und bearbeitbarer A4-Vorschau liegen höchstens drei Dialogschritte.
- Jeder Inhalt in der Vorschau lässt sich seinem Editorabschnitt zuordnen.
- Importunsicherheiten können ohne Bearbeitung technischer Daten korrigiert werden.
- Transposition des vollständigen Songs benötigt höchstens zwei bewusste Bestätigungen.
- Die Oberfläche funktioniert mit Maus und Tastatur unter Linux und Windows.
