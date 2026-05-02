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
    // Сначала заголовки (включая альтернативные с === и ---), чтобы они не были затронуты другими правилами
    html = parseHeaders(html);
    // Затем горизонтальные линии (после заголовков, чтобы --- в заголовках не стали <hr>)
    html = parseHorizontalRules(html);
    html = parseCode(html);  // Код обрабатываем до цитат и таблиц
    html = parseBlockquotes(html);
    html = parseTables(html);  // Таблицы после кода
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
    
    // Сначала обрабатываем альтернативные заголовки (Setext-style) до того, как они будут затронуты другими правилами
    // Alt H1: текст с === на следующей строке
    // Используем более строгое регулярное выражение - захватываем только одну строку текста без HTML тегов
    QRegularExpression altH1("^([^<>\n]+?)\\n===+\\s*$", QRegularExpression::MultilineOption);
    result.replace(altH1, "<h1>\\1</h1>");
    
    // Alt H2: текст с --- на следующей строке
    // Используем более строгое регулярное выражение - захватываем только одну строку текста без HTML тегов
    QRegularExpression altH2("^([^<>\n]+?)\\n---+\\s*$", QRegularExpression::MultilineOption);
    result.replace(altH2, "<h2>\\1</h2>");
    
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
    
    // Блоки кода с ``` (должны обрабатываться первыми, до инлайн кода)
    QRegularExpression codeBlock("```([\\s\\S]*?)```");
    result.replace(codeBlock, "<pre><code>\\1</code></pre>");
    
    // Инлайн код с ` (обработка после блоков кода)
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
        QRegularExpression blockquoteLine("^>(.*)$");
        QRegularExpressionMatch match = blockquoteLine.match(line);
        
        if (match.hasMatch()) {
            if (!inBlockquote) {
                processed += "<blockquote>";
                inBlockquote = true;
            } else {
                // Добавляем перенос строки между строками цитаты
                processed += "<br>";
            }
            QString quoteContent = match.captured(1).trimmed();
            // Рекурсивно парсим содержимое цитаты для поддержки заголовков, кода и т.д.
            // Но не вызываем parseBlockquotes снова, чтобы избежать бесконечной рекурсии
            processed += parseHeaders(quoteContent) + "\n";
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
    QStringList lines = result.split('\n');
    QString processed;
    QString currentParagraph;
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        QString trimmed = line.trimmed();
        
        // Проверяем, является ли строка блочным элементом HTML
        bool isBlockElement = trimmed.startsWith("<h") ||
                              trimmed.startsWith("<ul") ||
                              trimmed.startsWith("<ol") ||
                              trimmed.startsWith("<li") ||
                              trimmed.startsWith("<blockquote") ||
                              trimmed.startsWith("<pre") ||
                              trimmed.startsWith("<hr");
        
        if (isBlockElement) {
            // Если есть накопленный параграф, закрываем его
            if (!currentParagraph.isEmpty()) {
                processed += "<p>" + currentParagraph + "</p>\n";
                currentParagraph.clear();
            }
            // Добавляем блочный элемент как есть
            processed += trimmed + "\n";
        } else if (trimmed.isEmpty()) {
            // Пустая строка - конец параграфа
            if (!currentParagraph.isEmpty()) {
                processed += "<p>" + currentParagraph + "</p>\n";
                currentParagraph.clear();
            }
        } else {
            // Обычная строка - добавляем в текущий параграф
            if (!currentParagraph.isEmpty()) {
                currentParagraph += "<br>" + trimmed;
            } else {
                currentParagraph = trimmed;
            }
        }
    }
    
    // Закрываем последний параграф, если он есть
    if (!currentParagraph.isEmpty()) {
        processed += "<p>" + currentParagraph + "</p>\n";
    }
    
    return processed;
}

/**
 * @brief Парсит таблицы в формате GFM (GitHub Flavored Markdown)
 * @param text Текст для обработки
 * @return QString Текст с обработанными таблицами
 */
