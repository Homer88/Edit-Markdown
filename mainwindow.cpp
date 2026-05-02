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
#include <QPainter>
#include <QTextBlock>
#include <QStyleFactory>
#include "settings.h"
#include "settings_dialog.h"

// Реализация LineNumberArea
LineNumberArea::LineNumberArea(QPlainTextEdit *editor)
    : QWidget(editor)
{
    textEditor = editor;
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(textEditor->fontMetrics().horizontalAdvance(QLatin1Char('9')) * 4 + 10, 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(event->rect(), Qt::lightGray);
    
    PlainTextEditWithLineNumbers* editor = qobject_cast<PlainTextEditWithLineNumbers*>(textEditor);
    if (!editor) return;
    
    QTextBlock block = editor->firstVisibleBlockPublic();
    int blockNumber = block.blockNumber();
    qreal top = qRound(editor->blockBoundingGeometryPublic(block).translated(editor->contentOffsetPublic()).top());
    qreal bottom = top + qRound(editor->blockBoundingRectPublic(block).height());
    
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, width() - 5, textEditor->fontMetrics().height(),
                             Qt::AlignRight | Qt::AlignVCenter, number);
        }
        
        block = block.next();
        top = bottom;
        bottom = top + qRound(editor->blockBoundingRectPublic(block).height());
        ++blockNumber;
    }
}

// Реализация PlainTextEditWithLineNumbers
PlainTextEditWithLineNumbers::PlainTextEditWithLineNumbers(QWidget *parent)
    : QPlainTextEdit(parent)
{
    m_lineNumberArea = new LineNumberArea(this);
    
    connect(this, &QPlainTextEdit::blockCountChanged, this, &PlainTextEditWithLineNumbers::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &PlainTextEditWithLineNumbers::updateLineNumberArea);
    
    updateLineNumberAreaWidth(0);
}

void PlainTextEditWithLineNumbers::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    
    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), m_lineNumberArea->sizeHint().width(), cr.height()));
}

void PlainTextEditWithLineNumbers::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(m_lineNumberArea->sizeHint().width(), 0, 0, 0);
}

void PlainTextEditWithLineNumbers::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    
    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

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
    , m_darkThemeAction(nullptr)
    , m_previewAction(nullptr)
    , m_isWysiwygMode(false)
    , m_currentFile("")
    , m_currentEncoding("UTF-8")
    , m_isModified(false)
    , m_spellChecker(nullptr)
    , m_translator(new QTranslator(this))
    , m_currentLanguage("system")
    , m_isDarkMode(false)
    , m_isPreviewVisible(true)  // Предпросмотр включен по умолчанию
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
    
    // Создаем редактор Markdown (текстовый) с нумерацией строк
    m_markdownEditor = new PlainTextEditWithLineNumbers(splitter);
    m_markdownEditor->setPlaceholderText("Введите текст в формате Markdown...");
    QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    monoFont.setPointSize(12);
    m_markdownEditor->setFont(monoFont);
    m_markdownEditor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_markdownEditor, &QPlainTextEdit::customContextMenuRequested, this, &MainWindow::showEditorContextMenu);
    
    // Инициализация подсветки синтаксиса Markdown
    m_highlighter = new MarkdownHighlighter(m_markdownEditor->document());
    
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
    
    // Подключаем сигнал изменения текста в WYSIWYG редакторе для синхронизации с Markdown
    connect(m_previewEditor, &QTextEdit::textChanged, this, &MainWindow::onWysiwygTextChanged);
}

/**
 * @brief Создание панели инструментов
 */
