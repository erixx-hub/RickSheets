// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appstyle.h"

#include <QApplication>
#include <QColor>
#include <QStyleHints>
#include <QWidget>

QString rickSheetsStyleSheet()
{
    return R"(
QWidget {
  selection-background-color:#d8c126;
  selection-color:#171717;
}
QMainWindow, QDialog { background:palette(window); }
QLabel#sectionHeading, QLabel#dialogHeading {
  color:palette(placeholder-text);
  font-size:9pt;
  font-weight:800;
  letter-spacing:1px;
}
QFrame#editorToolbar, QFrame#previewToolbar {
  background:palette(alternate-base);
  border:1px solid palette(mid);
  border-radius:10px;
}
QLabel#lyricCard {
  background:palette(alternate-base);
  border:1px solid palette(mid);
  border-radius:9px;
  padding:10px;
}
QLabel#pageStatus {
  border-radius:10px;
  padding:4px 9px;
  font-weight:800;
}
QGroupBox {
  border:1px solid palette(mid);
  border-radius:10px;
  margin-top:14px;
  padding:12px;
  font-weight:700;
}
QGroupBox::title {
  subcontrol-origin:margin;
  left:12px;
  padding:0 6px;
}
QLineEdit, QPlainTextEdit, QSpinBox, QListWidget {
  background:palette(base);
  color:palette(text);
  border:1px solid palette(mid);
  border-radius:8px;
  padding:7px;
}
QLineEdit:focus, QPlainTextEdit:focus, QSpinBox:focus, QListWidget:focus {
  border:2px solid #c6b21e;
  padding:6px;
}
QLineEdit[readOnly="true"], QPlainTextEdit[readOnly="true"] {
  background:palette(alternate-base);
}
QListWidget::item {
  border-radius:6px;
  padding:7px 8px;
  margin:1px 0;
}
QListWidget::item:hover { background:palette(alternate-base); }
QListWidget::item:selected {
  background:#d8c126;
  color:#171717;
}
QPushButton {
  background:palette(button);
  color:palette(button-text);
  border:1px solid palette(mid);
  border-radius:8px;
  padding:7px 12px;
  min-height:20px;
}
QPushButton:hover {
  background:palette(light);
  border-color:#b9a817;
}
QPushButton:pressed { background:palette(midlight); }
QPushButton:disabled {
  color:palette(mid);
  background:palette(alternate-base);
}
QPushButton#primaryButton, QPushButton[primary="true"] {
  background:#d8c126;
  color:#171717;
  border-color:#b5a00d;
  font-weight:800;
}
QPushButton#primaryButton:hover, QPushButton[primary="true"]:hover {
  background:#ead42f;
}
QPushButton[quiet="true"] {
  background:transparent;
  border-color:transparent;
}
QPushButton[quiet="true"]:hover {
  background:palette(alternate-base);
  border-color:palette(mid);
}
QPushButton[chordSet="true"] {
  background:#fff8cc;
  color:#171717;
  border:2px solid #c6b21e;
  font-weight:800;
}
QTabWidget::pane {
  border:1px solid palette(mid);
  border-radius:9px;
  top:-1px;
}
QTabBar::tab {
  background:transparent;
  border:1px solid transparent;
  border-radius:7px;
  padding:8px 14px;
  margin-right:3px;
}
QTabBar::tab:hover { background:palette(alternate-base); }
QTabBar::tab:selected {
  background:palette(button);
  border-color:palette(mid);
  font-weight:700;
}
QScrollArea { border:0; background:transparent; }
QScrollBar:vertical {
  background:transparent;
  width:11px;
  margin:2px;
}
QScrollBar::handle:vertical {
  background:palette(mid);
  border-radius:4px;
  min-height:28px;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }
