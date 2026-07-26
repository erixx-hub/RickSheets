// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "appstyle.h"
#include "blockeditordialog.h"
#include "chordeditordialog.h"
#include "chordparser.h"
#include "importreviewdialog.h"
#include "language.h"
#include "previewwidget.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QCryptographicHash>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QImage>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPixmap>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QSplitter>
#include <QStandardPaths>
#include <QStyle>
#include <QStyleHints>
#include <QStyledItemDelegate>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextStream>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>

namespace {
constexpr int TitleRole = Qt::UserRole + 1;
constexpr int ArtistRole = Qt::UserRole + 2;

class LibraryItemDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override
    {
        Q_UNUSED(option);
        return QSize(180, index.data(ArtistRole).toString().isEmpty() ? 36 : 50);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem itemOption(option);
        initStyleOption(&itemOption, index);
        itemOption.text.clear();
        const QStyle *style =
            itemOption.widget ? itemOption.widget->style() : QApplication::style();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &itemOption,
                             painter, itemOption.widget);

        const bool selected = option.state & QStyle::State_Selected;
        const QColor primary =
            selected ? QColor("#ffffff") : option.palette.color(QPalette::Text);
        const QColor secondary =
            selected ? QColor("#dddddd")
                     : option.palette.color(QPalette::PlaceholderText);
        const QRect content = option.rect.adjusted(9, 5, -8, -5);
        const QString title = index.data(TitleRole).toString();
        const QString artist = index.data(ArtistRole).toString();

        painter->save();
        QFont titleFont = option.font;
        titleFont.setBold(true);
        titleFont.setPointSizeF(titleFont.pointSizeF() + 0.5);
        painter->setFont(titleFont);
        painter->setPen(primary);
        const QString titleText =
            QFontMetrics(titleFont).elidedText(title, Qt::ElideRight, content.width());
        painter->drawText(
            artist.isEmpty() ? content : QRect(content.left(), content.top(),
                                               content.width(), 22),
            artist.isEmpty() ? Qt::AlignVCenter : Qt::AlignTop,
            titleText);

        if (!artist.isEmpty()) {
            QFont artistFont = option.font;
            artistFont.setPointSizeF(qMax(8.0, artistFont.pointSizeF() - 1.0));
            painter->setFont(artistFont);
            painter->setPen(secondary);
            painter->drawText(
                QRect(content.left(), content.top() + 24, content.width(), 18),
                Qt::AlignTop,
                QFontMetrics(artistFont).elidedText(
                    artist, Qt::ElideRight, content.width()));
        }
        painter->restore();
    }
};

QPixmap wordmarkForPalette(const QPalette &palette)
{
    QPixmap wordmark =
        QPixmap(":/brand/ricksheets-wordmark.svg")
            .scaledToWidth(180, Qt::SmoothTransformation);
    if (palette.color(QPalette::Window).lightness() >= 128)
        return wordmark;

    QImage image = wordmark.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); ++y) {
        auto *pixels = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            const int alpha = qAlpha(pixels[x]);
            if (alpha == 0)
                continue;
            const QColor source = QColor::fromRgba(pixels[x]);
            const QColor target =
                source.lightness() < 65 ? QColor("#f3efe4") : QColor("#aaa69d");
            pixels[x] = qRgba(target.red(), target.green(), target.blue(), alpha);
        }
    }
    return QPixmap::fromImage(image);
}

QPalette darkPalette()
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

QPalette lightPalette()
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
    palette.setColor(QPalette::PlaceholderText, QColor("#777777"));
    palette.setColor(QPalette::Mid, QColor("#d7d2c7"));
    palette.setColor(QPalette::Midlight, QColor("#e4e0d7"));
    palette.setColor(QPalette::Light, QColor("#f5f3ed"));
    palette.setColor(QPalette::Shadow, QColor("#aaa59a"));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#888888"));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#888888"));
    return palette;
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    qApp->setStyleSheet(rickSheetsStyleSheet());
    applyTheme(QSettings().value("appearance/theme", "system").toString());
    buildUi();
    buildMenus();
    m_previewTimer = new QTimer(this);
    m_previewTimer->setSingleShot(true);
    m_previewTimer->setInterval(180);
    connect(m_previewTimer, &QTimer::timeout, this, &MainWindow::updatePreview);
    newSong();

    QSettings settings;
    const QByteArray geometry = settings.value("window/geometry").toByteArray();
    if (!geometry.isEmpty())
        restoreGeometry(geometry);
    if (settings.value("window/maximized", false).toBool())
        setWindowState(windowState() | Qt::WindowMaximized);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!maybeSave()) {
        event->ignore();
        return;
    }

    QSettings settings;
    settings.setValue("window/geometry", saveGeometry());
    settings.setValue("window/maximized", isMaximized());
    settings.sync();
    event->accept();
}

