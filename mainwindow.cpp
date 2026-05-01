#include <QDebug>
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QFont>
#include <QFontDatabase>
#include <QCloseEvent>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QInputDialog>
#include <QFileInfo>
#include <QToolButton>
#include <QColorDialog>
#include <QLocale>
#include <QDir>
#include <QPrinter>
#include <QApplication>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QTextCodec>
#include <QTextStream>

/**
 * @brief Конструктор главного окна
 * @param parent Родительский виджет (по умолчанию nullptr)
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_previewEditor(nullptr)
    , m_markdownEditor(nullptr)
    , m_currentEditor(nullptr)
    , m_parser(new MarkdownParser())
    , m_toolBar(nullptr)
    , m_statusBar(nullptr)
    , m_wysiwygAction(nullptr)
    , m_markdownAction(nullptr)
    , m_isWysiwygMode(false)
    , m_isModified(false)
    , m_spellChecker(nullptr)
    , m_translator(new QTranslator(this))
    , m_currentLanguage("system")
{
    // Инициализация проверки орфографии с путями к словарям
    initSpellChecker();
    
    // Загружаем системный язык при запуске
    loadTranslations("system");
    
    setWindowTitle(tr("Untitled.md"));
    resize(1200, 800);
    
    initUI();
    createToolBar();
    createMenuBar();
    createStatusBar();
    applyPreviewStyles();
    
    // Переключаемся в режим Markdown по умолчанию
    toggleMarkdownMode();
}

/**
 * @brief Деструктор класса MainWindow
 */
MainWindow::~MainWindow()
{
    delete m_parser;
    delete m_spellChecker;
}

/**
 * @brief Инициализация пользовательского интерфейса
 */
void MainWindow::initUI()
{
    // Создаем центральный виджет
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Создаем основной layout
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Создаем сплиттер для разделения экрана
    QSplitter* splitter = new QSplitter(Qt::Horizontal, centralWidget);
    
    // Создаем редактор Markdown (текстовый)
    m_markdownEditor = new QPlainTextEdit(splitter);
    m_markdownEditor->setPlaceholderText("Введите текст в формате Markdown...");
    QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    monoFont.setPointSize(12);
    m_markdownEditor->setFont(monoFont);
    m_markdownEditor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_markdownEditor, &QPlainTextEdit::customContextMenuRequested, this, &MainWindow::showEditorContextMenu);
    
    // Создаем редактор предпросмотра (WYSIWYG)
    m_previewEditor = new QTextEdit(splitter);
    m_previewEditor->setReadOnly(true);
    m_previewEditor->setPlaceholderText("Предпросмотр будет здесь...");
    m_previewEditor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_previewEditor, &QTextEdit::customContextMenuRequested, this, &MainWindow::showEditorContextMenu);
    
    // Устанавливаем размеры панелей сплиттера (50/50)
    splitter->setSizes(QList<int>() << 600 << 600);
    
    // Добавляем сплиттер в основной layout
    mainLayout->addWidget(splitter);
    
    // Сохраняем текущий редактор
    m_currentEditor = m_markdownEditor;
    
    // Подключаем сигнал изменения текста
    connect(m_markdownEditor, &QPlainTextEdit::textChanged, this, &MainWindow::onTextChanged);
}

/**
 * @brief Создание панели инструментов
 */
void MainWindow::createToolBar()
{
    m_toolBar = addToolBar("Formatting");
    m_toolBar->setMovable(false);
    
    // Кнопки форматирования текста
    QAction* boldAction = m_toolBar->addAction("B");
    boldAction->setToolTip("Жирный текст (Ctrl+B)");
    QFont boldFont = boldAction->font();
    boldFont.setBold(true);
    boldFont.setPointSize(12);
    boldAction->setFont(boldFont);
    connect(boldAction, &QAction::triggered, this, &MainWindow::insertBold);
    
    QAction* italicAction = m_toolBar->addAction("I");
    italicAction->setToolTip("Курсив (Ctrl+I)");
    QFont italicFont = italicAction->font();
    italicFont.setItalic(true);
    italicFont.setPointSize(12);
    italicAction->setFont(italicFont);
    connect(italicAction, &QAction::triggered, this, &MainWindow::insertItalic);
    
    QAction* strikeAction = m_toolBar->addAction("S");
    strikeAction->setToolTip("Зачеркнутый текст");
    QFont strikeFont = strikeAction->font();
    strikeFont.setStrikeOut(true);
    strikeFont.setPointSize(12);
    strikeAction->setFont(strikeFont);
    connect(strikeAction, &QAction::triggered, this, &MainWindow::insertStrikeThrough);
    
    m_toolBar->addSeparator();
    
    // Кнопки заголовков
    QAction* h1Action = m_toolBar->addAction("H1");
    h1Action->setToolTip("Заголовок 1 уровня");
    h1Action->setFont(QFont(h1Action->font().family(), 12, QFont::Bold));
    connect(h1Action, &QAction::triggered, this, &MainWindow::insertHeader1);
    
    QAction* h2Action = m_toolBar->addAction("H2");
    h2Action->setToolTip("Заголовок 2 уровня");
    h2Action->setFont(QFont(h2Action->font().family(), 11, QFont::Bold));
    connect(h2Action, &QAction::triggered, this, &MainWindow::insertHeader2);
    
    QAction* h3Action = m_toolBar->addAction("H3");
    h3Action->setToolTip("Заголовок 3 уровня");
    h3Action->setFont(QFont(h3Action->font().family(), 10, QFont::Bold));
    connect(h3Action, &QAction::triggered, this, &MainWindow::insertHeader3);
    
    m_toolBar->addSeparator();
    
    // Кнопки списков
    QAction* bulletAction = m_toolBar->addAction("• Список");
    bulletAction->setToolTip("Маркированный список");
    connect(bulletAction, &QAction::triggered, this, &MainWindow::insertBulletList);
    
    QAction* numberAction = m_toolBar->addAction("1. Список");
    numberAction->setToolTip("Нумерованный список");
    connect(numberAction, &QAction::triggered, this, &MainWindow::insertNumberedList);
    
    m_toolBar->addSeparator();
    
    // Кнопки остальных элементов
    QAction* quoteAction = m_toolBar->addAction("Цитата");
    quoteAction->setToolTip("Цитата");
    connect(quoteAction, &QAction::triggered, this, &MainWindow::insertBlockquote);
    
    QAction* codeAction = m_toolBar->addAction("Код");
    codeAction->setToolTip("Код");
    connect(codeAction, &QAction::triggered, this, &MainWindow::insertCode);
    
    QAction* linkAction = m_toolBar->addAction("Ссылка");
    linkAction->setToolTip("Вставить ссылку");
    connect(linkAction, &QAction::triggered, this, &MainWindow::insertLink);
    
    QAction* imageAction = m_toolBar->addAction("Изображение");
    imageAction->setToolTip("Вставить изображение");
    connect(imageAction, &QAction::triggered, this, &MainWindow::insertImage);
    
    QAction* hrAction = m_toolBar->addAction("Линия");
    hrAction->setToolTip("Горизонтальная линия");
    connect(hrAction, &QAction::triggered, this, &MainWindow::insertHorizontalRule);
    
    m_toolBar->addSeparator();
    
    // Кнопка таблицы
    QAction* tableAction = m_toolBar->addAction("Таблица");
    tableAction->setToolTip("Вставить таблицу");
    connect(tableAction, &QAction::triggered, this, &MainWindow::insertTable);
    
    // Создаем меню для операций с таблицей
    QMenu* tableMenu = new QMenu(m_toolBar);
    tableMenu->setTitle("▼");
    tableMenu->setToolTip("Операции с таблицей");
    
    QAction* insertRowAction = tableMenu->addAction("Вставить строку");
    insertRowAction->setToolTip("Вставить строку в таблицу");
    connect(insertRowAction, &QAction::triggered, this, &MainWindow::insertTableRow);
    
    QAction* insertColAction = tableMenu->addAction("Вставить столбец");
    insertColAction->setToolTip("Вставить столбец в таблицу");
    connect(insertColAction, &QAction::triggered, this, &MainWindow::insertTableColumn);
    
    tableMenu->addSeparator();
    
    QAction* deleteRowAction = tableMenu->addAction("Удалить строку");
    deleteRowAction->setToolTip("Удалить строку из таблицы");
    connect(deleteRowAction, &QAction::triggered, this, &MainWindow::deleteTableRow);
    
    QAction* deleteColAction = tableMenu->addAction("Удалить столбец");
    deleteColAction->setToolTip("Удалить столбец из таблицы");
    connect(deleteColAction, &QAction::triggered, this, &MainWindow::deleteTableColumn);
    
    QWidget* tableWidget = new QWidget(m_toolBar);
    QHBoxLayout* tableLayout = new QHBoxLayout(tableWidget);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    tableLayout->setSpacing(2);
    tableLayout->addWidget(new QPushButton("Таблица"));
    QToolButton* tableToolBtn = new QToolButton();
    tableToolBtn->setMenu(tableMenu);
    tableToolBtn->setPopupMode(QToolButton::InstantPopup);
    tableToolBtn->setText("▼");
    tableLayout->addWidget(tableToolBtn);
    
    // Заменяем действие таблицы на виджет с меню
    m_toolBar->removeAction(tableAction);
    m_toolBar->addWidget(tableWidget);
    
    // Кнопка блока кода
    QAction* codeBlockAction = m_toolBar->addAction("Блок кода");
    codeBlockAction->setToolTip("Вставить многострочный блок кода");
    connect(codeBlockAction, &QAction::triggered, this, &MainWindow::insertCodeBlock);
    
    // Кнопка задачи
    QAction* taskAction = m_toolBar->addAction("Задача");
    taskAction->setToolTip("Вставить задачу (checkbox)");
    connect(taskAction, &QAction::triggered, this, &MainWindow::insertTask);
    
    // Кнопка спецсимволов
    QAction* specialAction = m_toolBar->addAction("Спецсимволы");
    specialAction->setToolTip("Вставить специальные символы");
    connect(specialAction, &QAction::triggered, this, &MainWindow::insertSpecialCharacter);
    
    m_toolBar->addSeparator();
    
    // Кнопка проверки орфографии
    QAction* spellCheckAction = m_toolBar->addAction("ABC ✓");
    spellCheckAction->setToolTip("Проверить орфографию (F7)");
    spellCheckAction->setFont(QFont(spellCheckAction->font().family(), 10, QFont::Bold));
    connect(spellCheckAction, &QAction::triggered, this, &MainWindow::checkSpelling);
    
    m_toolBar->addSeparator();
    
    // Переключатели режимов
    m_wysiwygAction = m_toolBar->addAction("WYSIWYG");
    m_wysiwygAction->setToolTip("Режим визуального редактирования");
    m_wysiwygAction->setCheckable(true);
    connect(m_wysiwygAction, &QAction::triggered, this, &MainWindow::toggleWysiwygMode);
    
    m_markdownAction = m_toolBar->addAction("Markdown");
    m_markdownAction->setToolTip("Режим редактирования Markdown");
    m_markdownAction->setCheckable(true);
    m_markdownAction->setChecked(true);
    connect(m_markdownAction, &QAction::triggered, this, &MainWindow::toggleMarkdownMode);
}

