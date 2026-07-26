import "./styles.css";

export const metadata = {
  title: "RickSheets — Chordsheets, die auf die Seite passen",
  description:
    "Songs aus Text oder PDF importieren, Akkorde bearbeiten und einheitliche Chordsheets als PDF exportieren.",
  openGraph: {
    title: "RickSheets",
    description:
      "Aus Text und PDF wird ein übersichtliches, druckbares Chordsheet.",
    type: "website"
  }
};

export default function RootLayout({ children }) {
  return (
    <html lang="de">
      <body>{children}</body>
    </html>
  );
}
