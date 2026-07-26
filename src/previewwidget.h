// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "song.h"

#include <QTextDocument>
#include <QWidget>

class PreviewWidget : public QWidget {
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = nullptr);
    void setSong(const Song &song);
    int pageCount() const;
    bool exportPdf(const QString &fileName, QString *error = nullptr);

signals:
    void pageCountChanged(int pages);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void updateGeometryForDocument();

    QTextDocument m_document;
    int m_pageCount = 1;
    qreal m_scale = 1.0;
    int m_gap = 22;
};