/**
 * @brief Создание меню
 */
void MainWindow::createMenuBar()
{
    // Очищаем существующее меню перед созданием нового (чтобы избежать дублирования при смене языка)
    QMenuBar* menuBar = this->menuBar();
    menuBar->clear();
    
    // Меню Файл
    QMenu* fileMenu = menuBar->addMenu(tr("File"));
    
    QAction* newAction = fileMenu->addAction(tr("New"));
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);
    
    QAction* openAction = fileMenu->addAction(tr("Open..."));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    
    QAction* saveAction = fileMenu->addAction(tr("Save"));
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    
    QAction* saveAsAction = fileMenu->addAction(tr("Save As..."));
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);
    
    QAction* exportPdfAction = fileMenu->addAction(tr("Export to PDF..."));
    exportPdfAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
    connect(exportPdfAction, &QAction::triggered, this, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("PDF Files (*.pdf)"));
        if (!fileName.isEmpty()) {
            if (!fileName.endsWith(".pdf")) {
                fileName += ".pdf";
            }
            QPrinter printer(QPrinter::HighResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(fileName);
            m_previewEditor->document()->print(&printer);
        }
    });
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction(tr("Exit"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // Меню Правка
    QMenu* editMenu = menuBar->addMenu(tr("Edit"));
    
    QAction* undoAction = editMenu->addAction(tr("Undo"));
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, [this]() {
        if (m_isWysiwygMode) {
            m_previewEditor->undo();
        } else {
            m_markdownEditor->undo();
        }
    });
    
    QAction* redoAction = editMenu->addAction(tr("Redo"));
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, [this]() {
        if (m_isWysiwygMode) {
            m_previewEditor->redo();
        } else {
            m_markdownEditor->redo();
        }
    });
    
    editMenu->addSeparator();
    
    QAction* cutAction = editMenu->addAction(tr("Cut"));
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, [this]() {
        if (m_isWysiwygMode) {
            m_previewEditor->cut();
        } else {
            m_markdownEditor->cut();
        }
    });
    
    QAction* copyAction = editMenu->addAction(tr("Copy"));
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, [this]() {
        if (m_isWysiwygMode) {
            m_previewEditor->copy();
        } else {
            m_markdownEditor->copy();
        }
    });
    
    QAction* pasteAction = editMenu->addAction(tr("Paste"));
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, [this]() {
        if (m_isWysiwygMode) {
            m_previewEditor->paste();
        } else {
            m_markdownEditor->paste();
        }
    });
    
    editMenu->addSeparator();
    
    QAction* spellCheckAction = editMenu->addAction(tr("Check Spelling"));
    spellCheckAction->setShortcut(QKeySequence(Qt::Key_F7));
    connect(spellCheckAction, &QAction::triggered, this, &MainWindow::checkSpelling);
    
    // Меню Вид
    QMenu* viewMenu = menuBar->addMenu(tr("View"));
    
    m_markdownAction = viewMenu->addAction(tr("Markdown Mode"));
    m_markdownAction->setCheckable(true);
    m_markdownAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_1));
    connect(m_markdownAction, &QAction::triggered, this, &MainWindow::toggleMarkdownMode);
    
    m_wysiwygAction = viewMenu->addAction(tr("Preview Mode"));
    m_wysiwygAction->setCheckable(true);
    m_wysiwygAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_2));
    connect(m_wysiwygAction, &QAction::triggered, this, &MainWindow::toggleWysiwygMode);
    
    // Меню Справка
    QMenu* helpMenu = menuBar->addMenu(tr("Help"));
    
    QAction* helpAction = helpMenu->addAction(tr("Help..."));
    helpAction->setShortcut(QKeySequence::HelpContents);
    connect(helpAction, &QAction::triggered, this, &MainWindow::showHelp);
    
    // Подменю языков - теперь заполняется динамически
    QMenu* langMenu = helpMenu->addMenu(tr("Language"));
    
    // Динамическое заполнение списка языков на основе файлов в папке translations
    QString appPath = QCoreApplication::applicationDirPath();
    QString transPath = QDir(appPath).filePath("translations");
    QDir dir(transPath);
    
    // Множество для хранения уникальных кодов языков
    QSet<QString> languages;
    languages.insert("system"); // Всегда добавляем системный
    
    if (dir.exists()) {
        QStringList filters;
        filters << "*.qm";
        dir.setNameFilters(filters);
        QStringList files = dir.entryList();
        
        for (const QString &file : files) {
            QString baseName = QFileInfo(file).completeBaseName();
            QString langCode = baseName;
            
            // Логика извлечения кода языка из имени файла
            if (baseName.contains('_')) {
                int lastUnderscore = baseName.lastIndexOf('_');
                QString potentialLang = baseName.mid(lastUnderscore + 1);
                if (potentialLang.length() >= 2 && potentialLang.length() <= 5) {
                    langCode = potentialLang;
                }
            }
            
            languages.insert(langCode);
        }
    }
    
    // Преобразуем в список и сортируем
    QList<QString> sortedLangs = languages.values();
    std::sort(sortedLangs.begin(), sortedLangs.end());
    
    // Создаем действия для каждого языка
    for (const QString &lang : sortedLangs) {
        QAction *langAction = langMenu->addAction(lang.toUpper());
        langAction->setCheckable(true);
        langAction->setChecked(m_currentLanguage == lang);
        
        // Используем замыкание для передачи кода языка
        connect(langAction, &QAction::triggered, [this, lang]() {
            changeLanguage(lang);
        });
    }
    
    // Меню Кодировка
    QMenu* encodingMenu = menuBar->addMenu(tr("Encoding"));
    
    // Автоопределение кодировки - работает даже без сохранения файла
    QAction* autoDetectAction = encodingMenu->addAction(tr("Auto Detect"));
    autoDetectAction->setToolTip(tr("Автоматически определить кодировку текста в редакторе"));
    connect(autoDetectAction, &QAction::triggered, [this]() {
        QString content = m_markdownEditor->toPlainText();
        
        // Пробуем определить кодировку путем перебора популярных кодировок
        QList<QByteArray> codecNames = {
            "UTF-8", "Windows-1251", "KOI8-R", "CP866", "ISO 8859-5",
            "UTF-16", "UTF-32", "Windows-1252", "ISO 8859-1"
        };
        
        QString detectedCodec = "UTF-8"; // По умолчанию
        int bestScore = 0;
        
        for (const QByteArray& codecName : codecNames) {
            QTextCodec* codec = QTextCodec::codecForName(codecName);
            if (!codec) continue;
            
            // Кодируем и декодируем текст
            QByteArray encoded = codec->fromUnicode(content);
            QString decoded = codec->toUnicode(encoded);
            
            // Считаем сколько символов совпало
            int score = 0;
            for (int i = 0; i < qMin(content.length(), decoded.length()); i++) {
                if (content[i] == decoded[i]) score++;
            }
            
            if (score > bestScore) {
                bestScore = score;
                detectedCodec = codecName;
            }
        }
        
        m_statusBar->showMessage(tr("Предполагаемая кодировка: ") + detectedCodec);
        
        // Показываем информацию в диалоге
        QMessageBox::information(this, tr("Автоопределение кодировки"),
            tr("Наиболее вероятная кодировка: %1\n\n"
               "Текст содержит %2 символов.\n"
               "Для изменения кодировки используйте меню Encoding.")
            .arg(detectedCodec).arg(content.length()));
    });
    
    encodingMenu->addSeparator();
    
    // Популярные кодировки
    QAction* utf8Action = encodingMenu->addAction("UTF-8");
    utf8Action->setToolTip(tr("Конвертировать в UTF-8"));
    connect(utf8Action, &QAction::triggered, [this]() {
        convertEncoding("UTF-8");
    });
    
    QAction* utf8BomAction = encodingMenu->addAction("UTF-8 with BOM");
    utf8BomAction->setToolTip(tr("Конвертировать в UTF-8 с BOM"));
    connect(utf8BomAction, &QAction::triggered, [this]() {
        convertEncoding("UTF-8");
    });
    
    QAction* asciiAction = encodingMenu->addAction("ASCII");
    asciiAction->setToolTip(tr("Конвертировать в ASCII"));
    connect(asciiAction, &QAction::triggered, [this]() {
        convertEncoding("ASCII");
    });
    
    encodingMenu->addSeparator();
    
    // Кириллические кодировки
    QMenu* cyrillicMenu = encodingMenu->addMenu(tr("Cyrillic"));
    
    QAction* win1251Action = cyrillicMenu->addAction("Windows-1251");
    connect(win1251Action, &QAction::triggered, [this]() {
        convertEncoding("Windows-1251");
    });
    
    QAction* koi8rAction = cyrillicMenu->addAction("KOI8-R");
    connect(koi8rAction, &QAction::triggered, [this]() {
        convertEncoding("KOI8-R");
    });
    
    QAction* cp866Action = cyrillicMenu->addAction("CP866");
    connect(cp866Action, &QAction::triggered, [this]() {
        convertEncoding("CP866");
    });
    
    QAction* iso88595Action = cyrillicMenu->addAction("ISO 8859-5");
    connect(iso88595Action, &QAction::triggered, [this]() {
        convertEncoding("ISO 8859-5");
    });
    
    encodingMenu->addSeparator();
    
    // Другие популярные кодировки
    QMenu* otherMenu = encodingMenu->addMenu(tr("Other"));
    
    QAction* latin1Action = otherMenu->addAction("ISO 8859-1 (Latin-1)");
    connect(latin1Action, &QAction::triggered, [this]() {
        convertEncoding("ISO 8859-1");
    });
    
    QAction* latin9Action = otherMenu->addAction("ISO 8859-15 (Latin-9)");
    connect(latin9Action, &QAction::triggered, [this]() {
        convertEncoding("ISO 8859-15");
    });
    
    QAction* utf16Action = otherMenu->addAction("UTF-16");
    connect(utf16Action, &QAction::triggered, [this]() {
        convertEncoding("UTF-16");
    });
    
    QAction* utf32Action = otherMenu->addAction("UTF-32");
    connect(utf32Action, &QAction::triggered, [this]() {
        convertEncoding("UTF-32");
    });
    
    encodingMenu->addSeparator();
    
    // Выбор произвольной кодировки
    QAction* customAction = encodingMenu->addAction(tr("Custom Encoding..."));
    customAction->setToolTip(tr("Выбрать кодировку из списка"));
    connect(customAction, &QAction::triggered, [this]() {
        bool ok;
        QStringList codecs = {"UTF-8", "UTF-16", "UTF-32", "Windows-1251", "Windows-1252", 
                              "KOI8-R", "KOI8-U", "CP866", "ISO 8859-1", "ISO 8859-5", 
                              "ISO 8859-15", "ASCII", "Shift_JIS", "GB18030", "Big5"};
        QString codecName = QInputDialog::getItem(this, tr("Выберите кодировку"),
                                                   tr("Кодировка:"), codecs, 0, false, &ok);
        if (ok && !codecName.isEmpty()) {
            convertEncoding(codecName);
        }
    });
    
    helpMenu->addSeparator();
    
    QAction* aboutAction = helpMenu->addAction(tr("About..."));
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, tr("About"),
            tr("<h2>Markdown Editor</h2>"
            "<p>Version 1.0</p>"
            "<p>A feature-rich Markdown editor built with Qt C++.</p>"
            "<p><b>Features:</b></p>"
            "<ul>"
            "<li>WYSIWYG Preview</li>"
            "<li>Syntax Highlighting</li>"
            "<li>Spell Checking (Russian/English)</li>"
            "<li>Table Editing</li>"
            "<li>Image and Link Insertion</li>"
            "<li>Code Highlighting</li>"
            "<li>Multi-language Support</li>"
            "<li>PDF Export</li>"
            "</ul>"
            "<p>Built using Qt and C++</p>"));
    });
}

