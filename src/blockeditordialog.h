// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDialog>

class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QPushButton;

class BlockEditorDialog final : public QDialog {
    Q_OBJECT
public:
    explicit BlockEditorDialog(const QString &content, QWidget *parent = nullptr);

    QString content() const;

private:
    void addBlock(const QString &name, const QString &body, bool hasHeader);
    void updateSelection();
    void addSection();
    void deleteSection();

    QListWidget *m_sections = nullptr;
    QLineEdit *m_nameEdit = nullptr;
    QPlainTextEdit *m_bodyEdit = nullptr;
    QPushButton *m_deleteButton = nullptr;
    QString m_originalContent;
    bool m_modified = false;
    bool m_updating = false;
};