QString MarkdownParser::parseTables(const QString& text)
{
    QStringList lines = text.split('\n');
    QString result;
    bool inTable = false;
    QStringList tableRows;
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        QRegularExpression tableRow("^\\|(.*)\\|$");
        QRegularExpressionMatch match = tableRow.match(line);
        
        // Проверяем, является ли строка разделителем таблицы (содержит | и -)
        QRegularExpression tableSeparator("^\\|?[\\s:|-]+\\|?[\\s:|-]*$");
        QRegularExpressionMatch sepMatch = tableSeparator.match(line.trimmed());
        
        if (match.hasMatch() && !sepMatch.hasMatch()) {
            // Это строка таблицы
            if (!inTable) {
                inTable = true;
                tableRows.clear();
            }
            tableRows << line;
        } else {
            if (inTable) {
                // Конец таблицы - обрабатываем накопленные строки
                if (tableRows.size() >= 2) {
                    // Есть как минимум заголовок и разделитель
                    result += "<table>\n";
                    
                    for (int j = 0; j < tableRows.size(); ++j) {
                        QString row = tableRows[j];
                        
                        // Пропускаем строку-разделитель
                        if (row.contains(QRegularExpression("^\\|[\\s:-]+\\|$"))) {
                            continue;
                        }
                        
                        // Извлекаем ячейки
                        QStringList cells;
                        QString rowContent = row.mid(1, row.length() - 2); // убираем крайние |
                        cells = rowContent.split('|');
                        
                        result += "<tr>";
                        for (const QString& cell : cells) {
                            QString cellContent = cell.trimmed();
                            // Определяем тип ячейки (заголовок или данные)
                            if (j == 0) {
                                result += "<th>" + cellContent + "</th>";
                            } else {
                                result += "<td>" + cellContent + "</td>";
                            }
                        }
                        result += "</tr>\n";
                    }
                    
                    result += "</table>\n";
                } else {
                    // Недостаточно строк для таблицы, выводим как есть
                    for (const QString& row : tableRows) {
                        result += row + "\n";
                    }
                }
                inTable = false;
                tableRows.clear();
            }
            result += line + "\n";
        }
    }
    
    // Обрабатываем таблицу в конце файла
    if (inTable && tableRows.size() >= 2) {
        result += "<table>\n";
        
        for (int j = 0; j < tableRows.size(); ++j) {
            QString row = tableRows[j];
            
            // Пропускаем строку-разделитель
            if (row.contains(QRegularExpression("^\\|[\\s:-]+\\|$"))) {
                continue;
            }
            
            // Извлекаем ячейки
            QStringList cells;
            QString rowContent = row.mid(1, row.length() - 2);
            cells = rowContent.split('|');
            
            result += "<tr>";
            for (const QString& cell : cells) {
                QString cellContent = cell.trimmed();
                if (j == 0) {
                    result += "<th>" + cellContent + "</th>";
                } else {
                    result += "<td>" + cellContent + "</td>";
                }
            }
            result += "</tr>\n";
        }
        
        result += "</table>\n";
    } else if (inTable) {
        for (const QString& row : tableRows) {
            result += row + "\n";
        }
    }
    
    return result.trimmed();
}


/**
 * @brief Преобразует HTML обратно в Markdown (для синхронизации WYSIWYG -> Markdown)
 * @param html HTML текст для конвертации
 * @return QString Текст в формате Markdown
 */