/**
 * @brief Создание статусной строки
 */
void MainWindow::createStatusBar()
{
    m_statusBar = statusBar();
    m_statusBar->showMessage("Готов к работе");
}

/**
 * @brief Применение стилей к предпросмотру
 */
void MainWindow::applyPreviewStyles()
{
    QString style = R"(
        body {
            font-family: Arial, sans-serif;
            font-size: 14px;
            line-height: 1.6;
            color: #333;
            max-width: 900px;
            margin: 0 auto;
            padding: 20px;
        }
        h1 { color: #2c3e50; border-bottom: 2px solid #eee; padding-bottom: 10px; }
        h2 { color: #34495e; border-bottom: 1px solid #eee; padding-bottom: 8px; }
        h3 { color: #4a6278; }
        h4 { color: #5d7a8f; }
        h5 { color: #708fa3; }
        h6 { color: #83a3b5; }
        a { color: #3498db; text-decoration: none; }
        a:hover { text-decoration: underline; }
        code { 
            background-color: #f4f4f4; 
            padding: 2px 6px; 
            border-radius: 3px;
            font-family: 'Courier New', monospace;
        }
        pre { 
            background-color: #f4f4f4; 
            padding: 15px; 
            border-radius: 5px;
            overflow-x: auto;
            border: 1px solid #ddd;
        }
        pre code { 
            background-color: transparent; 
            padding: 0;
        }
        blockquote { 
            border-left: 4px solid #3498db; 
            margin: 10px 0; 
            padding: 10px 20px;
            background-color: #f9f9f9;
            color: #666;
        }
        ul, ol { margin: 10px 0; padding-left: 30px; }
        li { margin: 5px 0; }
        hr { border: none; border-top: 2px solid #eee; margin: 20px 0; }
        img { max-width: 100%; height: auto; }
        table { border-collapse: collapse; width: 100%; margin: 15px 0; }
        th, td { border: 1px solid #ddd; padding: 8px 12px; }
        th { background-color: #f4f4f4; font-weight: bold; }
    )";
    
    m_previewEditor->document()->setDefaultStyleSheet(style);
}

/**
 * @brief Инициализация проверки орфографии с определением путей к словарям
 */
void MainWindow::initSpellChecker()
{
    QString affPath;
    QString dicPath;
    
    // Сначала проверяем папку приложения (для переносимости между ОС)
    QString appDir = QCoreApplication::applicationDirPath();
    
    // Функция для поиска пары файлов .aff и .dic по базовому имени
    auto findDictionaryPair = [appDir](const QString& folder, const QString& baseName) -> bool {
        QString aff = appDir + "/" + folder + "/" + baseName + ".aff";
        QString dic = appDir + "/" + folder + "/" + baseName + ".dic";
        return QFile::exists(aff) && QFile::exists(dic);
    };
    
    // Список папок для поиска (ищем только в dictionary и корневой папке приложения)
    QStringList folders = {"dictionary", ""};
    
    // Приоритет: ru_RU (как основной), затем другие языки
    QStringList languagePriority = {"ru_RU", "en_US", "en_GB", "de_DE", "fr_FR", "es_ES"};
    
    bool found = false;
    
    // Ищем словари в приоритетном порядке
    for (const QString& folder : folders) {
        if (found) break;
        
        QString folderPath = appDir;
        if (!folder.isEmpty()) {
            folderPath += "/" + folder;
        }
        
        // Сначала ищем ru_RU (или другой приоритетный язык)
        for (const QString& lang : languagePriority) {
            QString aff = folderPath + "/" + lang + ".aff";
            QString dic = folderPath + "/" + lang + ".dic";
            
            if (QFile::exists(aff) && QFile::exists(dic)) {
                affPath = aff;
                dicPath = dic;
                found = true;
                break;
            }
        }
        
        // Если не нашли приоритетные, ищем любой первый попавшийся словарь в папке
        if (!found && !folder.isEmpty()) {
            QDir dir(folderPath);
            if (dir.exists()) {
                QStringList affFiles = dir.entryList(QStringList() << "*.aff", QDir::Files);
                for (const QString& affFile : affFiles) {
                    QString baseName = affFile.left(affFile.length() - 4); // убираем .aff
                    QString dicFile = baseName + ".dic";
                    if (QFile::exists(folderPath + "/" + dicFile)) {
                        affPath = folderPath + "/" + affFile;
                        dicPath = folderPath + "/" + dicFile;
                        found = true;
                        break;
                    }
                }
            }
        }
    }
    
    // Проверяем папку проекта (для разработки)
    if (!found && QFile::exists("hunspell/ru_RU.aff") && QFile::exists("hunspell/ru_RU.dic")) {
        affPath = "hunspell/ru_RU.aff";
        dicPath = "hunspell/ru_RU.dic";
        found = true;
    }
    
    // Для Windows: проверяем стандартные пути установки
#ifdef Q_OS_WIN
    if (!found) {
        QStringList winPaths;
        winPaths << appDir + "/hunspell"
                 << appDir + "/../hunspell";
        
        for (const QString& path : winPaths) {
            QDir dir(path);
            if (dir.exists()) {
                QStringList affFiles = dir.entryList(QStringList() << "*.aff", QDir::Files);
                for (const QString& affFile : affFiles) {
                    QString baseName = affFile.left(affFile.length() - 4);
                    QString dicFile = baseName + ".dic";
                    if (QFile::exists(path + "/" + dicFile)) {
                        affPath = path + "/" + affFile;
                        dicPath = path + "/" + dicFile;
                        found = true;
                        break;
                    }
                }
            }
            if (found) break;
        }
    }
#endif
    
    // Пробуем системные пути Linux
    if (!found && QFile::exists("/usr/share/hunspell/ru_RU.aff") && QFile::exists("/usr/share/hunspell/ru_RU.dic")) {
        affPath = "/usr/share/hunspell/ru_RU.aff";
        dicPath = "/usr/share/hunspell/ru_RU.dic";
        found = true;
    }
    
    // Пробуем альтернативные системные пути Linux
    if (!found && QFile::exists("/usr/share/hunspell/ru_RU-affix.dat") && QFile::exists("/usr/share/hunspell/ru_RU-dict.dat")) {
        affPath = "/usr/share/hunspell/ru_RU-affix.dat";
        dicPath = "/usr/share/hunspell/ru_RU-dict.dat";
        found = true;
    }
    
    if (found && !affPath.isEmpty() && !dicPath.isEmpty()) {
        m_spellChecker = new SpellChecker(affPath, dicPath);
        if (!m_spellChecker->isInitialized()) {
            qWarning() << "Не удалось инициализировать проверку орфографии";
            delete m_spellChecker;
            m_spellChecker = nullptr;
        } else {
            qDebug() << "Словарь загружен:" << affPath << dicPath;
        }
    } else {
        qWarning() << "Файлы словаря не найдены. Проверка орфографии будет недоступна.";
        m_spellChecker = nullptr;
    }
}

/**
 * @brief Обработчик изменения текста в редакторе
 * Обновляет предпросмотр при изменении содержимого редактора
 */
void MainWindow::onTextChanged()
{
    if (!m_isModified) {
        m_isModified = true;
        updateWindowTitle();
    }
    
    // Обновляем предпросмотр только если мы в режиме Markdown
    if (!m_isWysiwygMode) {
        QString markdownText = m_markdownEditor->toPlainText();
        QString htmlText = m_parser->parse(markdownText);
        m_previewEditor->setHtml(htmlText);
    }
    
    // Обновляем статусную строку
    int charCount = m_markdownEditor->toPlainText().length();
    int wordCount = m_markdownEditor->toPlainText().split(' ', Qt::SkipEmptyParts).size();
    m_statusBar->showMessage(QString("Символов: %1 | Слов: %2").arg(charCount).arg(wordCount));
}

/**
 * @brief Переключение в режим WYSIWYG (визуальный редактор)
 */
void MainWindow::toggleWysiwygMode()
{
    if (m_isWysiwygMode) {
        // Если уже в режиме WYSIWYG, переключаемся обратно в Markdown
        toggleMarkdownMode();
        return;
    }
    
    m_isWysiwygMode = true;
    m_wysiwygAction->setChecked(true);
    m_markdownAction->setChecked(false);
    
    // Скрываем редактор Markdown, показываем предпросмотр как редактор
    m_markdownEditor->hide();
    m_previewEditor->show();
    m_previewEditor->setReadOnly(false);
    m_previewEditor->setFocus();
    
    // Устанавливаем размеры сплиттера так, чтобы previewEditor занимал всё пространство
    QSplitter* splitter = qobject_cast<QSplitter*>(m_markdownEditor->parentWidget());
    if (splitter) {
        splitter->setSizes(QList<int>() << 0 << splitter->width());
    }
    
    m_currentEditor = m_previewEditor;
    m_statusBar->showMessage("Режим WYSIWYG");
}

/**
 * @brief Переключение в режим Markdown (текстовый редактор)
 */
void MainWindow::toggleMarkdownMode()
{
    if (!m_isWysiwygMode) {
        return;
    }
    
    m_isWysiwygMode = false;
    m_wysiwygAction->setChecked(false);
    m_markdownAction->setChecked(true);
    
    // Показываем редактор Markdown, скрываем редактирование предпросмотра
    m_previewEditor->setReadOnly(true);
    m_previewEditor->hide();
    m_markdownEditor->show();
    m_markdownEditor->setFocus();
    
    // Восстанавливаем размеры сплиттера (50/50)
    QSplitter* splitter = qobject_cast<QSplitter*>(m_markdownEditor->parentWidget());
    if (splitter) {
        splitter->setSizes(QList<int>() << splitter->width() / 2 << splitter->width() / 2);
    }
    
    m_currentEditor = m_markdownEditor;
    m_statusBar->showMessage("Режим Markdown");
    
    // Обновляем предпросмотр
    onTextChanged();
}

/**
 * @brief Открытие файла с диска
 */
void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Открыть файл",
        "",
        "Markdown файлы (*.md *.markdown *.txt);;Все файлы (*)"
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл");
        return;
    }
    
    // Автоопределение кодировки
    QTextStream in(&file);
    in.setAutoDetectUnicode(true);  // Включает автоопределение UTF-8, UTF-16, UTF-32 и системной кодировки
    
    // Проверяем наличие BOM для более точного определения
    QByteArray bom = file.peek(4);
    if (bom.startsWith(QByteArray::fromHex("EFBBBF"))) {
        in.setCodec("UTF-8");
    } else if (bom.startsWith(QByteArray::fromHex("FFFE0000")) || bom.startsWith(QByteArray::fromHex("0000FEFF"))) {
        in.setCodec("UTF-32");
    } else if (bom.startsWith(QByteArray::fromHex("FFFE")) || bom.startsWith(QByteArray::fromHex("FEFF"))) {
        in.setCodec("UTF-16");
    } else {
        // Если BOM нет, пробуем определить по содержимому или используем системную
        in.setAutoDetectUnicode(true);
    }
    
    QString content = in.readAll();
    file.close();
    
    m_markdownEditor->setPlainText(content);
    m_currentFile = fileName;
    m_isModified = false;
    updateWindowTitle();
    
    m_statusBar->showMessage("Файл открыт: " + fileName);
}

/**
 * @brief Сохранение файла на диск
 */
void MainWindow::saveFile()
{
    if (m_currentFile.isEmpty()) {
        saveFileAs();
        return;
    }
    
    QFile file(m_currentFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить файл");
        return;
    }
    
    QTextStream out(&file);
    // Сохраняем в UTF-8 с BOM для максимальной совместимости
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);
    out << m_markdownEditor->toPlainText();
    file.close();
    
    m_isModified = false;
    updateWindowTitle();
    
    m_statusBar->showMessage("Файл сохранен: " + m_currentFile);
}

/**
 * @brief Сохранение файла под новым именем
 */
void MainWindow::saveFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Сохранить файл",
        "",
        "Markdown файлы (*.md *.markdown);;Текстовые файлы (*.txt);;Все файлы (*)"
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Добавляем расширение .md если его нет
    if (!fileName.endsWith(".md") && !fileName.endsWith(".markdown") && !fileName.endsWith(".txt")) {
        fileName += ".md";
    }
    
    m_currentFile = fileName;
    saveFile();
}

/**
 * @brief Создание нового документа
 */
void MainWindow::newFile()
{
    if (m_isModified) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Подтверждение",
            "Текущий документ не сохранен. Сохранить перед созданием нового?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );
        
        if (reply == QMessageBox::Save) {
            saveFile();
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }
    
    m_markdownEditor->clear();
    m_previewEditor->clear();
    m_currentFile.clear();
    m_isModified = false;
    updateWindowTitle();
    
    m_statusBar->showMessage("Новый документ создан");
}

/**
 * @brief Вставка жирного текста
 */
void MainWindow::insertBold()
{
    if (m_isWysiwygMode) {
        m_previewEditor->setFontWeight(m_previewEditor->fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
        m_previewEditor->setFocus();
    } else {
        insertMarkdownFormatting("**", "**");
    }
}

/**
 * @brief Вставка курсивного текста
 */
void MainWindow::insertItalic()
{
    if (m_isWysiwygMode) {
        m_previewEditor->setFontItalic(!m_previewEditor->fontItalic());
        m_previewEditor->setFocus();
    } else {
        insertMarkdownFormatting("*", "*");
    }
}

/**
 * @brief Вставка зачеркнутого текста
 */
void MainWindow::insertStrikeThrough()
{
    insertMarkdownFormatting("~~", "~~");
}

/**
 * @brief Вставка заголовка 1 уровня
 */
void MainWindow::insertHeader1()
{
    insertMarkdownAtCursor("# ");
}

/**
 * @brief Вставка заголовка 2 уровня
 */
void MainWindow::insertHeader2()
{
    insertMarkdownAtCursor("## ");
}

/**
 * @brief Вставка заголовка 3 уровня
 */
void MainWindow::insertHeader3()
{
    insertMarkdownAtCursor("### ");
}

/**
 * @brief Вставка маркированного списка
 */
void MainWindow::insertBulletList()
{
    insertMarkdownAtCursor("- ");
}

/**
 * @brief Вставка нумерованного списка
 */
void MainWindow::insertNumberedList()
{
    insertMarkdownAtCursor("1. ");
}

/**
 * @brief Вставка цитаты
 */
void MainWindow::insertBlockquote()
{
    insertMarkdownAtCursor("> ");
}

/**
 * @brief Вставка кода
 */
void MainWindow::insertCode()
{
    insertMarkdownFormatting("`", "`");
}

/**
 * @brief Вставка ссылки
 */
void MainWindow::insertLink()
{
    insertMarkdownFormatting("[", "](url)");
}

/**
 * @brief Вставка изображения
 * Открывает диалог выбора файла изображения и вставляет Markdown-разметку
 */
void MainWindow::insertImage()
{
    // Открываем диалог выбора файла изображения
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Выберите изображение",
        "",
        "Изображения (*.png *.jpg *.jpeg *.gif *.bmp *.svg *.webp);;Все файлы (*)"
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Получаем только имя файла для относительного пути
    QFileInfo fileInfo(fileName);
    QString imageFileName = fileInfo.fileName();
    
    // Спрашиваем пользователя, использовать ли полный путь или только имя файла
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Путь к изображению",
        "Использовать полный путь к файлу?\n\nДа - полный путь\nНет - только имя файла (для относительных ссылок)",
        QMessageBox::Yes | QMessageBox::No
    );
    
    QString imagePath = (reply == QMessageBox::Yes) ? fileName : imageFileName;
    
    // Запрашиваем описание изображения (alt текст)
    bool ok;
    QString altText = QInputDialog::getText(
        this,
        "Описание изображения",
        "Введите альтернативный текст (описание):",
        QLineEdit::Normal,
        "Описание изображения",
        &ok
    );
    
    if (!ok) {
        altText = "Описание";
    }
    
    // Формируем Markdown-разметку для изображения
    QString markdown = QString("![%1](%2)").arg(altText).arg(imagePath);
    insertMarkdownAtCursor(markdown);
}

/**
 * @brief Вставка горизонтальной линии
 */
void MainWindow::insertHorizontalRule()
{
    insertMarkdownAtCursor("\n---\n");
}

/**
 * @brief Вставка таблицы
 */
void MainWindow::insertTable()
{
    if (m_isWysiwygMode) {
        // В режиме WYSIWYG вставляем HTML таблицу напрямую
        QTextCursor cursor = m_previewEditor->textCursor();
        QString htmlTable = "<table border=\"1\">"
                           "<tr><th>Заголовок 1</th><th>Заголовок 2</th><th>Заголовок 3</th></tr>"
                           "<tr><td>Ячейка 1</td><td>Ячейка 2</td><td>Ячейка 3</td></tr>"
                           "<tr><td>Ячейка 4</td><td>Ячейка 5</td><td>Ячейка 6</td></tr>"
                           "</table><p><br/></p>";
        cursor.insertHtml(htmlTable);
        m_previewEditor->setTextCursor(cursor);
        m_previewEditor->setFocus();
    } else {
        // В режиме Markdown вставляем Markdown таблицу
        QString table = "\n| Заголовок 1 | Заголовок 2 | Заголовок 3 |\n"
                        "|-------------|-------------|-------------|\n"
                        "| Ячейка 1    | Ячейка 2    | Ячейка 3    |\n"
                        "| Ячейка 4    | Ячейка 5    | Ячейка 6    |\n\n";
        insertMarkdownAtCursor(table);
    }
}

/**
 * @brief Вставка строки в таблицу
 * Находит текущую позицию курсора в таблице и добавляет новую строку
 */
void MainWindow::insertTableRow()
{
    QString newRow = "| Новый текст  | Новый текст  | Новый текст  |\n";
    insertMarkdownAtCursor(newRow);
}

/**
 * @brief Вставка столбца в таблицу
 * Добавляет новый столбец ко всем строкам таблицы
 */
void MainWindow::insertTableColumn()
{
    QPlainTextEdit* editor = m_markdownEditor;
    QString text = editor->toPlainText();
    int cursorPos = editor->textCursor().position();
    
    // Находим строку с курсором
    QTextCursor cursor = editor->textCursor();
    cursor.select(QTextCursor::BlockUnderCursor);
    QString currentLine = cursor.selectedText();
    
    // Проверяем, находится ли курсор в таблице
    if (!currentLine.contains("|")) {
        // Если не в таблице, просто вставляем подсказку
        insertMarkdownAtCursor("\n| Новый столбец |\n");
        return;
    }
    
    // Разбиваем текст на строки
    QStringList lines = text.split("\n");
    int currentLineNum = text.left(cursorPos).count("\n");
    
    bool inTable = false;
    int tableStartLine = -1;
    
    // Ищем начало таблицы
    for (int i = currentLineNum; i >= 0; --i) {
        if (i < lines.size() && lines[i].trimmed().startsWith("|") && lines[i].contains("|")) {
            if (!inTable) {
                tableStartLine = i;
                inTable = true;
            }
        } else if (inTable) {
            break;
        }
    }
    
    if (!inTable) {
        insertMarkdownAtCursor(" | Новый столбец");
        return;
    }
    
    // Ищем конец таблицы
    int tableEndLine = tableStartLine;
    for (int i = tableStartLine; i < lines.size(); ++i) {
        if (lines[i].trimmed().startsWith("|") && lines[i].contains("|")) {
            tableEndLine = i;
        } else {
            break;
        }
    }
    
    // Добавляем новый столбец к каждой строке таблицы
    for (int i = tableStartLine; i <= tableEndLine; ++i) {
        if (lines[i].trimmed().startsWith("|")) {
            // Вставляем новый столбец перед последней вертикальной чертой
            int lastPipePos = lines[i].lastIndexOf('|');
            if (lastPipePos > 0) {
                lines[i] = lines[i].left(lastPipePos) + " Новый столбец |" + lines[i].mid(lastPipePos + 1);
            } else {
                lines[i] += " Новый столбец |";
            }
        }
    }
    
    // Обновляем текст
    editor->setPlainText(lines.join("\n"));
    
    // Восстанавливаем позицию курсора
    QTextCursor newCursor = editor->textCursor();
    newCursor.setPosition(cursorPos);
    editor->setTextCursor(newCursor);
}

/**
 * @brief Удаление строки из таблицы
 * Удаляет строку, в которой находится курсор
 */
void MainWindow::deleteTableRow()
{
    QPlainTextEdit* editor = m_markdownEditor;
    QTextCursor cursor = editor->textCursor();
    
    // Выделяем всю строку
    cursor.select(QTextCursor::BlockUnderCursor);
    QString currentLine = cursor.selectedText();
    
    // Проверяем, находится ли курсор в таблице
    if (!currentLine.contains("|")) {
        QMessageBox::information(this, "Информация", "Курсор должен находиться в строке таблицы");
        return;
    }
    
    // Удаляем строку
    cursor.removeSelectedText();
    
    // Удаляем символ новой строки если он есть
    if (cursor.position() < editor->toPlainText().length()) {
        cursor.deleteChar();
    }
}

/**
 * @brief Удаление всей таблицы
 * Удаляет всю таблицу, в которой находится курсор
 */
void MainWindow::deleteTable()
{
    QPlainTextEdit* editor = m_markdownEditor;
    QString text = editor->toPlainText();
    int cursorPos = editor->textCursor().position();
    
    // Разбиваем текст на строки
    QStringList lines = text.split("\n");
    int currentLineNum = text.left(cursorPos).count("\n");
    
    bool inTable = false;
    int tableStartLine = -1;
    
    // Ищем начало таблицы
    for (int i = currentLineNum; i >= 0; --i) {
        if (i < lines.size() && lines[i].trimmed().startsWith("|") && lines[i].contains("|")) {
            if (!inTable) {
                tableStartLine = i;
                inTable = true;
            }
        } else if (inTable) {
            break;
        }
    }
    
    if (!inTable) {
        QMessageBox::information(this, "Информация", "Курсор должен находиться в таблице");
        return;
    }
    
    // Ищем конец таблицы
    int tableEndLine = tableStartLine;
    for (int i = tableStartLine; i < lines.size(); ++i) {
        if (lines[i].trimmed().startsWith("|") && lines[i].contains("|")) {
            tableEndLine = i;
        } else if (inTable) {
            break;
        }
    }
    
    // Вычисляем позиции для удаления
    int startPos = 0;
    for (int i = 0; i < tableStartLine; ++i) {
        startPos += lines[i].length() + 1;
    }
    
    int endPos = startPos;
    for (int i = tableStartLine; i <= tableEndLine; ++i) {
        endPos += lines[i].length() + 1;
    }
    
    // Удаляем таблицу
    QTextCursor cursor = editor->textCursor();
    cursor.setPosition(startPos);
    cursor.setPosition(endPos, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
}

/**
 * @brief Удаление столбца из таблицы
 * Удаляет последний столбец из всех строк таблицы
 */
void MainWindow::deleteTableColumn()
{
    QPlainTextEdit* editor = m_markdownEditor;
    QString text = editor->toPlainText();
    int cursorPos = editor->textCursor().position();
    
    // Разбиваем текст на строки
    QStringList lines = text.split("\n");
    int currentLineNum = text.left(cursorPos).count("\n");
    
    bool inTable = false;
    int tableStartLine = -1;
    
    // Ищем начало таблицы
    for (int i = currentLineNum; i >= 0; --i) {
        if (i < lines.size() && lines[i].trimmed().startsWith("|") && lines[i].contains("|")) {
            if (!inTable) {
                tableStartLine = i;
                inTable = true;
            }
        } else if (inTable) {
            break;
        }
    }
    
    if (!inTable) {
        QMessageBox::information(this, "Информация", "Курсор должен находиться в таблице");
        return;
    }
    
    // Ищем конец таблицы
    int tableEndLine = tableStartLine;
    for (int i = tableStartLine; i < lines.size(); ++i) {
        if (lines[i].trimmed().startsWith("|") && lines[i].contains("|")) {
            tableEndLine = i;
        } else {
            break;
        }
    }
    
    // Удаляем последний столбец из каждой строки таблицы
    for (int i = tableStartLine; i <= tableEndLine; ++i) {
        if (lines[i].trimmed().startsWith("|")) {
            // Находим предпоследнюю вертикальную черту
            int lastPipePos = lines[i].lastIndexOf('|');
            if (lastPipePos > 0) {
                int secondLastPipePos = lines[i].lastIndexOf('|', lastPipePos - 1);
                if (secondLastPipePos > 0) {
                    lines[i] = lines[i].left(secondLastPipePos) + lines[i].mid(lastPipePos);
                }
            }
        }
    }
    
    // Обновляем текст
    editor->setPlainText(lines.join("\n"));
}

/**
 * @brief Вставка специального символа
 */
void MainWindow::insertSpecialCharacter()
{
    // Создаем диалог выбора спецсимвола
    QDialog dialog(this);
    dialog.setWindowTitle("Вставить специальный символ");
    dialog.setMinimumSize(400, 300);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    QLabel* label = new QLabel("Выберите специальный символ:");
    layout->addWidget(label);
    
    QGridLayout* gridLayout = new QGridLayout();
    QStringList specialChars = {"©", "®", "™", "°", "±", "×", "÷", "≠", "≤", "≥", 
                                "∞", "√", "∫", "∑", "π", "Ω", "µ", "¶", "§", "…",
                                "—", "–", "«", "»", "„", "", "'", "'", "•", "·"};
    
    int row = 0, col = 0;
    for (const QString& ch : specialChars) {
        QPushButton* btn = new QPushButton(ch);
        btn->setFixedSize(40, 40);
        connect(btn, &QPushButton::clicked, [this, &dialog, ch]() {
            insertMarkdownAtCursor(ch);
            dialog.accept();
        });
        gridLayout->addWidget(btn, row, col);
        col++;
        if (col > 9) {
            col = 0;
            row++;
        }
    }
    
    layout->addLayout(gridLayout);
    
    dialog.exec();
}

/**
 * @brief Вставка блока кода (многострочный)
 */
void MainWindow::insertCodeBlock()
{
    QString codeBlock = "\n```\n"
                        "// Ваш код здесь\n"
                        "\n"
                        "```\n";
    insertMarkdownAtCursor(codeBlock);
}

/**
 * @brief Вставка задачи (checkbox)
 */
void MainWindow::insertTask()
{
    QString task = "- [ ] Задача\n";
    insertMarkdownAtCursor(task);
}

/**
 * @brief Вспомогательная функция для вставки Markdown форматирования
 * @param prefix Префикс форматирования
 * @param suffix Суффикс форматирования
 */
void MainWindow::insertMarkdownFormatting(const QString& prefix, const QString& suffix)
{
    QTextCursor cursor = m_markdownEditor->textCursor();
    QString selectedText = cursor.selectedText();
    
    if (selectedText.isEmpty()) {
        cursor.insertText(prefix + suffix);
        cursor.setPosition(cursor.position() - suffix.length());
        m_markdownEditor->setTextCursor(cursor);
    } else {
        cursor.insertText(prefix + selectedText + suffix);
    }
    
    m_markdownEditor->setFocus();
}

/**
 * @brief Вспомогательная функция для вставки текста в позицию курсора
 * @param text Текст для вставки
 */
void MainWindow::insertMarkdownAtCursor(const QString& text)
{
    QTextCursor cursor = m_markdownEditor->textCursor();
    cursor.insertText(text);
    m_markdownEditor->setTextCursor(cursor);
    m_markdownEditor->setFocus();
}

/**
 * @brief Обновление заголовка окна
 */
void MainWindow::updateWindowTitle()
{
    QString title = "Markdown Editor";
    if (!m_currentFile.isEmpty()) {
        title = QFileInfo(m_currentFile).fileName() + " - " + title;
    }
    if (m_isModified) {
        title = "* " + title;
    }
    setWindowTitle(title);
}

/**
 * @brief Проверка орфографии в тексте
 */
void MainWindow::checkSpelling()
{
    if (!m_spellChecker || !m_spellChecker->isInitialized()) {
        QMessageBox::warning(this, "Проверка орфографии", 
                            "Словарь не загружен. Проверка орфографии недоступна.");
        return;
    }
    
    // Получаем текст из редактора
    QString text;
    if (m_markdownEditor) {
        text = m_markdownEditor->toPlainText();
    } else {
        return;
    }
    
    // Проверяем текст на ошибки
    m_spellingErrors = m_spellChecker->checkText(text);
    
    if (m_spellingErrors.isEmpty()) {
        QMessageBox::information(this, "Проверка орфографии", 
                                "Ошибок не найдено!");
        return;
    }
    
    // Показываем первую ошибку
    QPair<int, int> firstError = m_spellingErrors.first();
    QString wrongWord = text.mid(firstError.first, firstError.second);
    
    showSpellingContextMenu(firstError.first, wrongWord);
}

/**
 * @brief Показать контекстное меню для исправления ошибки
 * @param position Позиция ошибки в тексте
 * @param word Слово с ошибкой
 */
void MainWindow::showSpellingContextMenu(int position, const QString& word)
{
    if (!m_spellChecker || !m_markdownEditor) {
        return;
    }
    
    // Создаем контекстное меню
    QMenu menu(this);
    menu.setWindowTitle("Исправление: " + word);
    
    // Получаем предложения по исправлению
    QStringList suggestions = m_spellChecker->getSuggestions(word);
    
    if (suggestions.isEmpty()) {
        QAction* noSuggestions = new QAction("Нет предложений", &menu);
        noSuggestions->setEnabled(false);
        menu.addAction(noSuggestions);
    } else {
        // Добавляем предложения в меню
        for (const QString& suggestion : suggestions) {
            QAction* action = new QAction(suggestion, &menu);
            connect(action, &QAction::triggered, this, [this, position, word, suggestion]() {
                // Заменяем слово в тексте
                QTextCursor cursor = m_markdownEditor->textCursor();
                cursor.setPosition(position);
                cursor.setPosition(position + word.length(), QTextCursor::KeepAnchor);
                cursor.insertText(suggestion);
                m_markdownEditor->setTextCursor(cursor);
                m_isModified = true;
                updateWindowTitle();
                onTextChanged();
            });
            menu.addAction(action);
        }
    }
    
    menu.addSeparator();
    
    // Действие "Добавить в словарь"
    QAction* addToDict = new QAction("Добавить в словарь", &menu);
    connect(addToDict, &QAction::triggered, this, [this, word]() {
        m_spellChecker->addWordToDictionary(word);
        QMessageBox::information(this, "Словарь", 
                                "Слово \"" + word + "\" добавлено в пользовательский словарь.");
    });
    menu.addAction(addToDict);
    
    // Действие "Пропустить"
    QAction* skip = new QAction("Пропустить", &menu);
    connect(skip, &QAction::triggered, this, [this, position, word]() {
        // Переходим к следующей ошибке
        for (int i = 0; i < m_spellingErrors.size(); ++i) {
            QPair<int, int> error = m_spellingErrors[i];
            if (error.first == position && error.second == word.length()) {
                if (i + 1 < m_spellingErrors.size()) {
                    QPair<int, int> nextError = m_spellingErrors[i + 1];
                    QString nextWord = m_markdownEditor->toPlainText().mid(nextError.first, nextError.second);
                    showSpellingContextMenu(nextError.first, nextWord);
                } else {
                    QMessageBox::information(this, "Проверка орфографии", 
                                            "Проверка завершена.");
                }
                return;
            }
        }
    });
    menu.addAction(skip);
    
    // Показываем меню
    menu.exec(QCursor::pos());
}

/**
 * @brief Открытие окна справки
 */
void MainWindow::showHelp()
{
    HelpWindow* helpWindow = new HelpWindow(this);
    helpWindow->exec();
}

/**
 * @brief Показать контекстное меню редактора при клике правой кнопкой
 * @param pos Позиция курсора в виджете
 */
void MainWindow::showEditorContextMenu(const QPoint& pos)
{
    QMenu contextMenu(this);
    
    // Добавляем пункты редактирования
    QAction* undoAction = contextMenu.addAction(tr("Отменить"));
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, m_markdownEditor, &QPlainTextEdit::undo);
    
    QAction* redoAction = contextMenu.addAction(tr("Повторить"));
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, m_markdownEditor, &QPlainTextEdit::redo);
    
    contextMenu.addSeparator();
    
    QAction* cutAction = contextMenu.addAction(tr("Вырезать"));
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, m_markdownEditor, &QPlainTextEdit::cut);
    
    QAction* copyAction = contextMenu.addAction(tr("Копировать"));
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, m_markdownEditor, &QPlainTextEdit::copy);
    
    QAction* pasteAction = contextMenu.addAction(tr("Вставить"));
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, m_markdownEditor, &QPlainTextEdit::paste);
    
    contextMenu.addSeparator();
    
    QAction* selectAllAction = contextMenu.addAction(tr("Выделить всё"));
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, m_markdownEditor, &QPlainTextEdit::selectAll);
    
    // --- Table Operations Submenu ---
    contextMenu.addSeparator();
    QMenu *tableMenu = contextMenu.addMenu(tr("Table"));
    
    QAction *insertTableAction = tableMenu->addAction(tr("Insert Table"));
    connect(insertTableAction, &QAction::triggered, this, [this]() {
        insertTable();
    });

    QAction *deleteTableAction = tableMenu->addAction(tr("Delete Table"));
    connect(deleteTableAction, &QAction::triggered, this, [this]() {
        deleteTable();
    });
    
    // Показываем меню в позиции курсора
    QWidget* editor = qobject_cast<QWidget*>(sender());
    if (editor) {
        contextMenu.exec(editor->mapToGlobal(pos));
    }
}

