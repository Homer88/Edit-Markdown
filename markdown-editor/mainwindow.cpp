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
    , m_spellChecker(new SpellChecker("/usr/share/hunspell/ru_RU.aff", "/usr/share/hunspell/ru_RU.dic"))
{
    setWindowTitle("Markdown Editor");
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
    
    // Создаем редактор предпросмотра (WYSIWYG)
    m_previewEditor = new QTextEdit(splitter);
    m_previewEditor->setReadOnly(true);
    m_previewEditor->setPlaceholderText("Предпросмотр будет здесь...");
    
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
    QMenuBar* menuBar = this->menuBar();
    
    // Меню Файл
    QMenu* fileMenu = menuBar->addMenu("Файл");
    
    QAction* newAction = fileMenu->addAction("Новый");
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);
    
    QAction* openAction = fileMenu->addAction("Открыть");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    
    QAction* saveAction = fileMenu->addAction("Сохранить");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    
    QAction* saveAsAction = fileMenu->addAction("Сохранить как...");
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction("Выход");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // Меню Правка
    QMenu* editMenu = menuBar->addMenu("Правка");
    
    QAction* boldAction = editMenu->addAction("Жирный");
    boldAction->setShortcut(QKeySequence::Bold);
    connect(boldAction, &QAction::triggered, this, &MainWindow::insertBold);
    
    QAction* italicAction = editMenu->addAction("Курсив");
    italicAction->setShortcut(QKeySequence::Italic);
    connect(italicAction, &QAction::triggered, this, &MainWindow::insertItalic);
    
    editMenu->addSeparator();
    
    QAction* spellCheckAction = editMenu->addAction("Проверить орфографию");
    spellCheckAction->setShortcut(QKeySequence(Qt::Key_F7));
    connect(spellCheckAction, &QAction::triggered, this, &MainWindow::checkSpelling);
    
    // Меню Справка
    QMenu* helpMenu = menuBar->addMenu("Справка");
    
    QAction* helpAction = helpMenu->addAction("Справка...");
    helpAction->setShortcut(QKeySequence::HelpContents);
    connect(helpAction, &QAction::triggered, this, &MainWindow::showHelp);
    
    QAction* aboutAction = helpMenu->addAction("О программе...");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "О программе",
            "<h2>Markdown Editor</h2>"
            "<p>Версия 1.0</p>"
            "<p>Редактор Markdown с поддержкой WYSIWYG режима.</p>"
            "<p><b>Возможности:</b></p>"
            "<ul>"
            "<li>Двухрежимное редактирование (Markdown/WYSIWYG)</li>"
            "<li>Проверка орфографии (русский/английский)</li>"
            "<li>Работа с таблицами</li>"
            "<li>Вставка изображений и ссылок</li>"
            "<li>Подсветка кода</li>"
            "</ul>"
            "<p>Создано с использованием Qt6 и C++</p>");
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
    
    QTextStream in(&file);
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
    QString table = "\n| Заголовок 1 | Заголовок 2 | Заголовок 3 |\n"
                    "|-------------|-------------|-------------|\n"
                    "| Ячейка 1    | Ячейка 2    | Ячейка 3    |\n"
                    "| Ячейка 4    | Ячейка 5    | Ячейка 6    |\n\n";
    insertMarkdownAtCursor(table);
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
