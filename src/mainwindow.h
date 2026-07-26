// SPDX-FileCopyrightText: 2026 Erik Heidenreich
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "song.h"

#include <QMainWindow>

class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QCloseEvent;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QTimer;
class PreviewWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    bool openDocument(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newSong();
    void importText();
    void importPdf();
    void openSong();
    void saveSong();
    void saveSongAs();
    void exportPdf();
    void transposeSong();
    void editBlocks();
    void editChords();
    void schedulePreview();
    void updatePreview();
    void updatePageStatus(int pages);
    void openLibraryItem(QListWidgetItem *item);
    void setSystemTheme();
    void setLightTheme();
    void setDarkTheme();
    void setSystemLanguage();
    void setGermanLanguage();
    void setEnglishLanguage();
    void deleteLibrarySong();

private:
    void buildUi();
    void buildMenus();
    void loadIntoEditor(const Song &song);
    Song songFromEditor() const;
    bool writeSong(const QString &fileName);
    bool openSongFile(const QString &fileName);
    bool maybeSave();
    void setDirty(bool dirty);
    void refreshLibrary(const QString &filter = {});
    void rememberLibraryFile(const QString &fileName);
    QString createLibraryCopy(const Song &song, const QString &sourceFile = {});
    QString libraryDirectory() const;
    void applyTheme(const QString &theme);
    void chooseLanguage(const QString &language);
    void retranslateUi();

    QListWidget *m_library = nullptr;
    QLineEdit *m_librarySearch = nullptr;
    QPushButton *m_deleteLibraryButton = nullptr;
    QLabel *m_brand = nullptr;
    QLineEdit *m_titleEdit = nullptr;
    QLineEdit *m_artistEdit = nullptr;
    QLineEdit *m_keyEdit = nullptr;
    QSpinBox *m_bpmEdit = nullptr;
    QSpinBox *m_capoEdit = nullptr;
    QPlainTextEdit *m_contentEdit = nullptr;
    PreviewWidget *m_preview = nullptr;
    QLabel *m_pageStatus = nullptr;
    QTimer *m_previewTimer = nullptr;
    int m_pageCount = 1;
    QString m_currentFile;
    bool m_dirty = false;
};
