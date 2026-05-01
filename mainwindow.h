#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QMenuBar>
#include <QStatusBar>
#include <QSplitter>
#include <QTranslator>
#include <QWidget>
#include "markdownparser.h"
#include "spellchecker.h"
#include "helpwindow.h"

/**
 * @class LineNumberArea
 * @brief Виджет для отображения номеров строк слева от редактора
 */
class LineNumberArea : public QWidget
{
    Q_OBJECT
public:
    explicit LineNumberArea(QPlainTextEdit *editor);
    
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPlainTextEdit *textEditor;
};

/**
 * @class PlainTextEditWithLineNumbers
 * @brief QPlainTextEdit с поддержкой нумерации строк
 */
class PlainTextEditWithLineNumbers : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit PlainTextEditWithLineNumbers(QWidget *parent = nullptr);
    
    LineNumberArea* lineNumberArea() const { return m_lineNumberArea; }

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    LineNumberArea *m_lineNumberArea;
};

/**
 * @class MainWindow
 * @brief Главное окно приложения Markdown Editor
 * 
 * Это основное окно приложения, которое предоставляет:
 * - Двухпанельный интерфейс (редактор и предпросмотр)
 * - Переключение между режимами редактирования
 * - Панель инструментов с кнопками форматирования
 * - Поддержку открытия и сохранения файлов
 * - Многоязычный интерфейс с автоопределением языка системы
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    /**
     * @brief Конструктор главного окна
     * @param parent Родительский виджет (по умолчанию nullptr)
     */
    explicit MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief Деструктор класса MainWindow
     */
    ~MainWindow();
    
private slots:
    /**
     * @brief Обработчик изменения текста в редакторе
     * Обновляет предпросмотр при изменении содержимого редактора
     */
    void onTextChanged();
    
    /**
     * @brief Переключение в режим WYSIWYG (визуальный редактор)
     */
    void toggleWysiwygMode();
    
    /**
     * @brief Переключение в режим Markdown (текстовый редактор)
     */
    void toggleMarkdownMode();
    
    /**
     * @brief Открытие файла с диска
     */
    void openFile();
    
    /**
     * @brief Сохранение файла на диск
     */
    void saveFile();
    
    /**
     * @brief Сохранение файла под новым именем
     */
    void saveFileAs();
    
    /**
     * @brief Создание нового документа
     */
    void newFile();
    
    /**
     * @brief Вставка жирного текста
     */
    void insertBold();
    
    /**
     * @brief Вставка курсивного текста
     */
    void insertItalic();
    
    /**
     * @brief Вставка зачеркнутого текста
     */
    void insertStrikeThrough();
    
    /**
     * @brief Вставка заголовка 1 уровня
     */
    void insertHeader1();
    
    /**
     * @brief Вставка заголовка 2 уровня
     */
    void insertHeader2();
    
    /**
     * @brief Вставка заголовка 3 уровня
     */
    void insertHeader3();
    
    /**
     * @brief Вставка маркированного списка
     */
    void insertBulletList();
    
    /**
     * @brief Вставка нумерованного списка
     */
    void insertNumberedList();
    
    /**
     * @brief Вставка цитаты
     */
    void insertBlockquote();
    
    /**
     * @brief Вставка кода
     */
    void insertCode();
    
    /**
     * @brief Вставка ссылки
     */
    void insertLink();
    
    /**
     * @brief Вставка изображения
     */
    void insertImage();
    
    /**
     * @brief Вставка горизонтальной линии
     */
    void insertHorizontalRule();
    
    /**
     * @brief Вставка таблицы
     */
    void insertTable();
    
    /**
     * @brief Вставка строки в таблицу
     */
    void insertTableRow();
    
    /**
     * @brief Вставка столбца в таблицу
     */
    void insertTableColumn();
    
    /**
     * @brief Удаление строки из таблицы
     */
    void deleteTableRow();
    
    /**
     * @brief Удаление всей таблицы
     */
    void deleteTable();
    
    /**
     * @brief Удаление столбца из таблицы
     */
    void deleteTableColumn();
    
    /**
     * @brief Вставка специального символа
     */
    void insertSpecialCharacter();
    
    /**
     * @brief Вставка блока кода (многострочный)
     */
    void insertCodeBlock();
    
    /**
     * @brief Вставка задачи (checkbox)
     */
    void insertTask();
    
    /**
     * @brief Проверка орфографии в тексте
     */
    void checkSpelling();
    
    /**
     * @brief Показать контекстное меню для исправления ошибки
     * @param position Позиция ошибки в тексте
     * @param word Слово с ошибкой
     */
    void showSpellingContextMenu(int position, const QString& word);
    
    /**
     * @brief Показать контекстное меню редактора при клике правой кнопкой
     * @param pos Позиция курсора в виджете
     */
    void showEditorContextMenu(const QPoint& pos);
    
    /**
     * @brief Открытие окна справки
     */
    void showHelp();
    
    /**
     * @brief Изменение языка интерфейса
     * @param language Код языка ("ru", "en", "system")
     */
    void changeLanguage(const QString& language);
    
    /**
     * @brief Изменение цвета текста
     */
    void changeTextColor();
    
    /**
     * @brief Конвертировать файл в указанную кодировку
     * @param codecName Название кодировки (например, "UTF-8", "Windows-1251")
     */
    void convertEncoding(const QString& codecName);
    