/**
 * @brief Загрузка переводов для указанного языка
 * @param language Код языка ("ru", "en", "system")
 */
void MainWindow::loadTranslations(const QString& language)
{
    // Определяем язык для загрузки
    QString langToLoad = language;
    if (language == "system") {
        QLocale systemLocale = QLocale::system();
        langToLoad = systemLocale.language() == QLocale::Russian ? "ru" : "en";
    }
    
    // Удаляем предыдущий переводчик
    qApp->removeTranslator(m_translator);
    
    // Путь к папке с переводами рядом с исполняемым файлом
    QString appPath = QCoreApplication::applicationDirPath();
    QString transPath = QDir(appPath).filePath("translations");
    
    // Формируем возможные имена файлов перевода
    QStringList possibleFiles;
    possibleFiles << ("editor_" + langToLoad + ".qm")
                  << ("markdown_editor_" + langToLoad + ".qm")
                  << (langToLoad + ".qm");
    
    bool loaded = false;
    
    // Пытаемся загрузить из папки translations рядом с приложением
    for (const QString &fileName : possibleFiles) {
        QString filePath = transPath + "/" + fileName;
        if (QFile::exists(filePath)) {
            if (m_translator->load(filePath)) {
                loaded = true;
                break;
            }
        }
    }
    
    // Если не нашли, пробуем загрузить из ресурсов
    if (!loaded) {
        for (const QString &fileName : possibleFiles) {
            QString resourcePath = ":/translations/" + fileName;
            if (m_translator->load(resourcePath)) {
                loaded = true;
                break;
            }
        }
    }
    
    if (loaded) {
        qApp->installTranslator(m_translator);
        m_currentLanguage = language;
        
        // Обновляем интерфейс после смены языка
        createMenuBar();
        createToolBar();
        updateWindowTitle();
    } else {
        qDebug() << "Warning: Translation file for" << language << "not found.";
        m_currentLanguage = language;
        // Всё равно обновляем меню, чтобы отразить выбор пользователя
        createMenuBar();
        createToolBar();
        updateWindowTitle();
    }
}