QString MarkdownParser::htmlToMarkdown(const QString& html)
{
    QString result = html;
    
    // 1. Сохраняем блоки кода <pre><code>...</code></pre> как ```...```
    QRegularExpression preCode("<pre><code>([\\s\\S]*?)</code></pre>", QRegularExpression::DotMatchesEverythingOption);
    result.replace(preCode, "```\n\\1\n```");
    
    // 2. Сохраняем инлайн код <code>...</code> как `...`
    QRegularExpression inlineCode("<code>([^<]+)</code>");
    result.replace(inlineCode, "`\\1`");
    
    // 3. Заголовки <h1>-<h6> → #, ##, ### и т.д.
    QRegularExpression h1("<h1[^>]*>(.+?)</h1>", QRegularExpression::DotMatchesEverythingOption);
    result.replace(h1, "# \\1");
    
    QRegularExpression h2("<h2[^>]*>(.+?)</h2>", QRegularExpression::DotMatchesEverythingOption);
    result.replace(h2, "## \\1");
    
    QRegularExpression h3("<h3[^>]*>(.+?)</h3>", QRegularExpression::DotMatchesEverythingOption);
    result.replace(h3, "### \\1");
    
    QRegularExpression h4("<h4[^>]*>(.+?)</h4>", QRegularExpression::DotMatchesEverythingOption);
    result.replace(h4, "#### \\1");
    
    QRegularExpression h5("<h5[^>]*>(.+?)</h5>", QRegularExpression::DotMatchesEverythingOption);
    result.replace(h5, "##### \\1");
    
    QRegularExpression h6("<h6[^>]*>(.+?)</h6>", QRegularExpression::DotMatchesEverythingOption);
    result.replace(h6, "###### \\1");
    
    // 4. Жирный текст <b>, <strong> → **...**
    QRegularExpression bold("<(?:b|strong)[^>]*>(.+?)</(?:b|strong)>", QRegularExpression::DotMatchesEverythingOption);
    result.replace(bold, "**\\1**");
    
    // 5. Курсив <i>, <em> → *...*
    QRegularExpression italic("<(?:i|em)[^>]*>(.+?)</(?:i|em)>", QRegularExpression::DotMatchesEverythingOption);
    result.replace(italic, "*\\1*");
    
    // 6. Зачеркнутый <s>, <strike>, <del> → ~~...~~
    QRegularExpression strike("<(?:s|strike|del)[^>]*>(.+?)</(?:s|strike|del)>", QRegularExpression::DotMatchesEverythingOption);
    result.replace(strike, "~~\\1~~");
    
    // 7. Ссылки <a href="url">text</a> → [text](url)
    QRegularExpression link("<a[^>]*href=\"([^\"]+)\"[^>]*>(.+?)</a>", QRegularExpression::DotMatchesEverythingOption);
    result.replace(link, "[\\2](\\1)");
    
    // 8. Изображения <img src="url" alt="alt"> → ![alt](url)
    QRegularExpression image("<img[^>]*src=\"([^\"]+)\"[^>]*alt=\"([^\"]*)\"[^>]*>");
    result.replace(image, "![\\2](\\1)");
    // Альтернативный порядок атрибутов
    QRegularExpression image2("<img[^>]*alt=\"([^\"]*)\"[^>]*src=\"([^\"]+)\"[^>]*>");
    result.replace(image2, "![\\1](\\2)");
    
    // 9. Таблицы - обрабатываем <table> блоки
    // Сначала сохраняем таблицы в специальные маркеры
    QRegularExpression table("<table[^>]*>([\\s\\S]*?)</table>", QRegularExpression::DotMatchesEverythingOption);
    QStringList tables;
    int tableIndex = 0;
    QRegularExpressionMatchIterator tableIter = table.globalMatch(result);
    while (tableIter.hasNext()) {
        QRegularExpressionMatch match = tableIter.next();
        QString tableContent = match.captured(1);
        
        // Парсим строки таблицы
        QStringList rows;
        QRegularExpression tr("<tr[^>]*>([\\s\\S]*?)</tr>", QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatchIterator trIter = tr.globalMatch(tableContent);
        
        while (trIter.hasNext()) {
            QRegularExpressionMatch trMatch = trIter.next();
            QString rowContent = trMatch.captured(1);
            
            // Извлекаем ячейки (th или td)
            QStringList cells;
            QRegularExpression cell("<t[hd][^>]*>([\\s\\S]*?)</t[hd]>", QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatchIterator cellIter = cell.globalMatch(rowContent);
            
            while (cellIter.hasNext()) {
                QRegularExpressionMatch cellMatch = cellIter.next();
                QString cellText = cellMatch.captured(1).trimmed();
                // Удаляем лишние HTML теги из ячейки
                cellText.replace(QRegularExpression("<br[^>]*>"), "  \n");
                cells << cellText;
            }
            
            if (!cells.isEmpty()) {
                rows << "|" + cells.join("|") + "|";
            }
        }
        
        // Формируем markdown таблицу
        if (!rows.isEmpty()) {
            QString markdownTable;
            int colCount = rows.first().count('|') - 2; // количество колонок
            
            for (int i = 0; i < rows.size(); ++i) {
                markdownTable += rows[i] + "\n";
                if (i == 0) {
                    // Добавляем разделитель после заголовка
                    markdownTable += "|" + QString("-|-").repeated(colCount) + "\n";
                }
            }
            
            // Заменяем таблицу на маркер
            result.replace(match.capturedStart(), match.capturedLength(), 
                          QString("\n<TABLE_%1>\n").arg(tableIndex));
            tables << markdownTable;
            tableIndex++;
        }
    }
    
    // 10. Цитаты <blockquote>...</blockquote> → обрабатываем построчно
    // Сначала заменяем opening/closing теги на маркеры
    result.replace(QRegularExpression("<blockquote[^>]*>"), "\n<BLOCKQUOTE_START>\n");
    result.replace(QRegularExpression("</blockquote>"), "\n<BLOCKQUOTE_END>\n");
    
    // 11. Списки - сначала маркированные
    result.replace(QRegularExpression("<ul[^>]*>"), "\n<UL_START>\n");
    result.replace(QRegularExpression("</ul>"), "\n<UL_END>\n");
    
    // 12. Нумерованные списки
    result.replace(QRegularExpression("<ol[^>]*>"), "\n<OL_START>\n");
    result.replace(QRegularExpression("</ol>"), "\n<OL_END>\n");
    
    // 13. Элементы списка <li> → - ...
    QRegularExpression listItem("<li[^>]*>(.+?)</li>", QRegularExpression::DotMatchesEverythingOption);
    result.replace(listItem, "- \\1\n");
    
    // 14. <br> → два пробела и перевод строки
    result.replace(QRegularExpression("<br[^>]*>"), "  \n");
    
    // 15. <hr> → ---
    result.replace(QRegularExpression("<hr[^>]*>"), "\n---\n");
    
    // 16. Удаляем оставшиеся HTML теги (span, div, p и т.д.)
    QRegularExpression remainingTags("<(?!/?(?:h[1-6]|b|strong|i|em|s|strike|del|a|img|ul|ol|li|blockquote|pre|code|table|tr|td|th)[^>]*>)[^>]+>");
    result.replace(remainingTags, "");
    
    // Также удаляем теги параграфов
    result.replace(QRegularExpression("<p[^>]*>(.+?)</p>"), "\\1\n");
    
    // 17. Теперь обрабатываем цитаты построчно
    QStringList lines = result.split('\n');
    QString processed;
    bool inBlockquote = false;
    int blockquoteLevel = 0;
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        QString trimmed = line.trimmed();
        
        if (trimmed == "<BLOCKQUOTE_START>") {
            inBlockquote = true;
            blockquoteLevel++;
            continue;
        }
        if (trimmed == "<BLOCKQUOTE_END>") {
            inBlockquote = false;
            if (blockquoteLevel > 0) blockquoteLevel--;
            continue;
        }
        if (trimmed == "<UL_START>" || trimmed == "<UL_END>" || 
            trimmed == "<OL_START>" || trimmed == "<OL_END>") {
            continue;
        }
        
        if (inBlockquote && !trimmed.isEmpty()) {
            // Добавляем > к каждой строке внутри цитаты
            QString prefix;
            for (int j = 0; j < blockquoteLevel; ++j) {
                prefix += "> ";
            }
            if (!trimmed.startsWith(">")) {
                line = prefix + trimmed;
            }
        }
        
        processed += line + "\n";
    }
    result = processed;
    
    // 18. Восстанавливаем таблицы
    for (int i = 0; i < tables.size(); ++i) {
        result.replace(QString("<TABLE_%1>").arg(i), "\n" + tables[i]);
    }
    
    // 19. Очищаем лишние пустые строки (более 2 подряд)
    result.replace(QRegularExpression("\\n{3,}"), "\n\n");
    
    // 20. Trim результата
    return result.trimmed();
}