void MainWindow::buildUi()
{
    setWindowTitle("RickSheets");
    setWindowIcon(QIcon(":/brand/de.rickrich.RickSheets.png"));
    resize(1480, 900);
    setMinimumSize(1040, 680);

    auto *splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);
    setCentralWidget(splitter);

    auto *libraryPane = new QWidget;
    libraryPane->setObjectName("libraryPane");
    libraryPane->setMinimumWidth(230);
    auto *libraryLayout = new QVBoxLayout(libraryPane);
    libraryLayout->setContentsMargins(14, 16, 14, 14);
    libraryLayout->setSpacing(9);
    m_brand = new QLabel;
    m_brand->setObjectName("brandWordmark");
    m_brand->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_brand->setPixmap(wordmarkForPalette(palette()));
    m_brand->setAccessibleName(tr("RickSheets"));
    auto *libraryLabel = new QLabel(tr("BIBLIOTHEK"));
    libraryLabel->setObjectName("sectionHeading");
    QFont labelFont = libraryLabel->font();
    labelFont.setBold(true);
    libraryLabel->setFont(labelFont);
    m_librarySearch = new QLineEdit;
    m_librarySearch->setObjectName("librarySearch");
    m_librarySearch->setPlaceholderText(tr("Suchen …"));
    m_library = new QListWidget;
    m_library->setObjectName("songLibrary");
    m_library->setAlternatingRowColors(true);
    m_library->setItemDelegate(new LibraryItemDelegate(m_library));
    m_library->setToolTip(tr("Doppelklick oder Eingabetaste öffnet einen Song"));
    auto *newButton = new QPushButton(tr("+ Neuer Song"));
    newButton->setProperty("primary", true);
    auto *importButton = new QPushButton(tr("Text importieren"));
    auto *pdfImportButton = new QPushButton(tr("PDF importieren"));
    m_deleteLibraryButton = new QPushButton(tr("In Papierkorb"));
    m_deleteLibraryButton->setObjectName("deleteLibrarySong");
    m_deleteLibraryButton->setProperty("quiet", true);
    m_deleteLibraryButton->setEnabled(false);
    connect(newButton, &QPushButton::clicked, this, &MainWindow::newSong);
    connect(importButton, &QPushButton::clicked, this, &MainWindow::importText);
    connect(pdfImportButton, &QPushButton::clicked, this, &MainWindow::importPdf);
    connect(m_librarySearch, &QLineEdit::textChanged,
            this, &MainWindow::refreshLibrary);
    connect(m_library, &QListWidget::itemActivated,
            this, &MainWindow::openLibraryItem);
    connect(m_library, &QListWidget::itemSelectionChanged, this, [this] {
        const QListWidgetItem *item = m_library->currentItem();
        m_deleteLibraryButton->setEnabled(
            item && !item->data(Qt::UserRole).toString().isEmpty());
    });
    connect(m_deleteLibraryButton, &QPushButton::clicked,
            this, &MainWindow::deleteLibrarySong);
    libraryLayout->addWidget(m_brand);
    libraryLayout->addSpacing(4);
    libraryLayout->addWidget(libraryLabel);
    libraryLayout->addWidget(m_librarySearch);
    libraryLayout->addWidget(m_library, 1);
    libraryLayout->addWidget(newButton);
    auto *importActions = new QHBoxLayout;
    importActions->setSpacing(7);
    importActions->addWidget(importButton);
    importActions->addWidget(pdfImportButton);
    libraryLayout->addLayout(importActions);
    libraryLayout->addWidget(m_deleteLibraryButton);
    splitter->addWidget(libraryPane);

    auto *editorScroll = new QScrollArea;
    editorScroll->setWidgetResizable(true);
    auto *editorPane = new QWidget;
    editorPane->setObjectName("editorPane");
    auto *editorLayout = new QVBoxLayout(editorPane);
    editorLayout->setContentsMargins(18, 16, 18, 16);
    editorLayout->setSpacing(11);

    auto *songHeading = new QLabel(tr("SONG"));
    songHeading->setObjectName("sectionHeading");
    songHeading->setFont(labelFont);
    auto *formBox = new QGroupBox(tr("Songdaten"));
    auto *form = new QFormLayout(formBox);
    m_titleEdit = new QLineEdit;
    m_artistEdit = new QLineEdit;
    m_keyEdit = new QLineEdit;
    m_keyEdit->setMaximumWidth(90);
    m_bpmEdit = new QSpinBox;
    m_bpmEdit->setRange(0, 300);
    m_bpmEdit->setSpecialValueText("–");
    m_capoEdit = new QSpinBox;
    m_capoEdit->setRange(0, 12);
    m_capoEdit->setSpecialValueText(tr("kein"));
    form->addRow(tr("Titel"), m_titleEdit);
    form->addRow(tr("Interpret"), m_artistEdit);
    form->addRow(tr("Tonart"), m_keyEdit);
    form->addRow(tr("BPM"), m_bpmEdit);
    form->addRow(tr("Capo"), m_capoEdit);

    auto *editorToolbar = new QFrame;
    editorToolbar->setObjectName("editorToolbar");
    auto *editorHeader = new QHBoxLayout(editorToolbar);
    editorHeader->setContentsMargins(9, 6, 7, 6);
    editorHeader->setSpacing(6);
    auto *contentLabel = new QLabel(tr("ARRANGEMENT"));
    contentLabel->setObjectName("sectionHeading");
    contentLabel->setFont(labelFont);
    auto *transposeButton = new QPushButton(tr("Transponieren"));
    transposeButton->setToolTip(tr("Tonart in Halbtonschritten ändern"));
    auto *blocksButton = new QPushButton(tr("Abschnitte"));
    blocksButton->setToolTip(tr("Abschnitte visuell bearbeiten und sortieren"));
    blocksButton->setObjectName("editBlocks");
    auto *chordsButton = new QPushButton(tr("Akkorde"));
    chordsButton->setToolTip(tr("Akkorde visuell Wörtern zuordnen"));
    chordsButton->setObjectName("editChords");
    connect(transposeButton, &QPushButton::clicked, this, &MainWindow::transposeSong);
    connect(blocksButton, &QPushButton::clicked, this, &MainWindow::editBlocks);
    connect(chordsButton, &QPushButton::clicked, this, &MainWindow::editChords);
    editorHeader->addWidget(contentLabel);
    editorHeader->addStretch();
    editorHeader->addWidget(blocksButton);
    editorHeader->addWidget(chordsButton);
    editorHeader->addWidget(transposeButton);

    m_contentEdit = new QPlainTextEdit;
    m_contentEdit->setObjectName("arrangementEditor");
    m_contentEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_contentEdit->setPlaceholderText(
        tr("[Intro]\n| A | D E | A |\n\n[Verse 1]\n"
           "You took my [Am]heart and turned me [F]on"));
    QFont mono("DejaVu Sans Mono");
    mono.setStyleHint(QFont::Monospace);
    mono.setPointSize(10);
    m_contentEdit->setFont(mono);
    m_contentEdit->setMinimumHeight(460);

    editorLayout->addWidget(songHeading);
    editorLayout->addWidget(formBox);
    editorLayout->addWidget(editorToolbar);
    editorLayout->addWidget(m_contentEdit, 1);
    editorScroll->setWidget(editorPane);
    splitter->addWidget(editorScroll);

    auto *previewPane = new QWidget;
    previewPane->setObjectName("previewPane");
    auto *previewLayout = new QVBoxLayout(previewPane);
    previewLayout->setContentsMargins(12, 16, 12, 12);
    auto *previewToolbar = new QFrame;
    previewToolbar->setObjectName("previewToolbar");
    auto *previewHeader = new QHBoxLayout(previewToolbar);
    previewHeader->setContentsMargins(10, 6, 7, 6);
    auto *previewLabel = new QLabel(tr("A4-VORSCHAU"));
    previewLabel->setObjectName("sectionHeading");
    previewLabel->setFont(labelFont);
    m_pageStatus = new QLabel(tr("1 Seite"));
    m_pageStatus->setObjectName("pageStatus");
    m_pageStatus->setStyleSheet(
        "background:#c9e6c4; color:#171717; padding:4px 8px; "
        "border:1px solid #8fbd88; border-radius:4px; font-weight:700;");
    auto *pdfButton = new QPushButton(tr("PDF exportieren"));
    pdfButton->setObjectName("primaryButton");
    connect(pdfButton, &QPushButton::clicked, this, &MainWindow::exportPdf);
    previewHeader->addWidget(previewLabel);
    previewHeader->addStretch();
    previewHeader->addWidget(m_pageStatus);
    previewHeader->addWidget(pdfButton);

    auto *previewScroll = new QScrollArea;
    previewScroll->setWidgetResizable(false);
    previewScroll->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_preview = new PreviewWidget;
    previewScroll->setWidget(m_preview);
    connect(m_preview, &PreviewWidget::pageCountChanged, this, &MainWindow::updatePageStatus);
    previewLayout->addWidget(previewToolbar);
    previewLayout->addWidget(previewScroll, 1);
    splitter->addWidget(previewPane);
    splitter->setSizes({240, 555, 685});
    refreshLibrary();

    const auto textChanged = [this] {
        setDirty(true);
        schedulePreview();
    };
    connect(m_titleEdit, &QLineEdit::textChanged, this, textChanged);
    connect(m_artistEdit, &QLineEdit::textChanged, this, textChanged);
    connect(m_keyEdit, &QLineEdit::textChanged, this, textChanged);
    connect(m_bpmEdit, &QSpinBox::valueChanged, this, textChanged);
    connect(m_capoEdit, &QSpinBox::valueChanged, this, textChanged);
    connect(m_contentEdit, &QPlainTextEdit::textChanged, this, textChanged);
    const auto updateChordOverwriteMode = [this] {
        const QString line = m_contentEdit->textCursor().block().text();
        m_contentEdit->setOverwriteMode(ChordParser::isChordLine(line));
    };
    connect(m_contentEdit, &QPlainTextEdit::cursorPositionChanged,
            this, updateChordOverwriteMode);
    connect(m_contentEdit, &QPlainTextEdit::textChanged,
            this, updateChordOverwriteMode);
    statusBar()->showMessage(tr("Bereit"));
}

