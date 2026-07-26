// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#include "importreviewdialog.h"
#include "appstyle.h"
#include "chordparser.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QSplitter>
#include <QStandardPaths>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTextBlock>
#include <QVBoxLayout>

namespace {
class PdfPageView final : public QWidget {
public:
    explicit PdfPageView(const QString &pdfFileName, QWidget *parent = nullptr)
        : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);

        auto *controls = new QHBoxLayout;
        m_previous = new QPushButton(tr("‹ Vorherige"));
        m_previous->setObjectName("pdfPreviousPage");
        m_next = new QPushButton(tr("Nächste ›"));
        m_next->setObjectName("pdfNextPage");
        m_pageLabel = new QLabel;
        m_pageLabel->setObjectName("pdfPageNumber");
        auto *zoomOut = new QPushButton(tr("−"));
        zoomOut->setObjectName("pdfZoomOut");
        auto *zoomIn = new QPushButton(tr("+"));
        zoomIn->setObjectName("pdfZoomIn");
        zoomOut->setToolTip(tr("PDF-Seite verkleinern"));
        zoomIn->setToolTip(tr("PDF-Seite vergrößern"));
        controls->addWidget(m_previous);
        controls->addWidget(m_next);
        controls->addWidget(m_pageLabel);
        controls->addStretch();
        controls->addWidget(zoomOut);
        controls->addWidget(zoomIn);
        layout->addLayout(controls);

        auto *scroll = new QScrollArea;
        scroll->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        scroll->setWidgetResizable(false);
        m_image = new QLabel;
        m_image->setObjectName("pdfPageImage");
        m_image->setAlignment(Qt::AlignCenter);
        scroll->setWidget(m_image);
        layout->addWidget(scroll, 1);

        connect(m_previous, &QPushButton::clicked, this, [this] {
            if (m_pageIndex > 0) {
                --m_pageIndex;
                updatePage();
            }
        });
        connect(m_next, &QPushButton::clicked, this, [this] {
            if (m_pageIndex + 1 < m_pages.size()) {
                ++m_pageIndex;
                updatePage();
            }
        });
        connect(zoomOut, &QPushButton::clicked, this, [this] {
            m_zoom = qMax(0.3, m_zoom - 0.1);
            updatePage();
        });
        connect(zoomIn, &QPushButton::clicked, this, [this] {
            m_zoom = qMin(2.0, m_zoom + 0.1);
            updatePage();
        });

        render(pdfFileName);
    }

private:
    void render(const QString &pdfFileName)
    {
        const QString renderer = QStandardPaths::findExecutable("pdftoppm");
        if (renderer.isEmpty() || !m_temporaryDirectory.isValid()) {
            showError(tr("PDF-Seitenvorschau ist nicht verfügbar."));
            return;
        }

        const QString prefix =
            QDir(m_temporaryDirectory.path()).filePath("page");
        QProcess process;
        process.start(renderer, {"-png", "-r", "110", pdfFileName, prefix});
        if (!process.waitForStarted(3000) ||
            !process.waitForFinished(30000) ||
            process.exitStatus() != QProcess::NormalExit ||
            process.exitCode() != 0) {
            showError(
                tr("Die PDF-Seiten konnten nicht gerendert werden.\n%1")
                    .arg(QString::fromUtf8(process.readAllStandardError())));
            return;
        }

        const QFileInfoList files =
            QDir(m_temporaryDirectory.path())
                .entryInfoList({"page-*.png"}, QDir::Files, QDir::Name);
        for (const QFileInfo &file : files) {
            const QPixmap page(file.absoluteFilePath());
            if (!page.isNull())
                m_pages << page;
        }
        if (m_pages.isEmpty()) {
            showError(tr("Die PDF enthält keine darstellbaren Seiten."));
            return;
        }
        updatePage();
    }

    void showError(const QString &message)
    {
        m_image->setText(message);
        m_image->setWordWrap(true);
        m_image->setMinimumSize(420, 180);
        m_previous->setEnabled(false);
        m_next->setEnabled(false);
        m_pageLabel->setText(tr("Keine Vorschau"));
    }

    void updatePage()
    {
        if (m_pages.isEmpty())
            return;
        const QPixmap &page = m_pages.at(m_pageIndex);
        m_image->setPixmap(
            page.scaled(page.size() * m_zoom, Qt::KeepAspectRatio,
                        Qt::SmoothTransformation));
        m_image->adjustSize();
        m_pageLabel->setText(
            tr("Seite %1 von %2").arg(m_pageIndex + 1).arg(m_pages.size()));
        m_previous->setEnabled(m_pageIndex > 0);
        m_next->setEnabled(m_pageIndex + 1 < m_pages.size());
    }

    QTemporaryDir m_temporaryDirectory;
    QList<QPixmap> m_pages;
    QLabel *m_image = nullptr;
    QLabel *m_pageLabel = nullptr;
    QPushButton *m_previous = nullptr;
    QPushButton *m_next = nullptr;
    int m_pageIndex = 0;
    qreal m_zoom = 0.6;
};
}

