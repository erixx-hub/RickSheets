// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "appstyle.h"
#include "language.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QHBoxLayout>
#include <QIcon>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QStyleHints>
#include <QVBoxLayout>

namespace {
void integrateAppImage()
{
    const QString appImage = qEnvironmentVariable("APPIMAGE");
    if (appImage.isEmpty())
        return;

    const QString dataHome =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    const QString applicationsDirectory = QDir(dataHome).filePath("applications");
    const QString iconDirectory =
        QDir(dataHome).filePath("icons/hicolor/256x256/apps");
    QDir().mkpath(applicationsDirectory);
    QDir().mkpath(iconDirectory);

    QFile embeddedIcon(":/brand/de.rickrich.RickSheets.png");
    const QString installedIconPath =
        QDir(iconDirectory).filePath("io.github.erixx_hub.RickSheets.png");
    QSaveFile installedIcon(installedIconPath);
    if (embeddedIcon.open(QIODevice::ReadOnly) &&
        installedIcon.open(QIODevice::WriteOnly)) {
        installedIcon.write(embeddedIcon.readAll());
        installedIcon.commit();
    }

    QString escapedPath = QFileInfo(appImage).absoluteFilePath();
    escapedPath.replace("\\", "\\\\");
    escapedPath.replace("\"", "\\\"");
    QSaveFile desktopFile(
        QDir(applicationsDirectory)
            .filePath("io.github.erixx_hub.RickSheets.desktop"));
    if (desktopFile.open(QIODevice::WriteOnly)) {
        const QString contents = QString(
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=RickSheets\n"
            "Comment=Import, edit and export printable chord sheets\n"
            "Comment[de]=Chordsheets importieren, bearbeiten und als PDF exportieren\n"
            "Exec=\"%1\" %F\n"
            "Icon=%2\n"
            "Terminal=false\n"
            "Categories=AudioVideo;Audio;\n"
            "StartupNotify=true\n"
            "StartupWMClass=ricksheets\n")
                                     .arg(escapedPath, installedIconPath);
        desktopFile.write(contents.toUtf8());
        desktopFile.setDirectWriteFallback(true);
        if (desktopFile.commit())
            QFile::setPermissions(desktopFile.fileName(),
            QFileDevice::ReadOwner | QFileDevice::WriteOwner |
            QFileDevice::ExeOwner | QFileDevice::ReadGroup |
            QFileDevice::ReadOther);
    }
}

bool showFirstStartWelcome()
{
    QSettings settings;
    constexpr int onboardingVersion = 3;
    const bool captureOnboarding =
        !qEnvironmentVariable("RICKSHEETS_ONBOARDING_SCREENSHOT").isEmpty();
    if (!captureOnboarding &&
        settings.value("onboarding/version", 0).toInt() >= onboardingVersion) {
        const QString desktopFile = QDir(
            QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation))
                                        .filePath(
                                            "applications/"
                                            "io.github.erixx_hub.RickSheets.desktop");
        if (!qEnvironmentVariable("APPIMAGE").isEmpty() &&
            QFileInfo::exists(desktopFile))
            integrateAppImage();
        return true;
    }