void MainWindow::buildMenus()
{
    auto *fileMenu = menuBar()->addMenu(tr("&Datei"));
    auto *newAction = fileMenu->addAction(tr("&Neuer Song"), QKeySequence::New, this, &MainWindow::newSong);
    Q_UNUSED(newAction);
    fileMenu->addAction(tr("Text &importieren …"), QKeySequence("Ctrl+I"), this, &MainWindow::importText);
    fileMenu->addAction(tr("&PDF importieren …"), this, &MainWindow::importPdf);
    fileMenu->addAction(tr("&Öffnen …"), QKeySequence::Open, this, &MainWindow::openSong);
    fileMenu->addAction(tr("&Speichern"), QKeySequence::Save, this, &MainWindow::saveSong);
    fileMenu->addAction(tr("Speichern &unter …"), QKeySequence::SaveAs, this, &MainWindow::saveSongAs);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("PDF &exportieren …"), QKeySequence("Ctrl+E"), this, &MainWindow::exportPdf);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Beenden"), QKeySequence::Quit, this, &QWidget::close);

    auto *songMenu = menuBar()->addMenu(tr("&Song"));
    songMenu->addAction(tr("&Transponieren …"), QKeySequence("Ctrl+Shift+T"), this, &MainWindow::transposeSong);

    auto *viewMenu = menuBar()->addMenu(tr("&Ansicht"));
    auto *themeMenu = viewMenu->addMenu(tr("Darstellung"));
    auto *themeGroup = new QActionGroup(this);
    themeGroup->setExclusive(true);
    const QString selectedTheme =
        QSettings().value("appearance/theme", "system").toString();
    auto addThemeAction = [&](const QString &label, const QString &value,
                              auto slot) {
        QAction *action = themeMenu->addAction(label);
        action->setCheckable(true);
        action->setChecked(selectedTheme == value);
        themeGroup->addAction(action);
        connect(action, &QAction::triggered, this, slot);
    };
    addThemeAction(tr("System"), "system", &MainWindow::setSystemTheme);
    addThemeAction(tr("Hell"), "light", &MainWindow::setLightTheme);
    addThemeAction(tr("Dunkel"), "dark", &MainWindow::setDarkTheme);

    auto *languageMenu = viewMenu->addMenu(tr("Sprache"));
    auto *languageGroup = new QActionGroup(this);
    languageGroup->setExclusive(true);
    const QString selectedLanguage = rickSheetsLanguagePreference();
    auto addLanguageAction = [&](const QString &label, const QString &value,
                                 auto slot) {
        QAction *action = languageMenu->addAction(label);
        action->setCheckable(true);
        action->setChecked(selectedLanguage == value);
        languageGroup->addAction(action);
        connect(action, &QAction::triggered, this, slot);
    };
    addLanguageAction(tr("Systemsprache"), "system",
                      &MainWindow::setSystemLanguage);
    addLanguageAction(tr("Deutsch"), "de", &MainWindow::setGermanLanguage);
    addLanguageAction(tr("Englisch"), "en", &MainWindow::setEnglishLanguage);
}