ImportReviewDialog::ImportReviewDialog(const QString &sourceText,
                                       const Song &detectedSong,
                                       const QString &sourceDescription,
                                       QWidget *parent,
                                       const QString &pdfFileName)
    : QDialog(parent)
{
    qApp->setStyleSheet(rickSheetsStyleSheet());
    setWindowTitle(tr("Import prüfen"));
    resize(1180, 760);
    setMinimumSize(880, 580);

    auto *layout = new QVBoxLayout(this);
    auto *intro = new QLabel(
        tr("<b>Bitte vergleiche Quelle und Erkennung.</b> "
           "Du kannst Songdaten und Arrangement rechts vor der Übernahme korrigieren."));
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto *splitter = new QSplitter;

    auto *sourcePane = new QWidget;
    auto *sourceLayout = new QVBoxLayout(sourcePane);
    sourceLayout->setContentsMargins(0, 0, 6, 0);
    auto *sourceLabel = new QLabel(sourceDescription);
    sourceLabel->setObjectName("dialogHeading");
    QFont headingFont = sourceLabel->font();
    headingFont.setBold(true);
    sourceLabel->setFont(headingFont);
    auto *sourceEdit = new QPlainTextEdit;
    sourceEdit->setObjectName("importSourceText");
    sourceEdit->setPlainText(sourceText);
    sourceEdit->setReadOnly(true);
    sourceEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    sourceLayout->addWidget(sourceLabel);
    if (pdfFileName.isEmpty()) {
        sourceLayout->addWidget(sourceEdit, 1);
    } else {
        auto *sourceTabs = new QTabWidget;
        sourceTabs->setObjectName("importSourceTabs");
        sourceTabs->addTab(new PdfPageView(pdfFileName, sourceTabs),
                           tr("PDF-Seite"));
        sourceTabs->addTab(sourceEdit, tr("Extrahierter Text"));
        sourceLayout->addWidget(sourceTabs, 1);
    }
    splitter->addWidget(sourcePane);

    auto *resultPane = new QWidget;
    auto *resultLayout = new QVBoxLayout(resultPane);
    resultLayout->setContentsMargins(6, 0, 0, 0);
    auto *resultLabel = new QLabel(tr("ERKANNTES RICKSHEET – BEARBEITBAR"));
    resultLabel->setObjectName("dialogHeading");
    resultLabel->setFont(headingFont);
    auto *form = new QFormLayout;
    m_titleEdit = new QLineEdit(detectedSong.title);
    m_titleEdit->setObjectName("importReviewTitle");
    m_artistEdit = new QLineEdit(detectedSong.artist);
    m_artistEdit->setObjectName("importReviewArtist");
    m_keyEdit = new QLineEdit(detectedSong.key);
    m_keyEdit->setMaximumWidth(100);
    m_bpmEdit = new QSpinBox;
    m_bpmEdit->setRange(0, 300);
    m_bpmEdit->setSpecialValueText("–");
    m_bpmEdit->setValue(detectedSong.bpm);
    m_capoEdit = new QSpinBox;
    m_capoEdit->setRange(0, 12);
    m_capoEdit->setSpecialValueText(tr("kein"));
    m_capoEdit->setValue(detectedSong.capo);
    form->addRow(tr("Titel"), m_titleEdit);
    form->addRow(tr("Interpret"), m_artistEdit);
    form->addRow(tr("Tonart"), m_keyEdit);
    form->addRow(tr("BPM"), m_bpmEdit);
    form->addRow(tr("Capo"), m_capoEdit);

    m_resultEdit = new QPlainTextEdit;
    m_resultEdit->setObjectName("importReviewResult");
    m_resultEdit->setPlainText(detectedSong.content);
    m_resultEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    QFont mono("DejaVu Sans Mono");
    mono.setStyleHint(QFont::Monospace);
    m_resultEdit->setFont(mono);

    resultLayout->addWidget(resultLabel);
    resultLayout->addLayout(form);
    resultLayout->addWidget(m_resultEdit, 1);
    splitter->addWidget(resultPane);
    splitter->setSizes({540, 640});
    layout->addWidget(splitter, 1);

    const QList<ImportWarning> warnings =
        ChordParser::analyzeImport(detectedSong);
    auto *warningList = new QListWidget;
    warningList->setObjectName("importWarnings");
    warningList->setMaximumHeight(105);
    if (warnings.isEmpty()) {
        auto *item = new QListWidgetItem(
            tr("Keine konkreten Auffälligkeiten erkannt."), warningList);
        item->setFlags(Qt::NoItemFlags);
    } else {
        for (const ImportWarning &warning : warnings) {
            auto *item = new QListWidgetItem(
                warning.line >= 0
                    ? tr("Zeile %1: %2").arg(warning.line + 1).arg(warning.message)
                    : warning.message,
                warningList);
            item->setData(Qt::UserRole, warning.line);
        }

        QList<QTextEdit::ExtraSelection> selections;
        for (const ImportWarning &warning : warnings) {
            if (warning.line < 0)
                continue;
            QTextBlock block =
                m_resultEdit->document()->findBlockByNumber(warning.line);
            if (!block.isValid())
                continue;
            QTextEdit::ExtraSelection selection;
            selection.cursor = QTextCursor(block);
            selection.cursor.select(QTextCursor::LineUnderCursor);
            selection.format.setBackground(QColor("#f3dc72"));
            selection.format.setForeground(QColor("#171717"));
            selections << selection;
        }
        m_resultEdit->setExtraSelections(selections);
    }
    connect(warningList, &QListWidget::itemActivated, this,
            [this](QListWidgetItem *item) {
                const int line = item->data(Qt::UserRole).toInt();
                if (line < 0)
                    return;
                QTextBlock block =
                    m_resultEdit->document()->findBlockByNumber(line);
                if (!block.isValid())
                    return;
                QTextCursor cursor(block);
                cursor.select(QTextCursor::LineUnderCursor);
                m_resultEdit->setTextCursor(cursor);
                m_resultEdit->setFocus();
                m_resultEdit->centerCursor();
            });

    auto *warningLabel = new QLabel(
        warnings.isEmpty()
            ? tr("IMPORT-HINWEISE")
            : tr("IMPORT-HINWEISE (%1)").arg(warnings.size()));
    warningLabel->setObjectName("dialogHeading");
    warningLabel->setFont(headingFont);
    layout->addWidget(warningLabel);
    layout->addWidget(warningList);

    auto *note = new QLabel(
        pdfFileName.isEmpty()
            ? tr("Die Quelle bleibt unverändert; bereinigt wird ausschließlich "
                 "das erkannte RickSheet auf der rechten Seite.")
            : tr("Bei PDF-Dateien kannst du zwischen der gerenderten "
                 "Originalseite und dem extrahierten Text wechseln."));
    note->setObjectName("importReviewNote");
    note->setWordWrap(true);
    layout->addWidget(note);

    auto *buttons =
        new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Geprüft übernehmen"));
    buttons->button(QDialogButtonBox::Ok)->setProperty("primary", true);
    buttons->button(QDialogButtonBox::Cancel)->setProperty("quiet", true);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

Song ImportReviewDialog::reviewedSong() const
{
    Song song;
    song.title = m_titleEdit->text().trimmed();
    song.artist = m_artistEdit->text().trimmed();
    song.key = m_keyEdit->text().trimmed();
    song.bpm = m_bpmEdit->value();
    song.capo = m_capoEdit->value();
    song.content = m_resultEdit->toPlainText();
    return song;
}
