#ifndef MARKDOWNPARSER_H
#define MARKDOWNPARSER_H

#include <QString>
#include <QTextDocument>
#include <QMap>

/**
 * @class MarkdownParser
 * @brief Класс для парсинга Markdown текста в форматированный HTML
 * 
 * Этот класс преобразует Markdown разметку в HTML формат,
 * который может быть отображен в QTextEdit виджете.
 */
class MarkdownParser
{
public:
    /**
     * @brief Конструктор класса MarkdownParser
     */
    MarkdownParser();
    
    /**
     * @brief Преобразует Markdown текст в QTextDocument (WYSIWYG)
     * @param markdown Исходный текст в формате Markdown
     * @return QTextDocument* Документ с форматированием
     */
    QTextDocument* parseToDocument(const QString& markdown);
    
    /**
     * @brief Преобразует QTextDocument обратно в Markdown
     * @param doc Документ для конвертации
     * @return QString Текст в формате Markdown
     */
    QString documentToMarkdown(QTextDocument* doc);

private:
    /**
     * @brief Парсит заголовки различных уровней
     * @param text Текст для обработки
     * @return QString Текст с обработанными заголовками
     */
    QString parseHeaders(const QString& text);
    
    /**
     * @brief Парсит жирный текст
     * @param text Текст для обработки
     * @return QString Текст с обработанным жирным форматированием
     */
    QString parseBold(const QString& text);
    
    /**
     * @brief Парсит курсивный текст
     * @param text Текст для обработки
     * @return QString Текст с обработанным курсивным форматированием
     */
    QString parseItalic(const QString& text);
    
    /**
     * @brief Парсит зачеркнутый текст
     * @param text Текст для обработки
     * @return QString Текст с обработанным зачеркнутым форматированием
     */
    QString parseStrikeThrough(const QString& text);
    
    /**
     * @brief Парсит маркированные и нумерованные списки
     * @param text Текст для обработки
     * @return QString Текст с обработанными списками
     */
    QString parseLists(const QString& text);
    
    /**
     * @brief Парсит код (инлайн и блоки)
     * @param text Текст для обработки
     * @return QString Текст с обработанным кодом
     */
    QString parseCode(const QString& text);
    
    /**
     * @brief Парсит цитаты
     * @param text Текст для обработки
     * @return QString Текст с обработанными цитатами
     */
    QString parseBlockquotes(const QString& text);
    
    /**
     * @brief Парсит горизонтальные линии
     * @param text Текст для обработки
     * @return QString Текст с обработанными горизонтальными линиями
     */
    QString parseHorizontalRules(const QString& text);
    
    /**
     * @brief Парсит ссылки
     * @param text Текст для обработки
     * @return QString Текст с обработанными ссылками
     */
    QString parseLinks(const QString& text);
    
    /**
     * @brief Парсит изображения
     * @param text Текст для обработки
     * @return QString Текст с обработанными изображениями
     */
    QString parseImages(const QString& text);
    
    /**
     * @brief Парсит параграфы (обычный текст)
     * @param text Текст для обработки
     * @return QString Текст с обработанными параграфами
     */
    QString parseParagraphs(const QString& text);
    
    /**
     * @brief Парсит таблицы (GFM формат)
     * @param text Текст для обработки
     * @return QString Текст с обработанными таблицами
     */
    QString parseTables(const QString& text);
    
    /**
     * @brief Экранирует специальные символы
     * @param text Текст для обработки
     * @return QString Текст с экранированными символами
     */
    QString parseEscapes(const QString& text);
    
    /**
     * @brief Сохраняет блоки кода в специальные плейсхолдеры
     * @param text Текст для обработки
     * @return QString Текст с замененными блоками кода
     */
    QString preserveCodeBlocks(const QString& text);
    
    /**
     * @brief Восстанавливает блоки кода и обрабатывает инлайн код
     * @param text Текст для обработки
     * @return QString Текст с восстановленным кодом
     */
    QString restoreAndParseCode(const QString& text);
    
    /**
     * @brief Парсит математику LaTeX
     * @param text Текст для обработки
     * @return QString Текст с обработанной математикой
     */
    QString parseMath(const QString& text);
    
    // Хранилища для плейсхолдеров кода
    QMap<QString, QString> codeBlocks_;
    QMap<QString, QString> inlineCodes_;
    
public:
    /**
     * @brief Преобразует HTML обратно в Markdown (для синхронизации WYSIWYG -> Markdown)
     * @param html HTML текст для конвертации
     * @return QString Текст в формате Markdown
     */
    QString htmlToMarkdown(const QString& html);
};

#endif // MARKDOWNPARSER_H
