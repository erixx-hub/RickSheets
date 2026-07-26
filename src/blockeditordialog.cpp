// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#include "blockeditordialog.h"
#include "appstyle.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

namespace {
constexpr int BodyRole = Qt::UserRole + 1;
constexpr int HasHeaderRole = Qt::UserRole + 2;
}

BlockEditorDialog::BlockEditorDialog(const QString &content, QWidget *parent)
    : QDialog(parent), m_originalContent(content)
{
    qApp->setStyleSheet(rickSheetsStyleSheet());
    setWindowTitle(tr("Abschnitte bearbeiten"));
    resize(900, 650);
    setMinimumSize(720, 500);

    auto *layout = new QVBoxLayout(this);
    auto *intro = new QLabel(
        tr("Abschnitte per Drag-and-drop sortieren. Name und Inhalt des "
           "ausgewählten Abschnitts können rechts bearbeitet werden."));
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto *columns = new QHBoxLayout;
    auto *left = new QVBoxLayout;
    auto *sectionLabel = new QLabel(tr("ABSCHNITTE"));
    sectionLabel->setObjectName("dialogHeading");
    QFont heading = sectionLabel->font();
    heading.setBold(true);
    sectionLabel->setFont(heading);
    m_sections = new QListWidget;
    m_sections->setObjectName("blockEditorSections");
    m_sections->setDragDropMode(QAbstractItemView::InternalMove);
    m_sections->setDefaultDropAction(Qt::MoveAction);
    auto *addButton = new QPushButton(tr("+ Abschnitt"));
    addButton->setObjectName("blockEditorAdd");
    m_deleteButton = new QPushButton(tr("Abschnitt löschen"));
    m_deleteButton->setObjectName("blockEditorDelete");
    left->addWidget(sectionLabel);
    left->addWidget(m_sections, 1);
    left->addWidget(addButton);
    left->addWidget(m_deleteButton);
    columns->addLayout(left, 1);

    auto *right = new QVBoxLayout;
    auto *nameLabel = new QLabel(tr("ABSCHNITTSNAME"));
    nameLabel->setObjectName("dialogHeading");
    nameLabel->setFont(heading);
    m_nameEdit = new QLineEdit;
    m_nameEdit->setObjectName("blockEditorName");
    auto *bodyLabel = new QLabel(tr("INHALT"));
    bodyLabel->setObjectName("dialogHeading");
    bodyLabel->setFont(heading);
    m_bodyEdit = new QPlainTextEdit;
    m_bodyEdit->setObjectName("blockEditorBody");
    m_bodyEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    QFont mono("DejaVu Sans Mono");
    mono.setStyleHint(QFont::Monospace);
    m_bodyEdit->setFont(mono);
    right->addWidget(nameLabel);
    right->addWidget(m_nameEdit);
    right->addWidget(bodyLabel);
    right->addWidget(m_bodyEdit, 1);
    columns->addLayout(right, 2);
    layout->addLayout(columns, 1);

    static const QRegularExpression sectionLine(
        R"(^\s*\[([^\]]+)\]\s*$)");
    QString currentName;
    QStringList currentBody;
    bool currentHasHeader = false;
    const auto flush = [&] {
        if (currentHasHeader || !currentBody.join('\n').trimmed().isEmpty())
            addBlock(currentName, currentBody.join('\n').trimmed(),
                     currentHasHeader);
        currentBody.clear();
    };
    for (const QString &line : content.split('\n')) {
        const auto match = sectionLine.match(line);
        if (match.hasMatch()) {
            flush();
            currentName = match.captured(1).trimmed();
            currentHasHeader = true;
        } else {
            currentBody << line;
        }
    }
    flush();
    if (m_sections->count() == 0)
        addBlock(tr("Neuer Abschnitt"), QString(), true);

    connect(m_sections, &QListWidget::currentRowChanged,
            this, &BlockEditorDialog::updateSelection);
    connect(m_nameEdit, &QLineEdit::textChanged, this, [this](const QString &name) {
        if (m_updating || !m_sections->currentItem())
            return;
        m_modified = true;
        const QString display = name.trimmed().isEmpty()
                                    ? tr("Unbenannter Abschnitt")
                                    : name.trimmed();
        m_sections->currentItem()->setText(display);
    });
    connect(m_bodyEdit, &QPlainTextEdit::textChanged, this, [this] {
        if (!m_updating && m_sections->currentItem())
        {
            m_modified = true;
            m_sections->currentItem()->setData(
                BodyRole, m_bodyEdit->toPlainText());
        }
    });
    connect(addButton, &QPushButton::clicked, this, &BlockEditorDialog::addSection);
    connect(m_deleteButton, &QPushButton::clicked,
            this, &BlockEditorDialog::deleteSection);
    connect(m_sections->model(), &QAbstractItemModel::rowsMoved,
            this, [this] { m_modified = true; });

    auto *buttons =
        new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Abschnitte übernehmen"));
    buttons->button(QDialogButtonBox::Ok)->setProperty("primary", true);
    buttons->button(QDialogButtonBox::Cancel)->setProperty("quiet", true);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    m_sections->setCurrentRow(0);
}

void BlockEditorDialog::addBlock(const QString &name, const QString &body,
                                 bool hasHeader)
{
    auto *item = new QListWidgetItem(
        name.trimmed().isEmpty() ? tr("Text vor erstem Abschnitt") : name,
        m_sections);
    item->setData(BodyRole, body);
    item->setData(HasHeaderRole, hasHeader);
}

void BlockEditorDialog::updateSelection()
{
    m_updating = true;
    const QListWidgetItem *item = m_sections->currentItem();
    const bool hasItem = item != nullptr;
    const bool hasHeader = hasItem && item->data(HasHeaderRole).toBool();
    m_nameEdit->setEnabled(hasHeader);
    m_nameEdit->setText(hasHeader ? item->text() : QString());
    m_bodyEdit->setEnabled(hasItem);
    m_bodyEdit->setPlainText(hasItem ? item->data(BodyRole).toString()
                                     : QString());
    m_deleteButton->setEnabled(hasItem);
    m_updating = false;
}

void BlockEditorDialog::addSection()
{
    m_modified = true;
    addBlock(tr("Neuer Abschnitt"), QString(), true);
    m_sections->setCurrentRow(m_sections->count() - 1);
    m_nameEdit->selectAll();
    m_nameEdit->setFocus();
}

void BlockEditorDialog::deleteSection()
{
    const int row = m_sections->currentRow();
    if (row < 0)
        return;
    m_modified = true;
    delete m_sections->takeItem(row);
    if (m_sections->count() == 0)
        addBlock(tr("Neuer Abschnitt"), QString(), true);
    m_sections->setCurrentRow(qMin(row, m_sections->count() - 1));
}

QString BlockEditorDialog::content() const
{
    if (!m_modified)
        return m_originalContent;
    QStringList blocks;
    for (int row = 0; row < m_sections->count(); ++row) {
        const QListWidgetItem *item = m_sections->item(row);
        QStringList lines;
        if (item->data(HasHeaderRole).toBool())
            lines << "[" + item->text().trimmed() + "]";
        const QString body = item->data(BodyRole).toString().trimmed();
        if (!body.isEmpty())
            lines << body;
        blocks << lines.join('\n');
    }
    return blocks.join("\n\n").trimmed();
}