void MainWindow::chooseLanguage(const QString &language)
{
    if (rickSheetsLanguagePreference() == language)
        return;
    storeRickSheetsLanguagePreference(language);
    QMessageBox::information(
        this, tr("Sprache geändert"),
        tr("Die neue Sprache wird beim nächsten Start von RickSheets verwendet."));
}

void MainWindow::setSystemLanguage()
{
    chooseLanguage("system");
}

void MainWindow::setGermanLanguage()
{
    chooseLanguage("de");
}

void MainWindow::setEnglishLanguage()
{
    chooseLanguage("en");
}

void MainWindow::applyTheme(const QString &theme)
{
    QString effectiveTheme = theme;
    if (effectiveTheme == "system") {
        effectiveTheme =
            QApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark
                ? "dark"
                : "light";
    }
    const QPalette selectedPalette =
        effectiveTheme == "dark" ? darkPalette() : lightPalette();
    QApplication::setPalette(selectedPalette);
    for (QWidget *widget : QApplication::allWidgets()) {
        widget->setPalette(selectedPalette);
        widget->update();
    }
    QSettings().setValue("appearance/theme", theme);
    if (m_brand)
        m_brand->setPixmap(wordmarkForPalette(QApplication::palette()));
}

void MainWindow::setSystemTheme()
{
    applyTheme("system");
}