void MainWindow::createToolBar()
{
    // Удаляем существующую панель инструментов, чтобы избежать дублирования при смене языка
    if (m_toolBar) {
        removeToolBar(m_toolBar);
        delete m_toolBar;
        m_toolBar = nullptr;
    }
    
    m_toolBar = addToolBar("Formatting");
    m_toolBar->setMovable(false);
    
    // Кнопки форматирования текста
    QAction* boldAction = m_toolBar->addAction("B");
    boldAction->setToolTip(tr("Bold (Ctrl+B)"));
    QFont boldFont = boldAction->font();
    boldFont.setBold(true);
    boldFont.setPointSize(12);
    boldAction->setFont(boldFont);
    connect(boldAction, &QAction::triggered, this, &MainWindow::insertBold);
    
    QAction* italicAction = m_toolBar->addAction("I");
    italicAction->setToolTip(tr("Italic (Ctrl+I)"));
    QFont italicFont = italicAction->font();
    italicFont.setItalic(true);
    italicFont.setPointSize(12);
    italicAction->setFont(italicFont);
    connect(italicAction, &QAction::triggered, this, &MainWindow::insertItalic);
    
    QAction* strikeAction = m_toolBar->addAction("S");
    strikeAction->setToolTip(tr("Strikethrough"));
    QFont strikeFont = strikeAction->font();
    strikeFont.setStrikeOut(true);
    strikeFont.setPointSize(12);
    strikeAction->setFont(strikeFont);
    connect(strikeAction, &QAction::triggered, this, &MainWindow::insertStrikeThrough);
    
    m_toolBar->addSeparator();
    
    // Кнопки заголовков
    QAction* h1Action = m_toolBar->addAction("H1");
    h1Action->setToolTip(tr("Heading 1"));
    h1Action->setFont(QFont(h1Action->font().family(), 12, QFont::Bold));
    connect(h1Action, &QAction::triggered, this, &MainWindow::insertHeader1);
    
    QAction* h2Action = m_toolBar->addAction("H2");
    h2Action->setToolTip(tr("Heading 2"));
    h2Action->setFont(QFont(h2Action->font().family(), 11, QFont::Bold));
    connect(h2Action, &QAction::triggered, this, &MainWindow::insertHeader2);
    
    QAction* h3Action = m_toolBar->addAction("H3");
    h3Action->setToolTip(tr("Heading 3"));
    h3Action->setFont(QFont(h3Action->font().family(), 10, QFont::Bold));
    connect(h3Action, &QAction::triggered, this, &MainWindow::insertHeader3);
    
    m_toolBar->addSeparator();
    
    // Кнопки списков
    QAction* bulletAction = m_toolBar->addAction(tr("• List"));
    bulletAction->setToolTip(tr("Bullet List"));
    connect(bulletAction, &QAction::triggered, this, &MainWindow::insertBulletList);
    
    QAction* numberAction = m_toolBar->addAction(tr("1. List"));
    numberAction->setToolTip(tr("Numbered List"));
    connect(numberAction, &QAction::triggered, this, &MainWindow::insertNumberedList);
    
    m_toolBar->addSeparator();
    
    // Кнопки остальных элементов
    QAction* quoteAction = m_toolBar->addAction(tr("Quote"));
    quoteAction->setToolTip(tr("Blockquote"));
    connect(quoteAction, &QAction::triggered, this, &MainWindow::insertBlockquote);
    
    QAction* codeAction = m_toolBar->addAction(tr("Code"));
    codeAction->setToolTip(tr("Inline Code"));
    connect(codeAction, &QAction::triggered, this, &MainWindow::insertCode);
    
    QAction* linkAction = m_toolBar->addAction(tr("Link"));
    linkAction->setToolTip(tr("Insert Link"));
    connect(linkAction, &QAction::triggered, this, &MainWindow::insertLink);
    
    QAction* imageAction = m_toolBar->addAction(tr("Image"));
    imageAction->setToolTip(tr("Insert Image"));
    connect(imageAction, &QAction::triggered, this, &MainWindow::insertImage);
    
    QAction* hrAction = m_toolBar->addAction(tr("HR"));
    hrAction->setToolTip(tr("Horizontal Rule"));
    connect(hrAction, &QAction::triggered, this, &MainWindow::insertHorizontalRule);
    
    m_toolBar->addSeparator();
    
    // Кнопка таблицы с выпадающим меню
    QToolButton* tableToolBtn = new QToolButton(m_toolBar);
    tableToolBtn->setText(tr("Table"));
    tableToolBtn->setToolTip(tr("Insert table and table operations"));
    tableToolBtn->setPopupMode(QToolButton::MenuButtonPopup);
    
    // Создаем меню для операций с таблицей
    QMenu* tableMenu = new QMenu(tableToolBtn);
    
    QAction* insertTableAction = tableMenu->addAction(tr("Insert Table"));
    connect(insertTableAction, &QAction::triggered, this, &MainWindow::insertTable);
    
    tableMenu->addSeparator();
    
    QAction* insertRowAction = tableMenu->addAction(tr("Insert Row"));
    insertRowAction->setToolTip(tr("Insert a row into the table"));
    connect(insertRowAction, &QAction::triggered, this, &MainWindow::insertTableRow);
    
    QAction* insertColAction = tableMenu->addAction(tr("Insert Column"));
    insertColAction->setToolTip(tr("Insert a column into the table"));
    connect(insertColAction, &QAction::triggered, this, &MainWindow::insertTableColumn);
    
    tableMenu->addSeparator();
    
    QAction* deleteRowAction = tableMenu->addAction(tr("Delete Row"));
    deleteRowAction->setToolTip(tr("Delete a row from the table"));
    connect(deleteRowAction, &QAction::triggered, this, &MainWindow::deleteTableRow);
    
    QAction* deleteColAction = tableMenu->addAction(tr("Delete Column"));
    deleteColAction->setToolTip(tr("Delete a column from the table"));
    connect(deleteColAction, &QAction::triggered, this, &MainWindow::deleteTableColumn);
    
    tableToolBtn->setMenu(tableMenu);
    m_toolBar->addWidget(tableToolBtn);
    
    // Кнопка блока кода
    QAction* codeBlockAction = m_toolBar->addAction(tr("Code Block"));
    codeBlockAction->setToolTip(tr("Insert multi-line code block"));
    connect(codeBlockAction, &QAction::triggered, this, &MainWindow::insertCodeBlock);
    
    // Кнопка задачи
    QAction* taskAction = m_toolBar->addAction(tr("Task"));
    taskAction->setToolTip(tr("Insert task (checkbox)"));
    connect(taskAction, &QAction::triggered, this, &MainWindow::insertTask);
    
    // Кнопка спецсимволов
    QAction* specialAction = m_toolBar->addAction(tr("Special Chars"));
    specialAction->setToolTip(tr("Insert special characters"));
    connect(specialAction, &QAction::triggered, this, &MainWindow::insertSpecialCharacter);
    
    m_toolBar->addSeparator();
    
    // Кнопка проверки орфографии
    QAction* spellCheckAction = m_toolBar->addAction("ABC ✓");
    spellCheckAction->setToolTip(tr("Check Spelling (F7)"));
    spellCheckAction->setFont(QFont(spellCheckAction->font().family(), 10, QFont::Bold));
    connect(spellCheckAction, &QAction::triggered, this, &MainWindow::checkSpelling);
    
    m_toolBar->addSeparator();
    
    // Группа действий для переключателей режимов (взаимоисключающие)
    QActionGroup* modeGroup = new QActionGroup(this);
    modeGroup->setExclusive(true);
    
    // Переключатели режимов
    m_wysiwygAction = m_toolBar->addAction(tr("WYSIWYG"));
    m_wysiwygAction->setToolTip(tr("Visual Editing Mode"));
    m_wysiwygAction->setCheckable(true);
    m_wysiwygAction->setChecked(m_isWysiwygMode);
    m_wysiwygAction->setActionGroup(modeGroup);
    connect(m_wysiwygAction, &QAction::toggled, this, &MainWindow::toggleWysiwygMode);
    
    m_markdownAction = m_toolBar->addAction(tr("Markdown"));
    m_markdownAction->setToolTip(tr("Markdown Editing Mode"));
    m_markdownAction->setCheckable(true);
    m_markdownAction->setChecked(!m_isWysiwygMode);
    m_markdownAction->setActionGroup(modeGroup);
    connect(m_markdownAction, &QAction::toggled, this, &MainWindow::toggleMarkdownMode);
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
    
    // Группа действий для переключателей режимов в меню (взаимоисключающие)
    QActionGroup* menuModeGroup = new QActionGroup(this);
    menuModeGroup->setExclusive(true);
    
    m_markdownAction = viewMenu->addAction(tr("Markdown Mode"));
    m_markdownAction->setCheckable(true);
    m_markdownAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_1));
    m_markdownAction->setChecked(!m_isWysiwygMode);
    m_markdownAction->setActionGroup(menuModeGroup);
    connect(m_markdownAction, &QAction::triggered, this, &MainWindow::toggleMarkdownMode);
    
    m_wysiwygAction = viewMenu->addAction(tr("Preview Mode"));
    m_wysiwygAction->setCheckable(true);
    m_wysiwygAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_2));
    m_wysiwygAction->setChecked(m_isWysiwygMode);
    m_wysiwygAction->setActionGroup(menuModeGroup);
    connect(m_wysiwygAction, &QAction::triggered, this, &MainWindow::toggleWysiwygMode);
    
    // Действие для предпросмотра (сплит-режим)
    m_previewAction = viewMenu->addAction(tr("Show Preview"));
    m_previewAction->setCheckable(true);
    m_previewAction->setChecked(m_isPreviewVisible);
    m_previewAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P));
    connect(m_previewAction, &QAction::triggered, this, &MainWindow::togglePreview);
    
    // Действие для тёмной темы
    m_darkThemeAction = viewMenu->addAction(tr("Dark Theme"));
    m_darkThemeAction->setCheckable(true);
    m_darkThemeAction->setChecked(m_isDarkMode);
    m_darkThemeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));
    connect(m_darkThemeAction, &QAction::toggled, this, &MainWindow::toggleDarkTheme);
    

    // Меню Настройки
    QMenu* settingsMenu = menuBar->addMenu(tr("Settings"));
    
    QAction* settingsAction = settingsMenu->addAction(tr("Basic Settings..."));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettings);
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
    
    // Создаем метку для количества строк
    m_lineCountLabel = new QLabel(this);
    m_lineCountLabel->setText(tr("Lines: 0"));
    m_statusBar->addPermanentWidget(m_lineCountLabel);
    
    // Создаем метку для кодировки
    m_encodingLabel = new QLabel(this);
    m_encodingLabel->setText("UTF-8");
    m_statusBar->addPermanentWidget(m_encodingLabel);
    
    m_statusBar->showMessage(tr("Ready"));
    
    // Подключаем обновление счетчика строк
    connect(m_markdownEditor, &QPlainTextEdit::textChanged, this, [this]() {
        int lineCount = m_markdownEditor->blockCount();
        m_lineCountLabel->setText(tr("Lines: %1").arg(lineCount));
    });
    
    // Инициализируем счетчик при создании
    int lineCount = m_markdownEditor->blockCount();
    m_lineCountLabel->setText(tr("Lines: %1").arg(lineCount));
}

/**
 * @brief Обновление информации в статусной строке
 */
