#include "markdownparser.h"
#include <QRegularExpression>

/**
 * @brief Конструктор класса MarkdownParser
 */
MarkdownParser::MarkdownParser()
{
}

/**
 * @brief Преобразует Markdown текст в HTML формат
 * @param markdown Исходный текст в формате Markdown
 * @return QString Текст в формате HTML
 */
QString MarkdownParser::parse(const QString& markdown)
{
    QString html = markdown;
    
    // Последовательная обработка всех элементов Markdown
    html = parseHorizontalRules(html);
    html = parseHeaders(html);
    html = parseBlockquotes(html);
    html = parseCode(html);
    html = parseLists(html);
    html = parseImages(html);
    html = parseLinks(html);
    html = parseStrikeThrough(html);
    html = parseBold(html);
    html = parseItalic(html);
    html = parseParagraphs(html);
    
    return html;
}

/**
 * @brief Парсит заголовки различных уровней
 * @param text Текст для обработки
 * @return QString Текст с обработанными заголовками
 */
QString MarkdownParser::parseHeaders(const QString& text)
{
    QString result = text;
    
    // Заголовок 6 уровня (######)
    QRegularExpression h6("^######\\s+(.+)$", QRegularExpression::MultilineOption);
    result.replace(h6, "<h6>\\1</h6>");
    
    // Заголовок 5 уровня (#####)
    QRegularExpression h5("^#####\\s+(.+)$", QRegularExpression::MultilineOption);
    result.replace(h5, "<h5>\\1</h5>");
    
    // Заголовок 4 уровня (####)
    QRegularExpression h4("^####\\s+(.+)$", QRegularExpression::MultilineOption);
    result.replace(h4, "<h4>\\1</h4>");
    
    // Заголовок 3 уровня (###)
    QRegularExpression h3("^###\\s+(.+)$", QRegularExpression::MultilineOption);
    result.replace(h3, "<h3>\\1</h3>");
    
    // Заголовок 2 уровня (##)
    QRegularExpression h2("^##\\s+(.+)$", QRegularExpression::MultilineOption);
    result.replace(h2, "<h2>\\1</h2>");
    
    // Заголовок 1 уровня (#)
    QRegularExpression h1("^#\\s+(.+)$", QRegularExpression::MultilineOption);
    result.replace(h1, "<h1>\\1</h1>");
    
    return result;
}

/**
 * @brief Парсит жирный текст
 * @param text Текст для обработки
 * @return QString Текст с обработанным жирным форматированием
 */
QString MarkdownParser::parseBold(const QString& text)
{
    QString result = text;
    
    // Жирный текст с ** или __
    QRegularExpression bold("(\\*\\*|__)(?!\\s)(.+?)(?<!\\s)\\1");
    result.replace(bold, "<b>\\2</b>");
    
    return result;
}

/**
 * @brief Парсит курсивный текст
 * @param text Текст для обработки
 * @return QString Текст с обработанным курсивным форматированием
 */
QString MarkdownParser::parseItalic(const QString& text)
{
    QString result = text;
    
    // Курсив с * или _ (но не **, __)
    QRegularExpression italic("(?<![*_])([*_])(?!\\s)(.+?)(?<!\\s)\\1(?![*_])");
    result.replace(italic, "<i>\\2</i>");
    
    return result;
}

/**
 * @brief Парсит зачеркнутый текст
 * @param text Текст для обработки
 * @return QString Текст с обработанным зачеркнутым форматированием
 */
QString MarkdownParser::parseStrikeThrough(const QString& text)
{
    QString result = text;
    
    // Зачеркнутый текст с ~~
    QRegularExpression strike("~~(?<!\\s)(.+?)(?<!\\s)~~");
    result.replace(strike, "<s>\\1</s>");
    
    return result;
}

/**
 * @brief Парсит маркированные и нумерованные списки
 * @param text Текст для обработки
 * @return QString Текст с обработанными списками
 */
