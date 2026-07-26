"use client";

import { useEffect, useState } from "react";

const releaseBase =
  "https://github.com/erixx-hub/RickSheets/releases/download/v0.3.3";

const copy = {
  de: {
    navFeatures: "Funktionen",
    navDownload: "Download",
    navSource: "Quellcode",
    eyebrow: "RickSheets 0.3.3 · Linux & Windows",
    title: "Chordsheets, die auf die Seite passen.",
    intro:
      "Importiere Songs aus Text oder PDF, räume Akkorde und Abschnitte visuell auf und exportiere ein einheitliches A4-Chordsheet.",
    primary: "RickSheets herunterladen",
    secondary: "Auf GitHub ansehen",
    proof: "Lokal. Werbefrei. Open Source.",
    previewAlt: "RickSheets mit Songbibliothek, Editor und A4-Vorschau",
    featureTitle: "Vom Fundstück zum spielbaren Sheet",
    featureIntro:
      "RickSheets hält den Arbeitsablauf bewusst übersichtlich: importieren, prüfen, bearbeiten, spielen.",
    features: [
      ["01", "Importieren", "Text einfügen oder eine PDF öffnen. Navigationsleisten, Griffbilder und typische Webseitenreste werden konservativ herausgefiltert."],
      ["02", "Prüfen", "Original und erkanntes Ergebnis stehen nebeneinander. Auffällige Stellen werden markiert, bevor sie in die Bibliothek wandern."],
      ["03", "Bearbeiten", "Abschnitte ordnen, Akkorde wortgenau setzen und den Song transponieren – ohne im Leerzeichenchaos zu versinken."],
      ["04", "Exportieren", "Die A4-Vorschau zeigt sofort das Ergebnis. Derselbe Renderer erzeugt anschließend die druckbare PDF."]
    ],
    downloadTitle: "Für dein System",
    downloadIntro:
      "Alle Pakete enthalten die Komponenten für den PDF-Import. Deine Songs und Bibliothek bleiben auf deinem Rechner.",
    recommended: "Empfohlen",
    install: "Herunterladen",
    windowsSetup: "Windows Installer",
    windowsSetupText: "Klassisches Setup mit Startmenü, Deinstallation und optionalem Desktop-Symbol.",
    windowsZip: "Windows Portable",
    windowsZipText: "ZIP entpacken und starten – ohne Installation.",
    appImage: "Linux AppImage",
    appImageText: "Portable Linux-Version. Ausführbar machen und direkt starten.",
    flatpak: "Linux Flatpak",
    flatpakText: "Sandbox-Paket für eine lokale Flatpak-Installation.",
    unsigned:
      "Hinweis: Der Windows-Installer ist noch nicht digital signiert. Windows kann deshalb einen unbekannten Herausgeber melden.",
    valuesTitle: "Für Musiker gebaut, nicht für Accounts",
    values: [
      ["Keine Cloudpflicht", "Bibliothek, Chordsheets und PDFs bleiben lokal."],
      ["Offenes Format", "Songs werden als lesbare JSON-basierte .ricksheet-Dateien gespeichert."],
      ["Freie Software", "GPL-3.0-or-later, mit öffentlichem Quellcode auf GitHub."]
    ],
    footer: "Entwickelt von Erik Heidenreich · Rick Rich",
    release: "Version 0.3.3",
    language: "English"
  },
  en: {
    navFeatures: "Features",
    navDownload: "Download",
    navSource: "Source",
    eyebrow: "RickSheets 0.3.3 · Linux & Windows",
    title: "Chord sheets that fit the page.",
    intro:
      "Import songs from text or PDF, tidy chords and sections visually, and export a consistent A4 chord sheet.",
    primary: "Download RickSheets",
    secondary: "View on GitHub",
    proof: "Local. Ad-free. Open source.",
    previewAlt: "RickSheets with song library, editor and A4 preview",
    featureTitle: "From found tab to playable sheet",
    featureIntro:
      "RickSheets keeps the workflow deliberately clear: import, review, edit, play.",
    features: [
      ["01", "Import", "Paste text or open a PDF. Navigation, chord diagrams and common website clutter are removed conservatively."],
      ["02", "Review", "Source and detected result sit side by side. Suspicious lines are highlighted before entering your library."],
      ["03", "Edit", "Reorder sections, place chords on exact words and transpose the song without wrestling with whitespace."],
      ["04", "Export", "The A4 preview shows the result immediately. The same renderer then creates the printable PDF."]
    ],
    downloadTitle: "Choose your system",
    downloadIntro:
      "Every package includes the PDF import components. Your songs and library stay on your computer.",
    recommended: "Recommended",
    install: "Download",
    windowsSetup: "Windows Installer",
    windowsSetupText: "Conventional setup with Start Menu, uninstaller and optional desktop shortcut.",
    windowsZip: "Windows Portable",
    windowsZipText: "Extract the ZIP and launch without installing.",
    appImage: "Linux AppImage",
    appImageText: "Portable Linux edition. Make executable and launch directly.",
    flatpak: "Linux Flatpak",
    flatpakText: "Sandboxed package for a local Flatpak installation.",
    unsigned:
      "Note: The Windows installer is not digitally signed yet. Windows may report an unknown publisher.",
    valuesTitle: "Built for musicians, not accounts",
    values: [
      ["No cloud required", "Your library, chord sheets and PDFs stay local."],
      ["Open format", "Songs are stored as readable JSON-based .ricksheet files."],
      ["Free software", "GPL-3.0-or-later, with public source code on GitHub."]
    ],
    footer: "Created by Erik Heidenreich · Rick Rich",
    release: "Version 0.3.3",
    language: "Deutsch"
  }
};