void MainWindow::updateStatusBarInfo()
{
    // Обновляем количество строк
    int lineCount = m_markdownEditor->blockCount();
    m_lineCountLabel->setText(tr("Lines: %1").arg(lineCount));
    
    // Обновляем кодировку из настроек
    const auto& settings = Settings::instance();
    m_encodingLabel->setText(settings.defaultEncoding());
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
    
    // Обновляем предпросмотр только если мы в режиме Markdown и видимость предпросмотра включена
    if (!m_isWysiwygMode && m_isPreviewVisible) {
        updatePreview();
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
        // Уже в режиме WYSIWYG, ничего не делаем (QActionGroup уже обработал переключение)
        return;
    }
    
    m_isWysiwygMode = true;
    m_wysiwygAction->setChecked(true);
    m_markdownAction->setChecked(false);
    
    // Синхронизируем содержимое: конвертируем Markdown в HTML для WYSIWYG редактора
    QString markdownText = m_markdownEditor->toPlainText();
    QString htmlText = m_parser->parse(markdownText);
    
    // Формируем полный HTML с базовыми стилями для WYSIWYG редактора
    QString fullHtml = R"(
        <html>
        <head>
            <style>
                body { font-family: sans-serif; padding: 20px; line-height: 1.6; )";
    
    if (m_isDarkMode) {
        fullHtml += "background-color: #2b2b2b; color: #e0e0e0;";
    } else {
        fullHtml += "background-color: #ffffff; color: #333333;";
    }
    
    fullHtml += R"( }
                code { background-color: #f4f4f4; padding: 2px 5px; border-radius: 3px; }
                pre { background-color: #f4f4f4; padding: 10px; border-radius: 5px; overflow-x: auto; }
                blockquote { border-left: 4px solid #ccc; margin-left: 0; padding-left: 10px; color: #666; }
                table { border-collapse: collapse; width: 100%; margin-bottom: 1em; }
                th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
                th { background-color: #f2f2f2; }
                img { max-width: 100%; height: auto; }
            </style>
        </head>
        <body>)" + htmlText + R"(
        </body>
        </html>
    )";
    
    m_previewEditor->setHtml(fullHtml);
    
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
        // Уже в режиме Markdown, ничего не делаем (QActionGroup уже обработал переключение)
        return;
    }
    
    m_isWysiwygMode = false;
    m_wysiwygAction->setChecked(false);
    m_markdownAction->setChecked(true);
    
    // Конвертируем HTML обратно в Markdown при переключении из WYSIWYG в Markdown
    QString htmlText = m_previewEditor->toHtml();
    QString markdownText = m_parser->htmlToMarkdown(htmlText);
    
    // Обновляем содержимое Markdown редактора
    m_markdownEditor->blockSignals(true);
    m_markdownEditor->setPlainText(markdownText);
    m_markdownEditor->blockSignals(false);
    
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
        tr("Open File"),
        "",
        tr("Markdown files (*.md *.markdown *.txt);;All files (*)")
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to open file"));
        return;
    }
    
    QByteArray rawData = file.readAll();
    file.close();
    
    // Получаем кодировку по умолчанию из настроек
    Settings& settings = Settings::instance();
    QString defaultEncoding = settings.defaultEncoding();
    
    QTextCodec* codec = nullptr;
    QString content;
    bool ok = false;
    
    // Пробуем определить кодировку по BOM
    QByteArray bom = rawData.left(4);
    if (bom.startsWith(QByteArray::fromHex("EFBBBF"))) {
        codec = QTextCodec::codecForName("UTF-8");
        content = codec->toUnicode(rawData);
        ok = true;
    } else if (bom.startsWith(QByteArray::fromHex("FFFE0000")) || bom.startsWith(QByteArray::fromHex("0000FEFF"))) {
        codec = QTextCodec::codecForName("UTF-32");
        content = codec->toUnicode(rawData);
        ok = true;
    } else if (bom.startsWith(QByteArray::fromHex("FFFE")) || bom.startsWith(QByteArray::fromHex("FEFF"))) {
        codec = QTextCodec::codecForName("UTF-16");
        content = codec->toUnicode(rawData);
        ok = true;
    } else {
        // Если BOM нет, пробуем кодировку по умолчанию
        codec = QTextCodec::codecForName(defaultEncoding.toUtf8());
        if (codec) {
            content = codec->toUnicode(rawData);
            // Проверяем, нет ли символов замены (replacement characters)
            if (!content.contains(QChar(0xFFFD))) {
                ok = true;
            }
        }
        
        // Если не получилось или есть символы замены, пробуем автоопределение
        if (!ok) {
            // Пробуем UTF-8 без BOM
            codec = QTextCodec::codecForName("UTF-8");
            content = codec->toUnicode(rawData);
            if (!content.contains(QChar(0xFFFD))) {
                ok = true;
            } else {
                // Пробуем системную кодировку
                codec = QTextCodec::codecForLocale();
                content = codec->toUnicode(rawData);
                if (!content.contains(QChar(0xFFFD))) {
                    ok = true;
                    defaultEncoding = QString(codec->name());
                } else {
                    // Если всё ещё есть проблемы, показываем диалог выбора кодировки
                    QStringList codecNames = {
                        "UTF-8", "Windows-1251", "Windows-1252", "KOI8-R", "IBM866", 
                        "ISO-8859-1", "ISO-8859-5", "UTF-16", "UTF-32"
                    };
                    
                    bool codecOk = false;
                    while (!codecOk) {
                        QString selectedCodec = QInputDialog::getItem(
                            this,
                            tr("Select Encoding"),
                            tr("Could not auto-detect encoding. Please select:"),
                            codecNames,
                            0,
                            false,
                            &codecOk
                        );
                        
                        if (!codecOk) {
                            return; // Пользователь отменил
                        }
                        
                        codec = QTextCodec::codecForName(selectedCodec.toUtf8());
                        if (codec) {
                            content = codec->toUnicode(rawData);
                            if (!content.contains(QChar(0xFFFD))) {
                                ok = true;
                                defaultEncoding = selectedCodec;
                            } else {
                                int reply = QMessageBox::question(
                                    this,
                                    tr("Invalid Encoding"),
                                    tr("Selected encoding (%1) produced invalid characters. Try another?").arg(selectedCodec),
                                    QMessageBox::Yes | QMessageBox::No
                                );
                                if (reply == QMessageBox::No) {
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    if (!ok || content.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to decode file content"));
        return;
    }
    
    m_markdownEditor->setPlainText(content);
    m_currentFile = fileName;
    m_currentEncoding = codec ? QString(codec->name()) : defaultEncoding;
    m_isModified = false;
    updateWindowTitle();
    
    // Обновляем метки в статус-баре
    int lineCount = m_markdownEditor->blockCount();
    m_lineCountLabel->setText(tr("Lines: %1").arg(lineCount));
    m_encodingLabel->setText(m_currentEncoding);
    
    m_statusBar->showMessage(tr("File opened: ") + fileName + " (" + m_currentEncoding + ")");
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
    
    m_statusBar->showMessage(tr("File saved: ") + m_currentFile);
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
    
    // Сбрасываем счетчик строк для нового документа
    m_lineCountLabel->setText(tr("Lines: %1").arg(0));
    
    m_statusBar->showMessage(tr("New document created"));
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
    if (m_isWysiwygMode) {
        // Режим WYSIWYG - используем QTextEdit
        QTextCursor cursor = m_previewEditor->textCursor();
        QTextListFormat listFormat;
        listFormat.setStyle(QTextListFormat::ListDisc);
        cursor.createList(listFormat);
        m_previewEditor->setTextCursor(cursor);
        m_previewEditor->setFocus();
    } else {
        // Режим Markdown - вставляем текст
        insertMarkdownAtCursor("- ");
    }
}

/**
 * @brief Вставка нумерованного списка
 */
void MainWindow::insertNumberedList()
{
    if (m_isWysiwygMode) {
        // Режим WYSIWYG - используем QTextEdit
        QTextCursor cursor = m_previewEditor->textCursor();
        QTextListFormat listFormat;
        listFormat.setStyle(QTextListFormat::ListDecimal);
        cursor.createList(listFormat);
        m_previewEditor->setTextCursor(cursor);
        m_previewEditor->setFocus();
    } else {
        // Режим Markdown - вставляем текст
        insertMarkdownAtCursor("1. ");
    }
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
    if (m_isWysiwygMode) {
        QTextCursor cursor = m_previewEditor->textCursor();
        QString selectedText = cursor.selectedText();
        
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Добавление ссылки"));
        dialog.setMinimumWidth(400);
        
        QVBoxLayout* layout = new QVBoxLayout(&dialog);
        
        QLabel* textLabel = new QLabel(tr("Текст ссылки:"), &dialog);
        QLineEdit* textEdit = new QLineEdit(selectedText, &dialog);
        
        QLabel* urlLabel = new QLabel(tr("URL или путь к файлу:"), &dialog);
        QHBoxLayout* urlLayout = new QHBoxLayout();
        QLineEdit* urlEdit = new QLineEdit(&dialog);
        QPushButton* browseButton = new QPushButton(tr("Обзор..."), &dialog);
        urlLayout->addWidget(urlEdit);
        urlLayout->addWidget(browseButton);
        
        connect(browseButton, &QPushButton::clicked, this, [&urlEdit, this]() {
            QString fileName = QFileDialog::getOpenFileName(this, tr("Выберите файл"), "", tr("Все файлы (*)"));
            if (!fileName.isEmpty()) {
                urlEdit->setText(fileName);
            }
        });
        
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        QPushButton* okButton = new QPushButton(tr("ОК"), &dialog);
        QPushButton* cancelButton = new QPushButton(tr("Отмена"), &dialog);
        buttonLayout->addWidget(okButton);
        buttonLayout->addWidget(cancelButton);
        
        layout->addWidget(textLabel);
        layout->addWidget(textEdit);
        layout->addWidget(urlLabel);
        layout->addLayout(urlLayout);
        layout->addLayout(buttonLayout);
        
        connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
        connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
        
        if (dialog.exec() == QDialog::Accepted) {
            QString text = textEdit->text();
            QString url = urlEdit->text();
            
            if (!text.isEmpty() && !url.isEmpty()) {
                if (cursor.hasSelection()) {
                    cursor.insertHtml(QString("<a href=\"%1\">%2</a>").arg(url, text));
                } else {
                    cursor.insertHtml(QString("<a href=\"%1\">%2</a>").arg(url, text));
                }
                m_previewEditor->setTextCursor(cursor);
            }
        }
    } else {
        // Для режима Markdown
        QString selectedText = m_markdownEditor->textCursor().selectedText();
        
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Добавление ссылки"));
        dialog.setMinimumWidth(400);
        
        QVBoxLayout* layout = new QVBoxLayout(&dialog);
        
        QLabel* textLabel = new QLabel(tr("Текст ссылки:"), &dialog);
        QLineEdit* textEdit = new QLineEdit(selectedText, &dialog);
        
        QLabel* urlLabel = new QLabel(tr("URL или путь к файлу:"), &dialog);
        QHBoxLayout* urlLayout = new QHBoxLayout();
        QLineEdit* urlEdit = new QLineEdit(&dialog);
        QPushButton* browseButton = new QPushButton(tr("Обзор..."), &dialog);
        urlLayout->addWidget(urlEdit);
        urlLayout->addWidget(browseButton);
        
        connect(browseButton, &QPushButton::clicked, this, [&urlEdit, this]() {
            QString fileName = QFileDialog::getOpenFileName(this, tr("Выберите файл"), "", tr("Все файлы (*)"));
            if (!fileName.isEmpty()) {
                urlEdit->setText(fileName);
            }
        });
        
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        QPushButton* okButton = new QPushButton(tr("ОК"), &dialog);
        QPushButton* cancelButton = new QPushButton(tr("Отмена"), &dialog);
        buttonLayout->addWidget(okButton);
        buttonLayout->addWidget(cancelButton);
        
        layout->addWidget(textLabel);
        layout->addWidget(textEdit);
        layout->addWidget(urlLabel);
        layout->addLayout(urlLayout);
        layout->addLayout(buttonLayout);
        
        connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
        connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
        
        if (dialog.exec() == QDialog::Accepted) {
            QString text = textEdit->text();
            QString url = urlEdit->text();
            
            if (!text.isEmpty() && !url.isEmpty()) {
                insertMarkdownAtCursor(QString("[%1](%2)").arg(text, url));
            }
        }
    }
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
                        "|---|---|---|\n"
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
    if (m_isWysiwygMode) {
        // В режиме WYSIWYG работаем с QTextEdit
        QTextCursor cursor = m_previewEditor->textCursor();
        
        // Проверяем, находится ли курсор внутри таблицы
        QTextBlock block = cursor.block();
        bool inTable = false;
        
        // Ищем таблицу, проверяя HTML структуру
        QString blockText = block.text();
        if (blockText.contains("<table") || blockText.contains("<tr") || blockText.contains("<td") || blockText.contains("<th")) {
            inTable = true;
        }
        
        if (!inTable) {
            QMessageBox::information(this, tr("Information"), tr("Cursor must be inside a table"));
            return;
        }
        
        // Создаем новую строку таблицы
        QString newRowHtml = "<tr><td>Новый текст</td><td>Новый текст</td><td>Новый текст</td></tr>";
        
        // Находим текущую строку таблицы и вставляем после неё
        cursor.beginEditBlock();
        
        // Перемещаемся к началу текущей строки <tr>
        int pos = cursor.position();
        int startOfRow = -1;
        
        // Ищем начало тега <tr> перед позицией курсора
        QString html = m_previewEditor->toHtml();
        int searchPos = pos;
        while (searchPos > 0) {
            searchPos = html.lastIndexOf("<tr", searchPos - 1);
            if (searchPos >= 0) {
                startOfRow = searchPos;
                break;
            }
        }
        
        if (startOfRow >= 0) {
            // Находим конец этой строки </tr>
            int endOfRow = html.indexOf("</tr>", startOfRow);
            if (endOfRow >= 0) {
                endOfRow += 5; // Длина "</tr>"
                cursor.setPosition(endOfRow);
                cursor.insertHtml(newRowHtml);
            }
        } else {
            // Если не нашли, просто вставляем в текущую позицию
            cursor.insertHtml(newRowHtml);
        }
        
        cursor.endEditBlock();
        m_previewEditor->setTextCursor(cursor);
        m_previewEditor->setFocus();
    } else {
        // В режиме Markdown вставляем Markdown строку
        QString newRow = "| Новый текст  | Новый текст  | Новый текст  |\n";
        insertMarkdownAtCursor(newRow);
    }
}

/**
 * @brief Вставка столбца в таблицу
 * Добавляет новый столбец ко всем строкам таблицы
 */
void MainWindow::insertTableColumn()
{
    if (m_isWysiwygMode) {
        // В режиме WYSIWYG работаем с QTextEdit
        QTextCursor cursor = m_previewEditor->textCursor();
        
        // Проверяем, находится ли курсор внутри таблицы
        QTextBlock block = cursor.block();
        bool inTable = false;
        
        QString blockText = block.text();
        if (blockText.contains("<table") || blockText.contains("<tr") || blockText.contains("<td") || blockText.contains("<th")) {
            inTable = true;
        }
        
        if (!inTable) {
            QMessageBox::information(this, tr("Information"), tr("Cursor must be inside a table"));
            return;
        }
        
        // Получаем всю таблицу и добавляем столбец к каждой строке
        QString html = m_previewEditor->toHtml();
        
        // Находим все строки <tr>...</tr> и добавляем новую ячейку
        int pos = 0;
        while (true) {
            int trStart = html.indexOf("<tr", pos);
            if (trStart < 0) break;
            
            int trEnd = html.indexOf("</tr>", trStart);
            if (trEnd < 0) break;
            trEnd += 5; // Длина "</tr>"
            
            // Проверяем, есть ли уже ячейки в этой строке
            QString rowContent = html.mid(trStart, trEnd - trStart);
            if (rowContent.contains("<td") || rowContent.contains("<th")) {
                // Находим позицию перед </tr> и вставляем новую ячейку
                int insertPos = trEnd - 5; // Позиция перед "</tr>"
                QString newCell = "<td>Новый столбец</td>";
                html = html.left(insertPos) + newCell + html.mid(insertPos);
                
                // Корректируем позицию для следующей итерации
                pos = trEnd + newCell.length();
            } else {
                pos = trEnd;
            }
        }
        
        // Обновляем HTML документа
        cursor.beginEditBlock();
        m_previewEditor->setHtml(html);
        cursor.endEditBlock();
        
        m_previewEditor->setTextCursor(cursor);
        m_previewEditor->setFocus();
    } else {
        // В режиме Markdown работаем как раньше
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
}

/**
 * @brief Удаление строки из таблицы
 * Удаляет строку, в которой находится курсор
 */
void MainWindow::deleteTableRow()
{
    if (m_isWysiwygMode) {
        // В режиме WYSIWYG работаем с QTextEdit
        QTextCursor cursor = m_previewEditor->textCursor();
        
        // Проверяем, находится ли курсор внутри таблицы
        QTextBlock block = cursor.block();
        bool inTable = false;
        
        QString blockText = block.text();
        if (blockText.contains("<table") || blockText.contains("<tr") || blockText.contains("<td") || blockText.contains("<th")) {
            inTable = true;
        }
        
        if (!inTable) {
            QMessageBox::information(this, tr("Information"), tr("Cursor must be inside a table"));
            return;
        }
        
        // Находим текущую строку <tr> и удаляем её
        QString html = m_previewEditor->toHtml();
        int pos = cursor.position();
        
        // Ищем начало <tr> перед позицией курсора
        int trStart = html.lastIndexOf("<tr", pos);
        if (trStart < 0) {
            QMessageBox::information(this, tr("Information"), tr("Could not find table row"));
            return;
        }
        
        // Ищем конец </tr>
        int trEnd = html.indexOf("</tr>", trStart);
        if (trEnd < 0) {
            QMessageBox::information(this, tr("Information"), tr("Could not find table row end"));
            return;
        }
        trEnd += 5; // Длина "</tr>"
        
        // Удаляем строку
        cursor.beginEditBlock();
        html = html.left(trStart) + html.mid(trEnd);
        m_previewEditor->setHtml(html);
        cursor.endEditBlock();
        
        m_previewEditor->setTextCursor(cursor);
        m_previewEditor->setFocus();
    } else {
        // В режиме Markdown работаем как раньше
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
    if (m_isWysiwygMode) {
        // В режиме WYSIWYG работаем с QTextEdit
        QTextCursor cursor = m_previewEditor->textCursor();
        
        // Проверяем, находится ли курсор внутри таблицы
        QTextBlock block = cursor.block();
        bool inTable = false;
        
        QString blockText = block.text();
        if (blockText.contains("<table") || blockText.contains("<tr") || blockText.contains("<td") || blockText.contains("<th")) {
            inTable = true;
        }
        
        if (!inTable) {
            QMessageBox::information(this, tr("Information"), tr("Cursor must be inside a table"));
            return;
        }
        
        // Получаем весь HTML и удаляем последнюю ячейку из каждой строки
        QString html = m_previewEditor->toHtml();
        
        // Находим все строки <tr>...</tr> и удаляем последнюю ячейку
        int pos = 0;
        while (true) {
            int trStart = html.indexOf("<tr", pos);
            if (trStart < 0) break;
            
            int trEnd = html.indexOf("</tr>", trStart);
            if (trEnd < 0) break;
            trEnd += 5; // Длина "</tr>"
            
            // Находим последнюю ячейку </td> или </th> перед </tr>
            int lastTdEnd = html.lastIndexOf("</td>", trEnd);
            int lastThEnd = html.lastIndexOf("</th>", trEnd);
            int lastCellEnd = qMax(lastTdEnd, lastThEnd);
            
            if (lastCellEnd >= trStart && lastCellEnd < trEnd) {
                // Находим начало этой ячейки
                int cellStart = html.lastIndexOf("<td", lastCellEnd);
                int thStart = html.lastIndexOf("<th", lastCellEnd);
                int cellStartPos = qMax(cellStart, thStart);
                
                if (cellStartPos >= trStart) {
                    // Удаляем ячейку
                    html = html.left(cellStartPos) + html.mid(lastCellEnd + (html.mid(lastCellEnd, 6) == "</td>" ? 5 : 5));
                    // Корректируем позицию для следующей итерации
                    pos = trStart;
                } else {
                    pos = trEnd;
                }
            } else {
                pos = trEnd;
            }
        }
        
        // Обновляем HTML документа
        cursor.beginEditBlock();
        m_previewEditor->setHtml(html);
        cursor.endEditBlock();
        
        m_previewEditor->setTextCursor(cursor);
        m_previewEditor->setFocus();
    } else {
        // В режиме Markdown работаем как раньше
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
    
    // Принудительно обновляем предпросмотр
    if (!m_isWysiwygMode && m_isPreviewVisible) {
        updatePreview();
    }
}

/**
 * @brief Вспомогательная функция для вставки текста в позицию курсора
 * @param text Текст для вставки
 */
void MainWindow::insertMarkdownAtCursor(const QString& text)
{
    QTextCursor cursor = m_markdownEditor->textCursor();
    int start = cursor.position();
    cursor.insertText(text);
    
    // Перемещаем курсор в начало вставленного текста (после заголовков таблицы)
    // Для таблиц перемещаем курсор в первую ячейку содержимого
    if (text.contains("| Заголовок") && text.contains("|---|")) {
        // Находим позицию после строки разделителя
        QString insertedText = text;
        int firstNewline = insertedText.indexOf('\n');
        int secondNewline = insertedText.indexOf('\n', firstNewline + 1);
        if (secondNewline != -1) {
            // Позиция после заголовка и разделителя, в начале первой строки данных
            cursor.setPosition(start + secondNewline + 1);
            // Выделяем первую ячейку для удобства редактирования
            int cellEnd = insertedText.indexOf('|', secondNewline + 1);
            if (cellEnd != -1 && cellEnd > secondNewline + 1) {
                cursor.setPosition(start + cellEnd - 1, QTextCursor::KeepAnchor);
            }
        }
    }
    
    m_markdownEditor->setTextCursor(cursor);
    m_markdownEditor->setFocus();
    
    // Принудительно обновляем предпросмотр
    if (!m_isWysiwygMode && m_isPreviewVisible) {
        updatePreview();
    }
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
 * @brief Открытие диалога основных настроек
 */
void MainWindow::showSettings()
{
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Применяем новый размер шрифта
        QFont font = m_markdownEditor->font();
        font.setPointSize(dialog.fontSize());
        m_markdownEditor->setFont(font);
        m_previewEditor->setFont(font);
        
        // Если язык изменился, перезагружаем интерфейс
        if (dialog.language() != m_currentLanguage) {
            changeLanguage(dialog.language());
        }
        
        // Обновляем статус-бар с новой кодировкой по умолчанию
        updateStatusBarInfo();
        
        // Пересоздаем подсветку синтаксиса для применения новой цветовой схемы
        delete m_highlighter;
        m_highlighter = new MarkdownHighlighter(m_markdownEditor->document());
    }
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
    
    // Добавляем пункт "Сделать ссылкой" если текст выделен
    QString selectedText = m_markdownEditor->textCursor().selectedText();
    if (!selectedText.isEmpty()) {
        contextMenu.addSeparator();
        
        // Подменю для заголовков
        QMenu *headerMenu = contextMenu.addMenu(tr("Заголовок"));
        
        QAction* makeH1Action = headerMenu->addAction(tr("Заголовок 1 (H1)"));
        connect(makeH1Action, &QAction::triggered, this, [this]() { convertSelectionToHeader(1); });
        
        QAction* makeH2Action = headerMenu->addAction(tr("Заголовок 2 (H2)"));
        connect(makeH2Action, &QAction::triggered, this, [this]() { convertSelectionToHeader(2); });
        
        QAction* makeH3Action = headerMenu->addAction(tr("Заголовок 3 (H3)"));
        connect(makeH3Action, &QAction::triggered, this, [this]() { convertSelectionToHeader(3); });
        
        QAction* makeLinkAction = contextMenu.addAction(tr("Сделать ссылкой"));
        connect(makeLinkAction, &QAction::triggered, this, &MainWindow::makeSelectedTextLink);
    }
    
    // Добавляем пункт "Свойства гиперссылки"
    contextMenu.addSeparator();
    QAction* linkPropertiesAction = contextMenu.addAction(tr("Свойства гиперссылки"));
    connect(linkPropertiesAction, &QAction::triggered, this, &MainWindow::showLinkPropertiesDialog);
    
    // --- Table Operations Submenu ---
    contextMenu.addSeparator();
    QMenu *tableMenu = contextMenu.addMenu(tr("Table"));
    
    QAction *insertRowAction = tableMenu->addAction(tr("Insert Row"));
    connect(insertRowAction, &QAction::triggered, this, &MainWindow::insertTableRow);
    
    QAction *insertColAction = tableMenu->addAction(tr("Insert Column"));
    connect(insertColAction, &QAction::triggered, this, &MainWindow::insertTableColumn);
    
    QAction *deleteRowAction = tableMenu->addAction(tr("Delete Row"));
    connect(deleteRowAction, &QAction::triggered, this, &MainWindow::deleteTableRow);
    
    QAction *deleteColAction = tableMenu->addAction(tr("Delete Column"));
    connect(deleteColAction, &QAction::triggered, this, &MainWindow::deleteTableColumn);
    
    tableMenu->addSeparator();
    QAction *insertTableAction = tableMenu->addAction(tr("Insert Table"));
    connect(insertTableAction, &QAction::triggered, this, &MainWindow::insertTable);
    
    QAction *deleteTableAction = tableMenu->addAction(tr("Delete Table"));
    connect(deleteTableAction, &QAction::triggered, this, &MainWindow::deleteTable);
    
    // Показываем меню в позиции курсора
    QWidget* editor = qobject_cast<QWidget*>(sender());
    if (editor) {
        contextMenu.exec(editor->mapToGlobal(pos));
    }
}

/**
 * @brief Сделать выделенный текст ссылкой
 */
void MainWindow::makeSelectedTextLink()
{
    QString selectedText;
    
    if (m_isWysiwygMode) {
        selectedText = m_previewEditor->textCursor().selectedText();
    } else {
        selectedText = m_markdownEditor->textCursor().selectedText();
    }
    
    if (selectedText.isEmpty()) {
        return;
    }
    
    // Создаем диалог для ввода ссылки
    QDialog linkDialog(this);
    linkDialog.setWindowTitle(tr("Добавление ссылки"));
    linkDialog.setMinimumWidth(400);
    
    QVBoxLayout* layout = new QVBoxLayout(&linkDialog);
    
    QLabel* textLabel = new QLabel(tr("Текст ссылки:"), &linkDialog);
    QLineEdit* textEdit = new QLineEdit(selectedText, &linkDialog);
    
    QLabel* urlLabel = new QLabel(tr("URL или путь к файлу:"), &linkDialog);
    QHBoxLayout* urlLayout = new QHBoxLayout();
    QLineEdit* urlEdit = new QLineEdit(&linkDialog);
    QPushButton* browseButton = new QPushButton(tr("Обзор..."), &linkDialog);
    urlLayout->addWidget(urlEdit);
    urlLayout->addWidget(browseButton);
    
    // Кнопка обзора файла
    connect(browseButton, &QPushButton::clicked, this, [&urlEdit, this]() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Выберите файл"), "", tr("Все файлы (*)"));
        if (!fileName.isEmpty()) {
            urlEdit->setText(fileName);
        }
    });
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    QPushButton* okButton = new QPushButton(tr("ОК"), &linkDialog);
    QPushButton* cancelButton = new QPushButton(tr("Отмена"), &linkDialog);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    layout->addWidget(textLabel);
    layout->addWidget(textEdit);
    layout->addWidget(urlLabel);
    layout->addLayout(urlLayout);
    layout->addLayout(buttonLayout);
    
    connect(okButton, &QPushButton::clicked, &linkDialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &linkDialog, &QDialog::reject);
    
    if (linkDialog.exec() == QDialog::Accepted) {
        QString linkText = textEdit->text();
        QString url = urlEdit->text();
        
        if (!url.isEmpty()) {
            if (m_isWysiwygMode) {
                QTextCursor cursor = m_previewEditor->textCursor();
                cursor.insertHtml(QString("<a href=\"%1\">%2</a>").arg(url, linkText));
                m_previewEditor->setTextCursor(cursor);
            } else {
                QTextCursor cursor = m_markdownEditor->textCursor();
                QString linkMarkdown = QString("[%1](%2)").arg(linkText, url);
                cursor.insertText(linkMarkdown);
            }
        }
    }
}

/**
 * @brief Преобразовать выделенный текст в заголовок указанного уровня
 * @param level Уровень заголовка (1, 2 или 3)
 */
void MainWindow::convertSelectionToHeader(int level)
{
    QString selectedText;
    QTextCursor cursor;
    
    if (m_isWysiwygMode) {
        cursor = m_previewEditor->textCursor();
        selectedText = cursor.selectedText();
    } else {
        cursor = m_markdownEditor->textCursor();
        selectedText = cursor.selectedText();
    }
    
    if (selectedText.isEmpty()) {
        return;
    }
    
    // Формируем префикс заголовка
    QString headerPrefix;
    for (int i = 0; i < level; ++i) {
        headerPrefix += "#";
    }
    headerPrefix += " ";
    
    if (m_isWysiwygMode) {
        // В режиме WYSIWYG просто применяем форматирование
        QTextCharFormat format = cursor.charFormat();
        QFont font = format.font();
        int basePointSize = font.pointSize();
        if (basePointSize <= 0) basePointSize = 12;
        
        // Увеличиваем размер шрифта в зависимости от уровня
        switch (level) {
            case 1:
                font.setPointSize(basePointSize + 8);
                font.setBold(true);
                break;
            case 2:
                font.setPointSize(basePointSize + 4);
                font.setBold(true);
                break;
            case 3:
                font.setPointSize(basePointSize + 2);
                font.setBold(true);
                break;
        }
        
        format.setFont(font);
        cursor.setCharFormat(format);
        m_previewEditor->setTextCursor(cursor);
    } else {
        // В режиме Markdown добавляем символы # перед текстом
        QString headerText = headerPrefix + selectedText;
        cursor.insertText(headerText);
    }
}

/**
 * @brief Показать диалог свойств гиперссылки
 */
void MainWindow::showLinkPropertiesDialog()
{
    // Определяем текущий текст и ссылку под курсором
    QString currentText;
    QString currentUrl;
    bool isLink = false;
    
    if (m_isWysiwygMode) {
        QTextCursor cursor = m_previewEditor->textCursor();
        // Проверяем, находится ли курсор внутри ссылки
        QTextCharFormat format = cursor.charFormat();
        if (format.isAnchor()) {
            isLink = true;
            currentUrl = format.anchorHref();
            currentText = cursor.selectedText();
            if (currentText.isEmpty()) {
                // Получаем текст ссылки из блока
                cursor.select(QTextCursor::WordUnderCursor);
                currentText = cursor.selectedText();
            }
        }
    } else {
        QTextCursor cursor = m_markdownEditor->textCursor();
        int pos = cursor.position();
        QString text = m_markdownEditor->toPlainText();
        
        // Ищем шаблон ссылки [текст](url) вокруг позиции курсора
        QRegularExpression linkRegex("\\[([^\\]]+)\\]\\(([^)]+)\\)");
        QRegularExpressionMatchIterator it = linkRegex.globalMatch(text);
        
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            int matchStart = match.capturedStart();
            int matchEnd = match.capturedEnd();
            
            if (pos >= matchStart && pos <= matchEnd) {
                isLink = true;
                currentText = match.captured(1);
                currentUrl = match.captured(2);
                break;
            }
        }
    }
    
    // Создаем диалог для ввода свойств ссылки
    QDialog linkDialog(this);
    linkDialog.setWindowTitle(tr("Свойства гиперссылки"));
    linkDialog.setMinimumWidth(400);
    
    QVBoxLayout* layout = new QVBoxLayout(&linkDialog);
    
    QLabel* textLabel = new QLabel(tr("Текст:"), &linkDialog);
    QLineEdit* textEdit = new QLineEdit(currentText, &linkDialog);
    
    QLabel* urlLabel = new QLabel(tr("Ссылка:"), &linkDialog);
    QHBoxLayout* urlLayout = new QHBoxLayout();
    QLineEdit* urlEdit = new QLineEdit(currentUrl, &linkDialog);
    QPushButton* browseButton = new QPushButton(tr("Обзор..."), &linkDialog);
    urlLayout->addWidget(urlEdit);
    urlLayout->addWidget(browseButton);
    
    // Кнопка обзора файла
    connect(browseButton, &QPushButton::clicked, this, [&urlEdit, this]() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Выберите файл"), "", tr("Все файлы (*)"));
        if (!fileName.isEmpty()) {
            urlEdit->setText(fileName);
        }
    });
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    QPushButton* okButton = new QPushButton(tr("ОК"), &linkDialog);
    QPushButton* cancelButton = new QPushButton(tr("Отмена"), &linkDialog);
    QPushButton* deleteButton = nullptr;
    
    if (isLink) {
        deleteButton = new QPushButton(tr("Удалить ссылку"), &linkDialog);
        buttonLayout->addWidget(deleteButton);
    }
    
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    layout->addWidget(textLabel);
    layout->addWidget(textEdit);
    layout->addWidget(urlLabel);
    layout->addLayout(urlLayout);
    layout->addLayout(buttonLayout);
    
    connect(okButton, &QPushButton::clicked, &linkDialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &linkDialog, &QDialog::reject);
    
    if (deleteButton) {
        connect(deleteButton, &QPushButton::clicked, &linkDialog, [&linkDialog, this]() {
            // Удаляем ссылку
            if (m_isWysiwygMode) {
                QTextCursor textCursor = m_previewEditor->textCursor();
                QTextCharFormat format;
                format.setAnchor(false);
                format.setAnchorHref("");
                textCursor.setCharFormat(format);
                m_previewEditor->setTextCursor(textCursor);
            } else {
                QTextCursor textCursor = m_markdownEditor->textCursor();
                int pos = textCursor.position();
                QString text = m_markdownEditor->toPlainText();
                
                QRegularExpression linkRegex("\\[([^\\]]+)\\]\\(([^)]+)\\)");
                QRegularExpressionMatchIterator it = linkRegex.globalMatch(text);
                
                while (it.hasNext()) {
                    QRegularExpressionMatch match = it.next();
                    int matchStart = match.capturedStart();
                    int matchEnd = match.capturedEnd();
                    
                    if (pos >= matchStart && pos <= matchEnd) {
                        textCursor.setPosition(matchStart);
                        textCursor.setPosition(matchEnd, QTextCursor::KeepAnchor);
                        textCursor.removeSelectedText();
                        textCursor.insertText(match.captured(1));
                        break;
                    }
                }
            }
            linkDialog.accept();
        });
    }
    
    if (linkDialog.exec() == QDialog::Accepted) {
        QString linkText = textEdit->text();
        QString url = urlEdit->text();
        
        if (!url.isEmpty() && !linkText.isEmpty()) {
            if (m_isWysiwygMode) {
                QTextCursor cursor = m_previewEditor->textCursor();
                if (isLink) {
                    // Обновляем существующую ссылку
                    cursor.beginEditBlock();
                    cursor.select(QTextCursor::WordUnderCursor);
                    cursor.removeSelectedText();
                    cursor.insertHtml(QString("<a href=\"%1\">%2</a>").arg(url, linkText));
                    cursor.endEditBlock();
                } else {
                    cursor.insertHtml(QString("<a href=\"%1\">%2</a>").arg(url, linkText));
                }
                m_previewEditor->setTextCursor(cursor);
            } else {
                QTextCursor cursor = m_markdownEditor->textCursor();
                if (isLink) {
                    // Находим и заменяем существующую ссылку
                    int pos = cursor.position();
                    QString text = m_markdownEditor->toPlainText();
                    
                    QRegularExpression linkRegex("\\[([^\\]]+)\\]\\(([^)]+)\\)");
                    QRegularExpressionMatchIterator it = linkRegex.globalMatch(text);
                    
                    while (it.hasNext()) {
                        QRegularExpressionMatch match = it.next();
                        int matchStart = match.capturedStart();
                        int matchEnd = match.capturedEnd();
                        
                        if (pos >= matchStart && pos <= matchEnd) {
                            cursor.setPosition(matchStart);
                            cursor.setPosition(matchEnd, QTextCursor::KeepAnchor);
                            cursor.removeSelectedText();
                            cursor.insertText(QString("[%1](%2)").arg(linkText, url));
                            break;
                        }
                    }
                } else {
                    QString linkMarkdown = QString("[%1](%2)").arg(linkText, url);
                    cursor.insertText(linkMarkdown);
                }
            }
            updatePreview();
        }
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
    
    // Обновляем текст в статусной строке после смены языка
    if (m_lineCountLabel) {
        int lineCount = m_markdownEditor->blockCount();
        m_lineCountLabel->setText(tr("Lines: %1").arg(lineCount));
    }
    if (m_statusBar) {
        m_statusBar->showMessage(tr("Ready"));
    }
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
        
        // Обновляем метку кодировки в статусной строке
        m_encodingLabel->setText(codecName);
        
        m_statusBar->showMessage(tr("File converted to encoding: ") + codecName);
        
        // Перечитываем файл чтобы убедиться что всё корректно
        openFile();
    } else {
        // Файл не открыт - просто показываем сообщение что кодировка будет применена при сохранении
        // Обновляем метку кодировки
        m_encodingLabel->setText(codecName);
        m_statusBar->showMessage(tr("Encoding %1 will be applied when saving file").arg(codecName));
        
        // Для нового файла можно сразу перекодировать текст и показать результат
        QString recodedContent = codec->toUnicode(codec->fromUnicode(content));
        if (recodedContent != content) {
            QMessageBox::information(this, tr("Information"),
                tr("Text was recoded to %1. Some characters may have changed.").arg(codecName));
        }
        m_markdownEditor->setPlainText(recodedContent);
    }
}

/**
 * @brief Обработчик изменения текста в WYSIWYG редакторе для синхронизации с Markdown
 */
void MainWindow::onWysiwygTextChanged()
{
    if (!m_isWysiwygMode) {
        // Если мы не в режиме WYSIWYG, игнорируем изменения
        return;
    }
    
    // Получаем HTML из WYSIWYG редактора и конвертируем обратно в Markdown
    QString htmlText = m_previewEditor->toHtml();
    QString markdownText = m_parser->htmlToMarkdown(htmlText);
    
    // Блокируем сигналы чтобы избежать рекурсивного вызова
    m_markdownEditor->blockSignals(true);
    m_markdownEditor->setPlainText(markdownText);
    m_markdownEditor->blockSignals(false);
    
    // Обновляем статус модификации
    m_isModified = true;
    updateWindowTitle();
}

/**
 * @brief Переключение тёмной темы
 */
void MainWindow::toggleDarkTheme()
{
    m_isDarkMode = !m_isDarkMode;
    m_darkThemeAction->setChecked(m_isDarkMode);
    
    if (m_isDarkMode) {
        // Включаем тёмную тему
        qApp->setStyle(QStyleFactory::create("Fusion"));
        
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::black);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);

        qApp->setPalette(darkPalette);
        qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
        
        // Применяем тёмные стили к предпросмотру
        m_previewEditor->setStyleSheet(
            "QTextEdit { background-color: #2b2b2b; color: #e0e0e0; }"
        );
        m_markdownEditor->setStyleSheet(
            "QPlainTextEdit { background-color: #1e1e1e; color: #d4d4d4; }"
        );
    } else {
        // Выключаем тёмную тему (возвращаем системную)
        qApp->setPalette(QPalette());
        qApp->setStyle(QStyleFactory::create(""));
        qApp->setStyleSheet("");
        
        // Возвращаем светлые стили
        m_previewEditor->setStyleSheet("");
        m_markdownEditor->setStyleSheet("");
    }
    
    // Обновляем предпросмотр если он видим
    if (m_isPreviewVisible) {
        updatePreview();
    }
}

/**
 * @brief Переключение видимости предпросмотра
 */
void MainWindow::togglePreview()
{
    m_isPreviewVisible = !m_isPreviewVisible;
    m_previewAction->setChecked(m_isPreviewVisible);
    
    // Получаем сплиттер и управляем видимостью панелей
    QSplitter* splitter = qobject_cast<QSplitter*>(centralWidget()->layout()->itemAt(0)->widget());
    if (splitter) {
        if (m_isPreviewVisible) {
            m_previewEditor->show();
            // Восстанавливаем размеры 50/50
            splitter->setSizes(QList<int>() << 600 << 600);
            updatePreview();
        } else {
            m_previewEditor->hide();
            // Растягиваем редактор на всю ширину
            splitter->setSizes(QList<int>() << 1200 << 0);
        }
    }
}

/**
 * @brief Обновление предпросмотра
 */
void MainWindow::updatePreview()
{
    if (!m_isPreviewVisible) return;
    
    QString markdownText = m_markdownEditor->toPlainText();
    
    // Сначала парсим Markdown (заголовки, списки и т.д.)
    QString htmlContent = m_parser->parse(markdownText);
    
    // После парсинга обрабатываем оставшиеся переносы строк в обычном тексте
    // Заменяем одиночные переносы строк на <br>, но только вне HTML тегов
    QStringList lines = htmlContent.split('\n');
    QStringList processedLines;
    
    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        // Если строка содержит HTML теги блоков (h1-h6, p, ul, ol, li, blockquote, pre, hr),
        // то не добавляем <br>, иначе заменяем переносы
        if (!trimmed.isEmpty() && 
            !trimmed.startsWith("<h") &&
            !trimmed.startsWith("<ul") &&
            !trimmed.startsWith("<ol") &&
            !trimmed.startsWith("<li") &&
            !trimmed.startsWith("<blockquote") &&
            !trimmed.startsWith("<pre") &&
            !trimmed.startsWith("<hr") &&
            !trimmed.startsWith("<p")) {
            // Это обычный текст, нужно убедиться что переносы корректны
            // Но так как мы уже разбили по строкам, просто добавляем
        }
        processedLines << line;
    }
    
    htmlContent = processedLines.join('\n');
    
    // Формируем полный HTML с базовыми стилями
    QString fullHtml = R"(
        <html>
        <head>
            <style>
                body { font-family: sans-serif; padding: 20px; line-height: 1.6; )";
    
    if (m_isDarkMode) {
        fullHtml += "background-color: #2b2b2b; color: #e0e0e0;";
    } else {
        fullHtml += "background-color: #ffffff; color: #333333;";
    }
    
    fullHtml += R"( }
                code { background-color: #f4f4f4; padding: 2px 5px; border-radius: 3px; }
                pre { background-color: #f4f4f4; padding: 10px; border-radius: 5px; overflow-x: auto; }
                blockquote { border-left: 4px solid #ccc; margin-left: 0; padding-left: 10px; color: #666; }
                table { border-collapse: collapse; width: 100%; margin-bottom: 1em; }
                th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
                th { background-color: #f2f2f2; }
                img { max-width: 100%; height: auto; }
            </style>
        </head>
        <body>)" + htmlContent + R"(
        </body>
        </html>
    )";
    
    m_previewEditor->setHtml(fullHtml);
}

/**
 * @brief Экспорт в PDF
 */
void MainWindow::exportToPdf()
{
    QString defaultName = "document.pdf";
    if (!m_currentFile.isEmpty()) {
        QFileInfo fi(m_currentFile);
        defaultName = fi.baseName() + ".pdf";
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to PDF"), defaultName, tr("PDF Files (*.pdf)"));
    
    if (fileName.isEmpty()) return;
    
    if (!fileName.endsWith(".pdf")) {
        fileName += ".pdf";
    }
    
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    
    // Используем HTML из предпросмотра для печати
    m_previewEditor->document()->print(&printer);
    
    QMessageBox::information(this, tr("Success"), tr("File saved successfully:\n%1").arg(fileName));
}