void MainWindow::setLightTheme()
{
    applyTheme("light");
}

void MainWindow::setDarkTheme()
{
    applyTheme("dark");
}

void MainWindow::newSong()
{
    if (!maybeSave())
        return;
    Song song;
    song.title = tr("Neuer Song");
    song.content = "[Intro]\n\n[Verse 1]\n";
    m_currentFile.clear();
    loadIntoEditor(song);
    setDirty(false);
}

void MainWindow::importText()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Chordsheet-Text importieren"));
    dialog.resize(760, 620);
    auto *layout = new QVBoxLayout(&dialog);
    auto *label = new QLabel(tr("Kopiere das Chordsheet möglichst vollständig in dieses Feld:"));
    auto *input = new QPlainTextEdit;
    input->setPlaceholderText(tr("Titel – Interpret\n\n[Verse 1]\n    Am       F\nText …"));
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Erkennen und übernehmen"));
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(label);
    layout->addWidget(input, 1);
    layout->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted || input->toPlainText().trimmed().isEmpty())
        return;
    Song imported = ChordParser::importText(input->toPlainText());
    ImportReviewDialog review(input->toPlainText(), imported,
                              tr("EINGEFÜGTER ORIGINALTEXT"), this);
    if (review.exec() != QDialog::Accepted)
        return;
    imported = review.reviewedSong();
    if (!maybeSave())
        return;
    loadIntoEditor(imported);
    m_currentFile = createLibraryCopy(imported);
    setDirty(m_currentFile.isEmpty());
    refreshLibrary();
    statusBar()->showMessage(
        m_currentFile.isEmpty()
            ? tr("Text importiert – bitte speichern und Erkennung prüfen")
            : tr("Text importiert und als Bibliothekskopie gespeichert – bitte Erkennung prüfen"),
        5000);
}

void MainWindow::importPdf()
{
    const QString fileName = QFileDialog::getOpenFileName(
        this, tr("Chordsheet-PDF importieren"), {}, tr("PDF-Dateien (*.pdf)"));
    if (fileName.isEmpty())
        return;

    QProcess process;
    process.start("pdftotext", {"-raw", fileName, "-"});
    if (!process.waitForStarted(3000)) {
        QMessageBox::critical(
            this, tr("PDF-Import nicht verfügbar"),
            tr("Das Hilfsprogramm „pdftotext“ wurde nicht gefunden. "
               "Im späteren AppImage/Windows-Paket wird es mitgeliefert."));
        return;
    }
    if (!process.waitForFinished(20000) || process.exitCode() != 0) {
        QMessageBox::critical(
            this, tr("PDF-Import fehlgeschlagen"),
            tr("Der Text konnte nicht aus dem PDF gelesen werden.\n\n%1")
                .arg(QString::fromUtf8(process.readAllStandardError())));
        return;
    }
    const QString extracted = QString::fromUtf8(process.readAllStandardOutput());
    if (extracted.trimmed().isEmpty()) {
        QMessageBox::warning(
            this, tr("Kein Text gefunden"),
            tr("Dieses PDF enthält vermutlich nur eingescannte Bilder. OCR ist noch nicht eingebaut."));
        return;
    }
    Song imported =
        ChordParser::importText(extracted, QFileInfo(fileName).fileName());
    ImportReviewDialog review(extracted, imported,
                              tr("QUELLE AUS PDF: %1")
                                  .arg(QFileInfo(fileName).fileName()),
                              this, fileName);
    if (review.exec() != QDialog::Accepted)
        return;
    imported = review.reviewedSong();
    if (!maybeSave())
        return;
    loadIntoEditor(imported);
    m_currentFile = createLibraryCopy(imported);
    setDirty(m_currentFile.isEmpty());
    refreshLibrary();
    statusBar()->showMessage(
        m_currentFile.isEmpty()
            ? tr("Geprüfter PDF-Import übernommen – bitte speichern")
            : tr("Geprüfter PDF-Import als Bibliothekskopie gespeichert"),
        5000);
}

void MainWindow::openSong()
{
    if (!maybeSave())
        return;
    const QString fileName = QFileDialog::getOpenFileName(this, tr("RickSheets-Song öffnen"), {},
                                                          tr("RickSheets-Songs (*.ricksheet *.json)"));
    if (fileName.isEmpty())
        return;
    openSongFile(fileName);
}

