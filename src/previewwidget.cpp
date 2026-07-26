// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewwidget.h"
#include "renderer.h"

#include <QAbstractTextDocumentLayout>
#include <QFileInfo>
#include <QPainter>
#include <QPdfWriter>
#include <QPixmap>

namespace {
void drawRickSheetsMark(QPainter &painter, const QSizeF &page, int pageIndex)
{
    QFont font("DejaVu Sans");
    font.setPointSizeF(6.5);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 0.25);

    painter.save();
    painter.setFont(font);
    painter.setOpacity(0.32);
    painter.setPen(QColor("#777777"));
    const qreal pageTop = pageIndex * page.height();
    const QRectF footer(23.0, pageTop + page.height() - 18.0,
                        page.width() - 46.0, 10.0);
    const QString label = QStringLiteral("Erstellt mit RickSheets");
    const qreal textWidth = painter.fontMetrics().horizontalAdvance(label);
    const QRectF markRect(footer.right() - textWidth - 11.0, footer.top() - 1.0,
                          7.0, 11.0);
    const QPixmap mark(":/brand/ricksheets-favicon.svg");
    painter.drawPixmap(markRect, mark, mark.rect());
    painter.drawText(footer, Qt::AlignRight | Qt::AlignVCenter, label);
    painter.restore();
}
}

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(true);
    auto palette = this->palette();
    palette.setColor(QPalette::Window, QColor("#d8d8d5"));
    setPalette(palette);
}

void PreviewWidget::setSong(const Song &song)
{
    SongRenderer::configureDocument(m_document, song);
    const int pages = SongRenderer::pageCount(m_document);
    if (pages != m_pageCount) {
        m_pageCount = pages;
        emit pageCountChanged(pages);
    }
    updateGeometryForDocument();
    update();
}

int PreviewWidget::pageCount() const
{
    return m_pageCount;
}

void PreviewWidget::updateGeometryForDocument()
{
    const QSizeF page = m_document.pageSize();
    const int width = qCeil(page.width() * m_scale) + 32;
    const int height = qCeil(page.height() * m_scale) * m_pageCount + m_gap * (m_pageCount + 1);
    setMinimumSize(width, height);
    resize(width, height);
}

void PreviewWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QSizeF page = m_document.pageSize();
    const int scaledWidth = qCeil(page.width() * m_scale);
    const int scaledHeight = qCeil(page.height() * m_scale);
    const int left = qMax(16, (width() - scaledWidth) / 2);

    for (int pageIndex = 0; pageIndex < m_pageCount; ++pageIndex) {
        const int top = m_gap + pageIndex * (scaledHeight + m_gap);
        painter.fillRect(QRect(left + 4, top + 5, scaledWidth, scaledHeight), QColor(0, 0, 0, 35));
        painter.fillRect(QRect(left, top, scaledWidth, scaledHeight), Qt::white);

        painter.save();
        painter.setClipRect(QRect(left, top, scaledWidth, scaledHeight));
        painter.translate(left, top - pageIndex * page.height() * m_scale);
        painter.scale(m_scale, m_scale);
        QAbstractTextDocumentLayout::PaintContext context;
        context.clip = QRectF(0, pageIndex * page.height(), page.width(), page.height());
        m_document.documentLayout()->draw(&painter, context);
        drawRickSheetsMark(painter, page, pageIndex);
        painter.restore();
    }
}

bool PreviewWidget::exportPdf(const QString &fileName, QString *error)
{
    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(96);
    writer.setTitle("RickSheets");
    writer.setCreator("RickSheets");
    writer.setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Point);

    QPainter painter(&writer);
    if (!painter.isActive()) {
        if (error)
            *error = tr("Die PDF-Datei konnte nicht geöffnet werden.");
        return false;
    }

    const QSizeF documentPage = m_document.pageSize();
    const QRect paintRect = writer.pageLayout().paintRectPixels(writer.resolution());
    const qreal scale = qMin(paintRect.width() / documentPage.width(),
                             paintRect.height() / documentPage.height());

    for (int pageIndex = 0; pageIndex < m_pageCount; ++pageIndex) {
        if (pageIndex > 0)
            writer.newPage();
        painter.save();
        painter.scale(scale, scale);
        painter.translate(0, -pageIndex * documentPage.height());
        QAbstractTextDocumentLayout::PaintContext context;
        context.clip = QRectF(0, pageIndex * documentPage.height(),
                              documentPage.width(), documentPage.height());
        m_document.documentLayout()->draw(&painter, context);
        drawRickSheetsMark(painter, documentPage, pageIndex);
        painter.restore();
    }
    painter.end();

    if (!QFileInfo::exists(fileName) || QFileInfo(fileName).size() == 0) {
        if (error)
            *error = tr("Die erzeugte PDF-Datei ist leer.");
        return false;
    }
    return true;
}