QScrollBar:horizontal {
  background:transparent;
  height:11px;
  margin:2px;
}
QScrollBar::handle:horizontal {
  background:palette(mid);
  border-radius:4px;
  min-width:28px;
}
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width:0; }
QSplitter::handle { background:palette(mid); }
QSplitter::handle:horizontal { width:1px; }
QMenuBar {
  background:palette(window);
  border-bottom:1px solid palette(mid);
  padding:3px;
}
QMenuBar::item { padding:6px 9px; border-radius:6px; }
QMenuBar::item:selected { background:palette(alternate-base); }
QMenu {
  background:palette(base);
  border:1px solid palette(mid);
  border-radius:8px;
  padding:5px;
}
QMenu::item { padding:7px 24px 7px 10px; border-radius:5px; }
QMenu::item:selected { background:#d8c126; color:#171717; }
QStatusBar {
  background:palette(alternate-base);
  border-top:1px solid palette(mid);
}
QToolTip {
  background:palette(base);
  color:palette(text);
  border:1px solid palette(mid);
  padding:5px;
}
)";
}

QPalette rickSheetsDarkPalette()
{
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#191b1f"));
    palette.setColor(QPalette::WindowText, QColor("#eeeeee"));
    palette.setColor(QPalette::Base, QColor("#22252a"));
    palette.setColor(QPalette::AlternateBase, QColor("#282c32"));
    palette.setColor(QPalette::ToolTipBase, QColor("#eeeeee"));
    palette.setColor(QPalette::ToolTipText, QColor("#171717"));
    palette.setColor(QPalette::Text, QColor("#eeeeee"));
    palette.setColor(QPalette::Button, QColor("#30343b"));
    palette.setColor(QPalette::ButtonText, QColor("#eeeeee"));
    palette.setColor(QPalette::BrightText, Qt::white);
    palette.setColor(QPalette::Highlight, QColor("#d8c126"));
    palette.setColor(QPalette::HighlightedText, QColor("#171717"));
    palette.setColor(QPalette::PlaceholderText, QColor("#a5a5a5"));
    palette.setColor(QPalette::Mid, QColor("#41464f"));
    palette.setColor(QPalette::Midlight, QColor("#353a42"));
    palette.setColor(QPalette::Light, QColor("#3b4048"));
    palette.setColor(QPalette::Shadow, QColor("#101216"));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#888888"));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#888888"));
    return palette;
}

QPalette rickSheetsLightPalette()
{
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#f3f1eb"));
    palette.setColor(QPalette::WindowText, QColor("#171717"));
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::AlternateBase, QColor("#f7f5ef"));
    palette.setColor(QPalette::ToolTipBase, QColor("#fffbe8"));
    palette.setColor(QPalette::ToolTipText, QColor("#171717"));
    palette.setColor(QPalette::Text, QColor("#171717"));
    palette.setColor(QPalette::Button, QColor("#ece9e1"));
    palette.setColor(QPalette::ButtonText, QColor("#171717"));
    palette.setColor(QPalette::BrightText, Qt::white);
    palette.setColor(QPalette::Highlight, QColor("#d8c126"));
    palette.setColor(QPalette::HighlightedText, QColor("#171717"));
    palette.setColor(QPalette::PlaceholderText, QColor("#6b6862"));
    palette.setColor(QPalette::Mid, QColor("#d7d2c7"));
    palette.setColor(QPalette::Midlight, QColor("#e4e0d7"));
    palette.setColor(QPalette::Light, QColor("#f5f3ed"));
    palette.setColor(QPalette::Shadow, QColor("#aaa59a"));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#777777"));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#777777"));
    return palette;
}

QString rickSheetsEffectiveTheme(const QApplication &application,
                                 const QString &preference)
{
    if (preference == "dark" || preference == "light")
        return preference;

    const QVariant detected = application.property("ricksheetsSystemDark");
    if (detected.isValid())
        return detected.toBool() ? "dark" : "light";
    return application.styleHints()->colorScheme() == Qt::ColorScheme::Dark
               ? "dark"
               : "light";
}

void applyRickSheetsTheme(QApplication &application,
                          const QString &preference)
{
    const QPalette palette =
        rickSheetsEffectiveTheme(application, preference) == "dark"
            ? rickSheetsDarkPalette()
            : rickSheetsLightPalette();
    application.setPalette(palette);
    for (QWidget *widget : application.allWidgets()) {
        widget->setPalette(palette);
        widget->update();
    }
}