bool MainWindow::openSongFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Öffnen fehlgeschlagen"), file.errorString());
        return false;
    }
    QString error;
    Song song = Song::fromJson(file.readAll(), &error);
    if (!error.isEmpty()) {
        QMessageBox::critical(this, tr("Ungültige Datei"), error);
        return false;
    }
    const QFileInfo sourceInfo(fileName);
    const QFileInfo libraryInfo(libraryDirectory());
    const bool alreadyManaged =
        sourceInfo.absolutePath() == libraryInfo.absoluteFilePath();
    m_currentFile = alreadyManaged
                        ? sourceInfo.absoluteFilePath()
                        : createLibraryCopy(song, sourceInfo.absoluteFilePath());
    if (m_currentFile.isEmpty())
        m_currentFile = sourceInfo.absoluteFilePath();
    if (!alreadyManaged && m_currentFile != sourceInfo.absoluteFilePath()) {
        QSettings settings;
        QStringList recent = settings.value("library/recentFiles").toStringList();
        recent.removeAll(sourceInfo.absoluteFilePath());
        settings.setValue("library/recentFiles", recent);
        QFile managedFile(m_currentFile);
        if (managedFile.open(QIODevice::ReadOnly)) {
            QString managedError;
            const Song managedSong = Song::fromJson(managedFile.readAll(), &managedError);
            if (managedError.isEmpty())
                song = managedSong;
        }
    }
    loadIntoEditor(song);
    setDirty(false);
    rememberLibraryFile(m_currentFile);
    refreshLibrary();
    statusBar()->showMessage(
        alreadyManaged ? tr("Song geöffnet")
                       : tr("Song als Bibliothekskopie importiert"),
        3000);
    return true;
}

void MainWindow::saveSong()
{
    if (m_currentFile.isEmpty()) {
        saveSongAs();
        return;
    }
    writeSong(m_currentFile);
}

void MainWindow::saveSongAs()
{
    QString suggested = m_titleEdit->text().trimmed();
    if (suggested.isEmpty())
        suggested = "song";
    suggested.replace(QRegularExpression(R"([^\w\- ]+)"), "");
    const QString initialPath =
        QDir(libraryDirectory()).filePath(suggested + ".ricksheet");
    const QString fileName = QFileDialog::getSaveFileName(this, tr("RickSheets-Song speichern"),
                                                          initialPath,
                                                          tr("RickSheets-Songs (*.ricksheet)"));
    if (!fileName.isEmpty() && writeSong(fileName))
        m_currentFile = fileName;
}

bool MainWindow::writeSong(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::critical(this, tr("Speichern fehlgeschlagen"), file.errorString());
        return false;
    }
    if (file.write(songFromEditor().toJson()) < 0) {
        QMessageBox::critical(this, tr("Speichern fehlgeschlagen"), file.errorString());
        return false;
    }
    setDirty(false);
    rememberLibraryFile(QFileInfo(fileName).absoluteFilePath());
    refreshLibrary();
    statusBar()->showMessage(tr("Gespeichert"), 3000);
    return true;
}

QString MainWindow::libraryDirectory() const
{
    const QString overrideDirectory = qEnvironmentVariable("RICKSHEETS_LIBRARY_DIR");
    const QString directory =
        !overrideDirectory.isEmpty()
            ? overrideDirectory
            : QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
                  .filePath("RickSheets");
    QDir().mkpath(directory);
    return directory;
}

void MainWindow::rememberLibraryFile(const QString &fileName)
{
    QSettings settings;
    QStringList files = settings.value("library/recentFiles").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > 40)
        files.removeLast();
    settings.setValue("library/recentFiles", files);
}

QString MainWindow::createLibraryCopy(const Song &song, const QString &sourceFile)
{
    QSettings settings;
    QString sourceKey;
    if (!sourceFile.isEmpty()) {
        sourceKey =
            "library/imports/" +
            QString::fromLatin1(
                QCryptographicHash::hash(
                    QFileInfo(sourceFile).absoluteFilePath().toUtf8(),
                    QCryptographicHash::Sha256)
                    .toHex());
        const QString existing = settings.value(sourceKey).toString();
        if (!existing.isEmpty() && QFileInfo::exists(existing))
            return QFileInfo(existing).absoluteFilePath();
    }

    QString baseName = song.title.trimmed();
    if (baseName.isEmpty() && !sourceFile.isEmpty())
        baseName = QFileInfo(sourceFile).completeBaseName();
    if (baseName.isEmpty())
        baseName = tr("Importierter Song");
    baseName.replace(QRegularExpression(R"([^\w\- ]+)"), "");
    baseName = baseName.simplified();
    if (baseName.isEmpty())
        baseName = "song";

    const QDir directory(libraryDirectory());
    QString destination = directory.filePath(baseName + ".ricksheet");
    for (int suffix = 2; QFileInfo::exists(destination); ++suffix)
        destination = directory.filePath(QString("%1 - %2.ricksheet").arg(baseName).arg(suffix));

    QFile copy(destination);
    if (!copy.open(QIODevice::WriteOnly | QIODevice::Truncate) ||
        copy.write(song.toJson()) < 0) {
        statusBar()->showMessage(
            tr("Bibliothekskopie konnte nicht angelegt werden: %1")
                .arg(copy.errorString()),
            6000);
        return {};
    }
    copy.close();
    if (!sourceKey.isEmpty())
        settings.setValue(sourceKey, destination);
    rememberLibraryFile(destination);
    return QFileInfo(destination).absoluteFilePath();
}