    QDialog dialog;
    dialog.setWindowTitle(QObject::tr("Willkommen bei RickSheets"));
    dialog.setWindowIcon(QIcon(":/brand/de.rickrich.RickSheets.png"));
    dialog.setFixedSize(650, 540);
    dialog.setStyleSheet(R"(
QDialog { background:palette(window); color:palette(window-text); }
QLabel { color:palette(window-text); }
QLabel#title { color:palette(window-text); font-size:21pt; font-weight:800; }
QLabel#subtitle { color:palette(placeholder-text); font-size:10.5pt; }
QLabel#intro {
  background:palette(base); color:palette(text);
  border:1px solid palette(mid); border-radius:10px;
  padding:16px; font-size:10.5pt;
}
QPushButton {
  border-radius:10px; padding:14px; font-size:12pt; font-weight:700;
  min-height:58px;
}
)");

    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(30, 26, 30, 24);
    layout->setSpacing(14);

    auto *header = new QHBoxLayout;
    auto *logo = new QLabel;
    const bool darkOnboarding =
        qApp->palette().color(QPalette::Window).lightness() < 128;
    logo->setPixmap(
        QPixmap(darkOnboarding ? ":/brand/ricksheets-icon-light.svg"
                               : ":/brand/ricksheets-icon-dark.svg")
            .scaled(76, 76, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto *heading = new QVBoxLayout;
    auto *title = new QLabel(QObject::tr("Willkommen bei RickSheets"));
    title->setObjectName("title");
    auto *subtitle = new QLabel(
        QObject::tr("Aus Akkorden und Songtext wird ein einheitliches Chordsheet."));
    subtitle->setObjectName("subtitle");
    subtitle->setWordWrap(true);
    heading->addStretch();
    heading->addWidget(title);
    heading->addWidget(subtitle);
    heading->addStretch();
    header->addWidget(logo);
    header->addSpacing(12);
    header->addLayout(heading, 1);
    layout->addLayout(header);

    auto *intro = new QLabel(QObject::tr(
        "<b>Dein Arbeitsablauf:</b><br>"
        "Songs aus Text oder PDF importieren, Akkorde positionsgenau bearbeiten "
        "und transponieren. RickSheets hält eine lokale Bibliothekskopie bereit "
        "und zeigt jederzeit die fertige A4-Seite für den PDF-Export."));
    intro->setObjectName("intro");
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto *choiceLabel = new QLabel(
        QObject::tr("<b>Wie soll RickSheets aussehen?</b>"));
    layout->addWidget(choiceLabel);
    auto *choices = new QHBoxLayout;
    choices->setSpacing(12);
    auto *light = new QPushButton(QObject::tr("☀  Helle Darstellung"));
    light->setObjectName("lightChoice");
    light->setStyleSheet(
        "QPushButton { background:#ffffff; color:#171717; "
        "border:2px solid #c6b21e; }"
        "QPushButton:hover { background:#fffbea; }");
    auto *dark = new QPushButton(QObject::tr("●  Dunkle Darstellung"));
    dark->setObjectName("darkChoice");
    dark->setStyleSheet(
        "QPushButton { background:#242424; color:#f3efe4; "
        "border:2px solid #c6b21e; }"
        "QPushButton:hover { background:#343434; }");
    choices->addWidget(light);
    choices->addWidget(dark);
    layout->addLayout(choices);

    auto *note = new QLabel(QObject::tr(
        "Die Auswahl lässt sich später unter Ansicht → Darstellung ändern."));
    note->setObjectName("subtitle");
    note->setAlignment(Qt::AlignCenter);
    layout->addWidget(note);
    auto *integration = new QCheckBox(QObject::tr(
        "RickSheets im Anwendungsmenü integrieren "
        "(empfohlen für das Taskleisten-Icon)"));
    integration->setChecked(!qEnvironmentVariable("APPIMAGE").isEmpty());
    integration->setVisible(!qEnvironmentVariable("APPIMAGE").isEmpty());
    layout->addWidget(integration);
    layout->addStretch();
    auto *copyright = new QLabel(
        QObject::tr("RickSheets %1  ·  © 2026 Erik Heidenreich (Rick Rich)")
            .arg(RICKSHEETS_VERSION));
    copyright->setObjectName("subtitle");
    copyright->setAlignment(Qt::AlignCenter);
    layout->addWidget(copyright);

    QObject::connect(light, &QPushButton::clicked, &dialog, [&] {
        settings.setValue("appearance/theme", "light");
        settings.setValue("onboarding/completed", true);
        settings.setValue("onboarding/version", onboardingVersion);
        if (integration->isChecked())
            integrateAppImage();
        dialog.accept();
    });
    QObject::connect(dark, &QPushButton::clicked, &dialog, [&] {
        settings.setValue("appearance/theme", "dark");
        settings.setValue("onboarding/completed", true);
        settings.setValue("onboarding/version", onboardingVersion);
        if (integration->isChecked())
            integrateAppImage();
        dialog.accept();
    });

    const QString screenshotPath =
        qEnvironmentVariable("RICKSHEETS_ONBOARDING_SCREENSHOT");
    if (!screenshotPath.isEmpty()) {
        dialog.show();
        QApplication::processEvents();
        QImage image(dialog.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        dialog.render(&painter);
        painter.end();
        image.save(screenshotPath);
        return false;
    }
    return dialog.exec() == QDialog::Accepted;
}
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName("RickSheets");
    application.setApplicationDisplayName("RickSheets");
    application.setOrganizationName("RickSheets");
    application.setApplicationVersion(RICKSHEETS_VERSION);
    application.setDesktopFileName("io.github.erixx_hub.RickSheets");
    application.setWindowIcon(QIcon(":/brand/de.rickrich.RickSheets.png"));
    bool systemDark =
        application.styleHints()->colorScheme() == Qt::ColorScheme::Dark;
    if (application.styleHints()->colorScheme() == Qt::ColorScheme::Unknown)
        systemDark =
            application.palette().color(QPalette::Window).lightness() < 128;
    const QString testTheme =
        qEnvironmentVariable("RICKSHEETS_TEST_SYSTEM_THEME").toLower();
    if (testTheme == "dark" || testTheme == "light")
        systemDark = testTheme == "dark";
    application.setProperty("ricksheetsSystemDark", systemDark);
    application.setStyle(QStyleFactory::create("Fusion"));
    application.setStyleSheet(rickSheetsStyleSheet());
    applyRickSheetsLanguage(application);
    const QString testPreference =
        qEnvironmentVariable("RICKSHEETS_TEST_THEME_PREFERENCE").toLower();
    const QString themePreference =
        testPreference == "system" || testPreference == "dark" ||
                testPreference == "light"
            ? testPreference
            : testTheme == "dark" || testTheme == "light"
                  ? "system"
                  : QSettings().value("appearance/theme", "system").toString();
    applyRickSheetsTheme(application, themePreference);

    QFont font("DejaVu Sans");
    font.setPointSize(10);
    application.setFont(font);

    if (!showFirstStartWelcome())
        return 0;

    MainWindow window;
    const QStringList arguments = application.arguments();
    if (arguments.size() > 1) {
        const QFileInfo document(arguments.at(1));
        if (document.isFile() &&
            document.suffix().compare("ricksheet", Qt::CaseInsensitive) == 0) {
            window.openDocument(document.absoluteFilePath());
        }
    }
    window.showWithSavedWindowState();
    return application.exec();
}
