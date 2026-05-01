#ifndef MARKDOWNPARSER_H
#define MARKDOWNPARSER_H

#include <QString>
#include <QTextDocument>

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
     * @brief Преобразует Markdown текст в HTML формат
     * @param markdown Исходный текст в формате Markdown
     * @return QString Текст в формате HTML
     * 
     * Поддерживаемые элементы Markdown:
     * - Заголовки (#, ##, ### и т.д.)
     * - Жирный текст (**текст** или __текст__)
     * - Курсив (*текст* или _текст_)
     * - Зачеркнутый текст (~~текст~~)
     * - Списки (маркированные и нумерованные)
     * - Код (``код`` и блоки кода)
     * - Цитаты (> текст)
     * - Горизонтальные линии (---)
     * - Ссылки ([текст](url))
     * - Изображения (![альт](url))
     */
    QString parse(const QString& markdown);
    
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
     * @brief Преобразует HTML обратно в Markdown (для синхронизации WYSIWYG -> Markdown)
     * @param html HTML текст для конвертации
     * @return QString Текст в формате Markdown
     */
    QString htmlToMarkdown(const QString& html);
};

#endif // MARKDOWNPARSER_H