void MainWindow::refreshLibrary(const QString &filter)
{
    if (!m_library)
        return;

    const QString effectiveFilter =
        filter.isEmpty() && m_librarySearch ? m_librarySearch->text() : filter;
    QStringList files;
    const QDir directory(libraryDirectory());
    for (const QFileInfo &info :
         directory.entryInfoList({"*.ricksheet", "*.json"}, QDir::Files, QDir::Name))
        files << info.absoluteFilePath();

    const QStringList recent = QSettings().value("library/recentFiles").toStringList();
    for (const QString &fileName : recent) {
        const QFileInfo info(fileName);
        if (info.exists() && !files.contains(info.absoluteFilePath()))
            files << info.absoluteFilePath();
    }

    m_library->clear();
    for (const QString &fileName : files) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
            continue;
        QString error;
        const Song song = Song::fromJson(file.readAll(), &error);
        if (!error.isEmpty())
            continue;

        const QString searchable =
            song.title + " " + song.artist + " " + QFileInfo(fileName).completeBaseName();
        if (!effectiveFilter.trimmed().isEmpty() &&
            !searchable.contains(effectiveFilter.trimmed(), Qt::CaseInsensitive))
            continue;

        auto *item = new QListWidgetItem(song.title, m_library);
        item->setData(Qt::UserRole, fileName);
        item->setData(TitleRole, song.title);
        item->setData(ArtistRole, song.artist);
        item->setToolTip(fileName);
        if (QFileInfo(fileName) == QFileInfo(m_currentFile))
            item->setSelected(true);
    }

    if (m_library->count() == 0) {
        auto *empty = new QListWidgetItem(
            effectiveFilter.trimmed().isEmpty() ? tr("Noch keine Songs")
                                                : tr("Keine Treffer"),
            m_library);
        empty->setFlags(Qt::NoItemFlags);
    }
    if (m_deleteLibraryButton)
        m_deleteLibraryButton->setEnabled(false);
}

void MainWindow::openLibraryItem(QListWidgetItem *item)
{
    if (!item)
        return;
    const QString fileName = item->data(Qt::UserRole).toString();
    if (fileName.isEmpty() || QFileInfo(fileName) == QFileInfo(m_currentFile))
        return;
    if (!maybeSave())
        return;
    openSongFile(fileName);
}

void MainWindow::deleteLibrarySong()
{
    QListWidgetItem *item = m_library ? m_library->currentItem() : nullptr;
    if (!item)
        return;
    const QString fileName = item->data(Qt::UserRole).toString();
    if (fileName.isEmpty())
        return;

    const QFileInfo info(fileName);
    const bool managed =
        info.absolutePath() == QFileInfo(libraryDirectory()).absoluteFilePath();
    const QString question =
        managed
            ? tr("Soll „%1“ in den Papierkorb verschoben werden?")
                  .arg(item->data(TitleRole).toString())
            : tr("Soll „%1“ aus der Bibliothek entfernt werden?\n"
                 "Die externe Datei bleibt erhalten.")
                  .arg(item->data(TitleRole).toString());
    if (QMessageBox::warning(
            this, tr("Song aus Bibliothek entfernen"), question,
            QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) !=
        QMessageBox::Yes)
        return;

    if (managed) {
        QString pathInTrash;
        if (!QFile::moveToTrash(fileName, &pathInTrash)) {
            QMessageBox::critical(
                this, tr("Löschen fehlgeschlagen"),
                tr("Die Datei konnte nicht in den Papierkorb verschoben werden."));
            return;
        }
    }

    QSettings settings;
    QStringList recent = settings.value("library/recentFiles").toStringList();
    recent.removeAll(fileName);
    settings.setValue("library/recentFiles", recent);
    for (const QString &key : settings.allKeys()) {
        if (key.startsWith("library/imports/") &&
            settings.value(key).toString() == fileName)
            settings.remove(key);
    }

    if (QFileInfo(m_currentFile) == info) {
        m_currentFile.clear();
        setDirty(true);
    }
    refreshLibrary();
    statusBar()->showMessage(
        managed ? tr("Song in den Papierkorb verschoben")
                : tr("Song aus der Bibliothek entfernt"),
        4000);
}