/**
 * @brief Изменение языка интерфейса
 * @param language Код языка ("ru", "en", "system")
 */
void MainWindow::changeLanguage(const QString& language)
{
    loadTranslations(language);
}

/**
 * @brief Изменение цвета текста
 */
void MainWindow::changeTextColor()
{
    QColor color = QColorDialog::getColor(Qt::black, this, tr("Text Color"));
    if (color.isValid()) {
        // В зависимости от режима применяем цвет по-разному
        if (m_isWysiwygMode) {
            QTextCharFormat format = m_previewEditor->currentCharFormat();
            format.setForeground(color);
            m_previewEditor->setCurrentCharFormat(format);
        } else {
            // В режиме Markdown оборачиваем выделенный текст в HTML тег
            QTextCursor cursor = m_markdownEditor->textCursor();
            if (cursor.hasSelection()) {
                QString selectedText = cursor.selectedText();
                QString formattedText = QString("<span style=\"color: %1\">%2</span>")
                    .arg(color.name())
                    .arg(selectedText);
                cursor.insertText(formattedText);
            } else {
                // Если нет выделения, просто запоминаем цвет для следующего ввода
                QMessageBox::information(this, tr("Text Color"), 
                    tr("Выделите текст для изменения цвета."));
            }
        }
    }
}