const downloads = [
  {
    os: "windows",
    key: "windowsSetup",
    text: "windowsSetupText",
    file: "RickSheets-0.3.3-Setup-x64.exe",
    meta: "Windows 10/11 · x64 · 20 MB",
    primary: true
  },
  {
    os: "windows",
    key: "windowsZip",
    text: "windowsZipText",
    file: "RickSheets-0.3.3-Windows-x64.zip",
    meta: "Windows 10/11 · x64 · 29 MB"
  },
  {
    os: "linux",
    key: "appImage",
    text: "appImageText",
    file: "RickSheets-0.3.3-x86_64.AppImage",
    meta: "Linux · x86_64 · 50 MB",
    primary: true
  },
  {
    os: "linux",
    key: "flatpak",
    text: "flatpakText",
    file: "RickSheets-0.3.3-x86_64.flatpak",
    meta: "Linux · x86_64 · 27 MB"
  }
];

export default function Home() {
  const [language, setLanguage] = useState("de");
  const [platform, setPlatform] = useState("");
  const t = copy[language];

  useEffect(() => {
    const browserLanguage = navigator.language?.toLowerCase() || "";
    if (!browserLanguage.startsWith("de")) setLanguage("en");
    const value = navigator.userAgent.toLowerCase();
    setPlatform(
      value.includes("windows")
        ? "windows"
        : value.includes("linux") && !value.includes("android")
          ? "linux"
          : ""
    );
  }, []);

  useEffect(() => {
    document.documentElement.lang = language;
  }, [language]);

  return (
    <main>
      <nav className="nav shell" aria-label="Main navigation">
        <a className="brand" href="#top" aria-label="RickSheets home">
          <span className="brandMark">R</span>
          <span>RICKSHEETS</span>
        </a>
        <div className="navLinks">
          <a href="#features">{t.navFeatures}</a>
          <a href="#download">{t.navDownload}</a>
          <a href="https://github.com/erixx-hub/RickSheets">{t.navSource}</a>
          <button
            className="language"
            type="button"
            onClick={() => setLanguage(language === "de" ? "en" : "de")}
          >
            {t.language}
          </button>
        </div>
      </nav>

      <header id="top" className="hero shell">
        <div className="heroCopy">
          <p className="eyebrow">{t.eyebrow}</p>
          <h1>{t.title}</h1>
          <p className="lead">{t.intro}</p>
          <div className="actions">
            <a className="button primary" href="#download">{t.primary}</a>
            <a className="button ghost" href="https://github.com/erixx-hub/RickSheets">
              {t.secondary} <span aria-hidden="true">↗</span>
            </a>
          </div>
          <p className="proof"><span aria-hidden="true">✓</span> {t.proof}</p>
        </div>
        <div className="heroVisual">
          <div className="sheetDecor sheetOne" aria-hidden="true">Am&nbsp;&nbsp;&nbsp;F<br />Take me home</div>
          <div className="appFrame">
            <div className="frameBar"><i /><i /><i /><span>RickSheets</span></div>
            <img
              src="https://raw.githubusercontent.com/erixx-hub/RickSheets/v0.3.3/docs/screenshots/ricksheets-main-en.png"
              alt={t.previewAlt}
            />
          </div>
          <div className="sheetDecor sheetTwo" aria-hidden="true">[Chorus]<br />G&nbsp;&nbsp;D&nbsp;&nbsp;Em</div>
        </div>
      </header>

      <section id="features" className="section shell">
        <div className="sectionHead">
          <p className="kicker">WORKFLOW</p>
          <h2>{t.featureTitle}</h2>
          <p>{t.featureIntro}</p>
        </div>
        <div className="featureGrid">
          {t.features.map(([number, title, text]) => (
            <article className="feature" key={number}>
              <span>{number}</span>
              <h3>{title}</h3>
              <p>{text}</p>
            </article>
          ))}
        </div>
      </section>

      <section id="download" className="downloadSection">
        <div className="shell">
          <div className="sectionHead inverse">
            <p className="kicker">RICKSHEETS 0.3.3</p>
            <h2>{t.downloadTitle}</h2>
            <p>{t.downloadIntro}</p>
          </div>
          <div className="downloadGrid">
            {downloads.map((item) => {
              const recommended = platform === item.os && item.primary;
              return (
                <article className={`downloadCard ${recommended ? "highlight" : ""}`} key={item.file}>
                  <div className="cardTop">
                    <span className={`osIcon ${item.os}`}>{item.os === "windows" ? "⊞" : "⌁"}</span>
                    {recommended && <span className="badge">{t.recommended}</span>}
                  </div>
                  <h3>{t[item.key]}</h3>
                  <p>{t[item.text]}</p>
                  <small>{item.meta}</small>
                  <a href={`${releaseBase}/${item.file}`}>
                    {t.install} <span aria-hidden="true">↓</span>
                  </a>
                </article>
              );
            })}
          </div>
          <p className="unsigned">{t.unsigned}</p>
        </div>
      </section>

      <section className="values shell">
        <div className="sectionHead">
          <p className="kicker">PRIVACY & FREEDOM</p>
          <h2>{t.valuesTitle}</h2>
        </div>
        <div className="valueGrid">
          {t.values.map(([title, text], index) => (
            <article key={title}>
              <span aria-hidden="true">{["⌂", "{ }", "♡"][index]}</span>
              <h3>{title}</h3>
              <p>{text}</p>
            </article>
          ))}
        </div>
      </section>

      <footer>
        <div className="shell footerInner">
          <div className="brand"><span className="brandMark">R</span><span>RICKSHEETS</span></div>
          <p>{t.footer}</p>
          <div>
            <span>{t.release}</span>
            <a href="https://github.com/erixx-hub/RickSheets/blob/main/LICENSE">GPL-3.0-or-later</a>
          </div>
        </div>
      </footer>
    </main>
  );
}