QString MarkdownParser::parseLists(const QString& text)
{
    QStringList lines = text.split('\n');
    QString result;
    bool inUnorderedList = false;
    bool inOrderedList = false;
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        
        // Проверка на маркированный список (-, *, +)
        QRegularExpression unorderedItem("^\\s*[-*+]\\s+(.+)$");
        QRegularExpressionMatch matchU = unorderedItem.match(line);
        
        // Проверка на нумерованный список (1., 2., и т.д.)
        QRegularExpression orderedItem("^\\s*\\d+\\.\\s+(.+)$");
        QRegularExpressionMatch matchO = orderedItem.match(line);
        
        if (matchU.hasMatch()) {
            if (!inUnorderedList) {
                if (inOrderedList) {
                    result += "</ol>";
                    inOrderedList = false;
                }
                result += "<ul>";
                inUnorderedList = true;
            }
            result += "<li>" + matchU.captured(1) + "</li>";
        } else if (matchO.hasMatch()) {
            if (!inOrderedList) {
                if (inUnorderedList) {
                    result += "</ul>";
                    inUnorderedList = false;
                }
                result += "<ol>";
                inOrderedList = true;
            }
            result += "<li>" + matchO.captured(1) + "</li>";
        } else {
            if (inUnorderedList) {
                result += "</ul>";
                inUnorderedList = false;
            }
            if (inOrderedList) {
                result += "</ol>";
                inOrderedList = false;
            }
            result += line + "\n";
        }
    }
    
    // Закрытие открытых списков
    if (inUnorderedList) {
        result += "</ul>";
    }
    if (inOrderedList) {
        result += "</ol>";
    }
    
    return result.trimmed();
}

/**
 * @brief Парсит код (инлайн и блоки)
 * @param text Текст для обработки
 * @return QString Текст с обработанным кодом
 */
QString MarkdownParser::parseCode(const QString& text)
{
    QString result = text;
    
    // Блоки кода с ```
    QRegularExpression codeBlock("```([\\s\\S]*?)```");
    result.replace(codeBlock, "<pre><code>\\1</code></pre>");
    
    // Инлайн код с `
    QRegularExpression inlineCode("`([^`]+)`");
    result.replace(inlineCode, "<code>\\1</code>");
    
    return result;
}

/**
 * @brief Парсит цитаты
 * @param text Текст для обработки
 * @return QString Текст с обработанными цитатами
 */
QString MarkdownParser::parseBlockquotes(const QString& text)
{
    QString result = text;
    QStringList lines = result.split('\n');
    QString processed;
    bool inBlockquote = false;
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        QRegularExpression blockquoteLine("^>\\s*(.*)$");
        QRegularExpressionMatch match = blockquoteLine.match(line);
        
        if (match.hasMatch()) {
            if (!inBlockquote) {
                processed += "<blockquote>";
                inBlockquote = true;
            }
            processed += match.captured(1) + "\n";
        } else {
            if (inBlockquote) {
                processed += "</blockquote>";
                inBlockquote = false;
            }
            processed += line + "\n";
        }
    }
    
    if (inBlockquote) {
        processed += "</blockquote>";
    }
    
    return processed;
}

/**
 * @brief Парсит горизонтальные линии
 * @param text Текст для обработки
 * @return QString Текст с обработанными горизонтальными линиями
 */
QString MarkdownParser::parseHorizontalRules(const QString& text)
{
    QString result = text;
    
    // Горизонтальная линия (---, ***, ___)
    QRegularExpression hr("^([-*_]){3,}\\s*$", QRegularExpression::MultilineOption);
    result.replace(hr, "<hr>");
    
    return result;
}

/**
 * @brief Парсит ссылки
 * @param text Текст для обработки
 * @return QString Текст с обработанными ссылками
 */
QString MarkdownParser::parseLinks(const QString& text)
{
    QString result = text;
    
    // Ссылки [текст](url)
    QRegularExpression link("\\[([^\\]]+)\\]\\(([^)]+)\\)");
    result.replace(link, "<a href=\"\\2\">\\1</a>");
    
    return result;
}

/**
 * @brief Парсит изображения
 * @param text Текст для обработки
 * @return QString Текст с обработанными изображениями
 */
QString MarkdownParser::parseImages(const QString& text)
{
    QString result = text;
    
    // Изображения ![альт](url)
    QRegularExpression image("!\\[([^\\]]*)\\]\\(([^)]+)\\)");
    result.replace(image, "<img src=\"\\2\" alt=\"\\1\">");
    
    return result;
}

/**
 * @brief Парсит параграфы (обычный текст)
 * @param text Текст для обработки
 * @return QString Текст с обработанными параграфами
 */
QString MarkdownParser::parseParagraphs(const QString& text)
{
    QString result = text;
    QStringList paragraphs = result.split("\n\n");
    QString processed;
    
    for (const QString& para : paragraphs) {
        QString trimmed = para.trimmed();
        if (!trimmed.isEmpty() && 
            !trimmed.startsWith("<h") &&
            !trimmed.startsWith("<ul") &&
            !trimmed.startsWith("<ol") &&
            !trimmed.startsWith("<li") &&
            !trimmed.startsWith("<blockquote") &&
            !trimmed.startsWith("<pre") &&
            !trimmed.startsWith("<hr")) {
            processed += "<p>" + trimmed + "</p>";
        } else {
            processed += trimmed;
        }
        processed += "\n";
    }
    
    return processed;
}
