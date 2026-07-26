# RickSheets roadmap

This roadmap describes direction rather than fixed release dates. RickSheets
remains focused on importing, correcting and printing chord sheets; unrelated
library or performance-management features are not a current priority.

## Next: internationalization

Prepare the application for a German and English user interface:

1. Add Qt Linguist support to the CMake build.
2. Audit all visible strings and translation contexts.
3. Keep German as the existing complete source-language experience.
4. Add and review an English translation catalogue.
5. Add a language preference with a system-language default.
6. Test first-start, main-window and dialog flows in both languages.
7. Ensure import terminology distinguishes language-dependent labels from
   chord-sheet content, which must never be translated automatically.

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
