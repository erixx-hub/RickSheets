# RickSheets

RickSheets is a desktop editor for turning pasted chord sheets and text-based
PDF files into clean, consistent and printable A4 chord sheets.

The application focuses on one workflow: import an imperfect source, review
the detected song, correct lyrics and chords, and export a dependable PDF.

> RickSheets 0.3.1 is an early release. The interface is available in German
> and English.

## Features

- Import unformatted chord-sheet text.
- Import text-based PDFs using Poppler.
- Remove common website clutter and repeated headings during import.
- Detect title, artist, key, BPM, capo, sections and chord lines.
- Review the original PDF page beside the editable import result.
- Edit the raw arrangement or use visual section and chord editors.
- Keep chords aligned with their lyric positions in proportional print fonts.
- Transpose complete songs using international or German chord notation.
- Preview the final A4 pages while editing.
- Switch automatically to a two-column layout when it reduces the page count.
- Save songs in the open JSON-based `.ricksheet` format.
- Search a local song library.
- Export PDF files using the same rendering engine as the preview.
- Use light, dark or system appearance.
- Follow the system language or switch instantly between German and English.

## Current status

The Linux AppImage and the core import/edit/export workflow are functional and
covered by automated tests. RickSheets has also been regression-tested locally
against 22 different source PDFs. Those source documents and user-created
chord sheets are intentionally not part of this public repository.

Current limitations:

- Scanned image-only PDFs require OCR, which is not implemented yet.
- Linux is the currently tested and packaged platform.
- Fine-grained manual control over page and column breaks is still planned.

## Building on Linux

Required:

- a C++20 compiler
- CMake 3.24 or newer
- Ninja
- Qt 6 with Core, Gui, Widgets, PrintSupport, Test and LinguistTools
- Poppler command-line tools (`pdftotext` and `pdftoppm`)

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/ricksheets
```

## Windows

RickSheets uses portable Qt 6 and C++20 code, but the Windows package is not
yet part of the tested release process. A Windows build requires Qt 6, CMake,
Ninja, a C++20 compiler and Poppler:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
build\ricksheets.exe
```

The final standalone package will additionally require `windeployqt` and the
Poppler command-line tools.

## Input format

Section headings use square brackets:

```text
[Intro]
| A | D E | A |

[Verse 1]
    Am             F
A simple lyric line for testing
```

Inline chords are supported as well:

```text
A simple [Am]lyric line with an [F]inline chord
```

## Layout model

Preview and PDF export share the same renderer. RickSheets starts with a
single-column A4 layout and only uses two columns when doing so actually
reduces the page count. Column breaks prefer complete song sections, and a
chord line stays attached to its corresponding lyric line.

For classic “chords above lyrics” notation, chord positions are bound to the
matching lyric segments rather than relying on monospaced spaces. This keeps
the musical alignment stable when the page width or print font changes.

## Project structure

```text
src/song.*          Open song format
src/chordparser.*   Import cleanup, recognition and transposition
src/renderer.*      Shared preview and PDF layout
src/previewwidget.* A4 page preview and PDF export
src/mainwindow.*    Desktop user interface
tests/              Automated core and UI tests
packaging/          Linux desktop integration and AppImage tooling
```

## Roadmap and contributions

The near-term priorities are finer print-layout control, Flatpak packaging and
stronger import diagnostics. See [ROADMAP.md](ROADMAP.md) for the staged plan.

Bug reports and focused pull requests are welcome. Please do not attach or
commit copyrighted lyrics, downloaded chord sheets or third-party PDFs. Use
short synthetic examples when demonstrating import or rendering issues.

## Deutsch

RickSheets ist ein Desktop-Editor, der eingefügten Chordsheet-Text und
textbasierte PDFs bereinigt, bearbeitbar macht und als einheitliches
A4-Chordsheet exportiert. Die Anwendung befindet sich in einer frühen, aber
funktionsfähigen Entwicklungsphase. Die Oberfläche ist vollständig auf Deutsch
und Englisch verfügbar.

## License

Copyright (C) 2026 Erik Heidenreich.

RickSheets is free software licensed under
[GPL-3.0-or-later](LICENSE). Modified versions are permitted under the license
terms; existing copyright and license notices must be preserved.
