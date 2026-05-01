#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QToolBar>
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QSplitter>
#include "markdownparser.h"

/**
 * @class MainWindow
 * @brief Главное окно приложения Markdown Editor
 * 
 * Это основное окно приложения, которое предоставляет:
 * - Двухпанельный интерфейс (редактор и предпросмотр)
 * - Переключение между режимами редактирования
 * - Панель инструментов с кнопками форматирования
 * - Поддержку открытия и сохранения файлов
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
    QPlainTextEdit* m_markdownEditor;  ///< Виджет редактора Markdown
    QWidget* m_currentEditor;          ///< Текущий активный редактор
    
    // Парсер Markdown
    MarkdownParser* m_parser;          ///< Парсер для конвертации Markdown в HTML
    
    // Элементы интерфейса
    QToolBar* m_toolBar;               ///< Панель инструментов
    QStatusBar* m_statusBar;           ///< Статусная строка
    QAction* m_wysiwygAction;          ///< Действие переключения в режим WYSIWYG
    QAction* m_markdownAction;         ///< Действие переключения в режим Markdown
    
    // Состояние приложения
    bool m_isWysiwygMode;              ///< Флаг режима WYSIWYG
    QString m_currentFile;             ///< Путь к текущему файлу
    bool m_isModified;                 ///< Флаг изменений в документе
};

#endif // MAINWINDOW_H