private:
    /**
     * @brief Инициализация пользовательского интерфейса
     */
    void initUI();
    
    /**
     * @brief Создание панели инструментов
     */
    void createToolBar();
    
    /**
     * @brief Создание меню
     */
    void createMenuBar();
    
    /**
     * @brief Создание статусной строки
     */
    void createStatusBar();
    
    /**
     * @brief Применение стилей к предпросмотру
     */
    void applyPreviewStyles();
    
    /**
     * @brief Обновление заголовка окна
     */
    void updateWindowTitle();
    
    /**
     * @brief Загрузка переводов для указанного языка
     * @param language Код языка
     */
    void loadTranslations(const QString& language);
    
    /**
     * @brief Инициализация проверки орфографии с определением путей к словарям
     */
    void initSpellChecker();
    
    /**
     * @brief Вспомогательная функция для вставки Markdown форматирования
     * @param prefix Префикс форматирования
     * @param suffix Суффикс форматирования
     */
    void insertMarkdownFormatting(const QString& prefix, const QString& suffix);
    
    /**
     * @brief Вспомогательная функция для вставки текста в позицию курсора
     * @param text Текст для вставки
     */
    void insertMarkdownAtCursor(const QString& text);
    
    // Виджеты редактора
    QTextEdit* m_previewEditor;        ///< Виджет предпросмотра (WYSIWYG)
    PlainTextEditWithLineNumbers* m_markdownEditor;  ///< Виджет редактора Markdown с нумерацией строк
    QWidget* m_currentEditor;          ///< Текущий активный редактор
    
    // Парсер Markdown
    MarkdownParser* m_parser;          ///< Парсер для конвертации Markdown в HTML
    
    // Элементы интерфейса
    QToolBar* m_toolBar;               ///< Панель инструментов
    QStatusBar* m_statusBar;           ///< Статусная строка
    QAction* m_wysiwygAction;          ///< Действие переключения в режим WYSIWYG
    QAction* m_markdownAction;         ///< Действие переключения в режим Markdown
    
    // Элементы статусной строки
    QLabel* m_lineCountLabel;          ///< Метка количества строк
    QLabel* m_encodingLabel;           ///< Метка кодировки
    
    // Состояние приложения
    bool m_isWysiwygMode;              ///< Флаг режима WYSIWYG
    QString m_currentFile;             ///< Путь к текущему файлу
    bool m_isModified;                 ///< Флаг изменений в документе
    
    // Проверка орфографии
    SpellChecker* m_spellChecker;      ///< Экземпляр проверки орфографии
    QList<QPair<int, int>> m_spellingErrors;  ///< Список ошибок орфографии (позиция, длина)
    
    // Переводы
    QTranslator* m_translator;         ///< Основной переводчик интерфейса
    QString m_currentLanguage;         ///< Текущий выбранный язык
};

#endif // MAINWINDOW_H