void MainWindow::exportPdf()
{
    QString suggested = m_titleEdit->text().trimmed();
    if (suggested.isEmpty())
        suggested = "chordsheet";
    suggested.replace(QRegularExpression(R"([^\w\- ]+)"), "");
    const QString fileName = QFileDialog::getSaveFileName(this, tr("PDF exportieren"),
                                                          suggested + ".pdf", tr("PDF (*.pdf)"));
    if (fileName.isEmpty())
        return;
    updatePreview();
    QString error;
    if (!m_preview->exportPdf(fileName, &error)) {
        QMessageBox::critical(this, tr("PDF-Export fehlgeschlagen"), error);
        return;
    }
    statusBar()->showMessage(tr("PDF exportiert: %1").arg(fileName), 5000);
}

void MainWindow::transposeSong()
{
    bool ok = false;
    const int semitones = QInputDialog::getInt(
        this, tr("Song transponieren"),
        tr("Halbtonschritte (negativ = abwärts):"), 0, -11, 11, 1, &ok);
    if (!ok || semitones == 0)
        return;
    m_contentEdit->setPlainText(
        ChordParser::transposeContent(m_contentEdit->toPlainText(), semitones, true));
    if (!m_keyEdit->text().trimmed().isEmpty())
        m_keyEdit->setText(ChordParser::transposeChord(m_keyEdit->text().trimmed(), semitones, true));
    setDirty(true);
    statusBar()->showMessage(tr("Song um %1 Halbtonschritte transponiert").arg(semitones), 4000);
}

void MainWindow::editBlocks()
{
    BlockEditorDialog dialog(m_contentEdit->toPlainText(), this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    m_contentEdit->setPlainText(dialog.content());
    setDirty(true);
    schedulePreview();
}

void MainWindow::editChords()
{
    ChordEditorDialog dialog(m_contentEdit->toPlainText(), this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    m_contentEdit->setPlainText(dialog.content());
    setDirty(true);
    schedulePreview();
}

void MainWindow::schedulePreview()
{
    if (m_previewTimer)
        m_previewTimer->start();
}

void MainWindow::updatePreview()
{
    m_preview->setSong(songFromEditor());
}

void MainWindow::updatePageStatus(int pages)
{
    m_pageStatus->setText(pages == 1 ? tr("1 Seite") : tr("%1 Seiten").arg(pages));
    m_pageStatus->setStyleSheet(
        pages == 1
            ? "background:#c9e6c4; color:#171717; padding:4px 9px; border:1px solid #8fbd88; border-radius:10px; font-weight:800;"
            : "background:#f3dc72; color:#171717; padding:4px 9px; border:1px solid #c6a92c; border-radius:10px; font-weight:800;");
}

void MainWindow::loadIntoEditor(const Song &song)
{
    const QSignalBlocker b1(m_titleEdit);
    const QSignalBlocker b2(m_artistEdit);
    const QSignalBlocker b3(m_keyEdit);
    const QSignalBlocker b4(m_bpmEdit);
    const QSignalBlocker b5(m_capoEdit);
    const QSignalBlocker b6(m_contentEdit);
    m_titleEdit->setText(song.title);
    m_artistEdit->setText(song.artist);
    m_keyEdit->setText(song.key);
    m_bpmEdit->setValue(song.bpm);
    m_capoEdit->setValue(song.capo);
    m_contentEdit->setPlainText(song.content);
    updatePreview();
}

Song MainWindow::songFromEditor() const
{
    Song song;
    song.title = m_titleEdit->text().trimmed();
    song.artist = m_artistEdit->text().trimmed();
    song.key = m_keyEdit->text().trimmed();
    song.bpm = m_bpmEdit->value();
    song.capo = m_capoEdit->value();
    song.content = m_contentEdit->toPlainText();
    return song;
}

bool MainWindow::maybeSave()
{
    if (!m_dirty)
        return true;
    const auto answer = QMessageBox::question(
        this, tr("Ungespeicherte Änderungen"),
        tr("Möchtest du die Änderungen am aktuellen Song speichern?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (answer == QMessageBox::Cancel)
        return false;
    if (answer == QMessageBox::Save) {
        saveSong();
        return !m_dirty;
    }
    return true;
}

void MainWindow::setDirty(bool dirty)
{
    m_dirty = dirty;
    const QString title = m_titleEdit && !m_titleEdit->text().isEmpty()
                              ? m_titleEdit->text()
                              : tr("RickSheets");
    setWindowTitle(QString("%1%2 – RickSheets").arg(title, dirty ? " *" : ""));
}