/**
 * @brief Конвертировать файл в указанную кодировку
 */
void MainWindow::convertEncoding(const QString& codecName)
{
    // Читаем текущий текст из редактора (он уже в Unicode)
    QString content = m_markdownEditor->toPlainText();
    
    // Конвертируем текст в целевую кодировку и обратно для проверки
    QTextCodec* codec = QTextCodec::codecForName(codecName.toUtf8());
    if (!codec) {
        QMessageBox::critical(this, tr("Ошибка"), 
            tr("Не удалось найти кодировку: %1").arg(codecName));
        return;
    }
    
    // Проверяем, можно ли сконвертировать текст в эту кодировку
    QByteArray encodedData = codec->fromUnicode(content);
    QString decodedContent = codec->toUnicode(encodedData);
    
    // Предупреждаем о возможной потере данных
    if (decodedContent != content) {
        QMessageBox::StandardButton reply = QMessageBox::warning(
            this,
            tr("Предупреждение"),
            tr("При конвертации в кодировку %1 возможна потеря данных "
               "(некоторые символы могут быть заменены или утеряны).\n\n"
               "Продолжить?").arg(codecName),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        
        if (reply == QMessageBox::No) {
            return;
        }
    }
    
    // Если файл открыт - сохраняем в файл, иначе просто применяем кодировку к тексту
    if (!m_currentFile.isEmpty()) {
        // Сохраняем файл в новой кодировке
        QFile file(m_currentFile);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Ошибка"), tr("Не удалось сохранить файл."));
            return;
        }
        
        QTextStream out(&file);
        out.setCodec(codecName.toUtf8());
        
        // Добавляем BOM для UTF-8, UTF-16, UTF-32
        if (codecName.contains("UTF-8", Qt::CaseInsensitive)) {
            out.setGenerateByteOrderMark(true);
        } else if (codecName.contains("UTF-16", Qt::CaseInsensitive)) {
            out.setGenerateByteOrderMark(true);
        } else if (codecName.contains("UTF-32", Qt::CaseInsensitive)) {
            out.setGenerateByteOrderMark(true);
        }
        
        out << content;
        file.close();
        
        m_statusBar->showMessage(tr("Файл конвертирован в кодировку: ") + codecName);
        
        // Перечитываем файл чтобы убедиться что всё корректно
        openFile();
    } else {
        // Файл не открыт - просто показываем сообщение что кодировка будет применена при сохранении
        m_statusBar->showMessage(tr("Кодировка %1 будет применена при сохранении файла").arg(codecName));
        
        // Для нового файла можно сразу перекодировать текст и показать результат
        QString recodedContent = codec->toUnicode(codec->fromUnicode(content));
        if (recodedContent != content) {
            QMessageBox::information(this, tr("Информация"),
                tr("Текст был перекодирован в %1. Некоторые символы могли измениться.").arg(codecName));
        }
        m_markdownEditor->setPlainText(recodedContent);
    }
}
