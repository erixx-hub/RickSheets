# RickSheets roadmap

This roadmap describes direction rather than fixed release dates. RickSheets
remains focused on importing, correcting and printing chord sheets; unrelated
library or performance-management features are not a current priority.

## Completed: German and English interface

RickSheets 0.3.0 introduced:

- Qt Linguist support in the CMake build;
- a complete English translation catalogue;
- automatic German/English selection based on the system language;
- an explicit System, German or English preference;
- automated UI coverage and visual checks in both languages;
- strict separation between translated interface text and song content.

The `.ricksheet` file format will remain language-neutral. Section names in a
song belong to user content and will not change merely because the interface
language changes.

## Publication readiness

- Add AppStream metadata, release notes and store screenshots.
- Build RickSheets reproducibly as a sandboxed Flatpak.
- Validate desktop, icon, metainfo and Flatpak manifests.
- Test file import, local library storage and PDF export through the sandbox.
- Submit the package for review on Flathub.

## Editing and print control

- Add manual page and column break controls.
- Offer carefully bounded typography and spacing options.
- Make uncertain import detections more visible at the exact affected lines.
- Improve keyboard navigation, undo and redo across visual editing workflows.

## Import quality

- Extend source-clutter removal without deleting legitimate song content.
- Improve metadata recognition for international input.
- Investigate OCR as an optional path for image-only PDF files.
- Maintain private regression fixtures locally and public synthetic fixtures
  in the repository.

## Later

- Produce and test a native Windows package.
- Evaluate additional native Linux packages after the Flatpak release.
- Consider library enhancements only where they support the core chord-sheet
  workflow directly.
