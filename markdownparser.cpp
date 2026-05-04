#include "markdownparser.h"
#include <QRegularExpression>
#include <QDebug>
#include <QTextDocument>

static QString decodeHtmlEntities(const QString& text) {
    QString result = text;
    result.replace("&amp;", "&");
    result.replace("&lt;", "<");
    result.replace("&gt;", ">");
    result.replace("&quot;", "\"");
    result.replace("&#39;", "'");
    result.replace("&apos;", "'");
    result.replace("&nbsp;", " ");
    return result;
}

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
    QString result = markdown;
    
    // Сохраняем блоки кода
    result = preserveCodeBlocks(result);
    
    // Применяем все замены
    result = parseHeaders(result);
    result = parseBold(result);
    result = parseItalic(result);
    result = parseStrikeThrough(result);
    result = parseLists(result);
    result = parseCode(result);
    result = parseBlockquotes(result);
    result = parseHorizontalRules(result);
    result = parseLinks(result);
    result = parseImages(result);
    result = parseTables(result);
    result = parseEscapes(result);
    
    // Восстанавливаем блоки кода
    result = restoreAndParseCode(result);
    
    return result;
}

QTextDocument* MarkdownParser::parseToDocument(const QString& markdown)
{
    QTextDocument* doc = new QTextDocument();
    QStringList lines = markdown.split('\n');
    
    QTextCursor cursor(doc);
    bool inTable = false;
    QStringList tableRows;
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        
        // Проверяем, является ли строка строкой таблицы
        QRegularExpression tableRow("^\\|(.*)\\|$");
        QRegularExpressionMatch match = tableRow.match(line);
        
        if (match.hasMatch()) {
            if (!inTable) {
                inTable = true;
                tableRows.clear();
            }
            tableRows.append(line);
        } else {
            if (inTable) {
                // Конец таблицы - обрабатываем
                if (tableRows.size() >= 2) {
                    // Создаём таблицу
                    QTextTable* table = cursor.insertTable(tableRows.size() - 1,  // минус строка-разделитель
                                                          tableRows[0].split('|', Qt::SkipEmptyParts).size());
                    
                    int rowIdx = 0;
                    for (int j = 0; j < tableRows.size(); ++j) {
                        QString row = tableRows[j];
                        // Пропускаем строку-разделитель
                        if (row.contains(QRegularExpression("^\\|[\\s:]*-+[\\s:|]*\\|$"))) {
                            continue;
                        }
                        
                        QStringList cells = row.split('|', Qt::SkipEmptyParts);
                        for (int k = 0; k < cells.size(); ++k) {
                            QTextTableCell cell = table->cellAt(rowIdx, k);
                            QTextCursor cellCursor = cell.firstCursorPosition();
                            
                            // Применяем форматирование к ячейке
                            QString cellText = cells[k].trimmed();
                            processInline(cellCursor, cellText, 0);
                            
                            // Если это первая строка (заголовок), делаем жирным
                            if (rowIdx == 0) {
                                QTextCharFormat fmt;
                                fmt.setFontWeight(QFont::Bold);
                                cellCursor.mergeCharFormat(fmt);
                            }
                        }
                        ++rowIdx;
                    }
                }
                inTable = false;
                tableRows.clear();
            }
            
            // Обычный текст - применяем парсинг
            if (!line.trimmed().isEmpty()) {
                cursor.insertBlock();
                processInline(cursor, line, 0);
            }
        }
    }
    
    // Если таблица в конце файла
    if (inTable && tableRows.size() >= 2) {
        // Аналогично созданию таблицы выше
        QTextTable* table = cursor.insertTable(tableRows.size() - 1,  // минус строка-разделитель
                                                  tableRows[0].split('|', Qt::SkipEmptyParts).size());
        
        int rowIdx = 0;
        for (int j = 0; j < tableRows.size(); ++j) {
            QString row = tableRows[j];
            if (row.contains(QRegularExpression("^\\|[\\s:]*-+[\\s:|]*\\|$"))) {
                continue;
            }
            
            QStringList cells = row.split('|', Qt::SkipEmptyParts);
            for (int k = 0; k < cells.size(); ++k) {
                QTextTableCell cell = table->cellAt(rowIdx, k);
                QTextCursor cellCursor = cell.firstCursorPosition();
                QString cellText = cells[k].trimmed();
                processInline(cellCursor, cellText, 0);
                
                if (rowIdx == 0) {
                    QTextCharFormat fmt;
                    fmt.setFontWeight(QFont::Bold);
                    cellCursor.mergeCharFormat(fmt);
                }
            }
            ++rowIdx;
        }
    }
    
    return doc;
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
        
        // Проверяем, является ли строка блочным элементом HTML или плейсхолдером кода
        bool isBlockElement = trimmed.startsWith("<h") ||
                              trimmed.startsWith("<ul") ||
                              trimmed.startsWith("<ol") ||
                              trimmed.startsWith("<li") ||
                              trimmed.startsWith("<blockquote") ||
                              trimmed.startsWith("</blockquote") ||
                              trimmed.startsWith("</ul") ||
                              trimmed.startsWith("</ol") ||
                              trimmed.startsWith("<pre") ||
                              trimmed.startsWith("</pre") ||
                              trimmed.startsWith("<hr") ||
                              trimmed.startsWith("<table") ||
                              trimmed.startsWith("</table") ||
                              trimmed.startsWith("<th") ||
                              trimmed.startsWith("</tr") ||
                              trimmed.startsWith("<td") ||
                              trimmed.startsWith("</td") ||
                              trimmed.startsWith("</th") ||
                              trimmed.startsWith("%%_CODEBLOCK_") ||
                              trimmed.startsWith("%%_INDENTEDCODE_") ||
                              trimmed.startsWith("%%_INLINECODE_");
        
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
        
        // Проверяем, является ли строка разделителем таблицы (содержит только |, -, :, пробелы)
        QRegularExpression tableSeparator("^\\|?[\\s:]*-+[\\s:|-]*$");
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
                        if (row.contains(QRegularExpression("^\\|[\\s:]*-+[\\s:|-]*\\|$"))) {
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
            if (row.contains(QRegularExpression("^\\|[\\s:]*-+[\\s:|-]*\\|$"))) {
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
 * @brief Экранирует специальные символы (обратный слэш)
 * @param text Текст для обработки
 * @return QString Текст с экранированными символами
 */
QString MarkdownParser::parseEscapes(const QString& text)
{
    QString result = text;
    
    // Сохраняем экранированные символы во временные плейсхолдеры
    // Используем простую строковую замену вместо regex для надежности
    result.replace("\\\\*", "ESCAPEDASTERISK");
    result.replace("\\\\#", "ESCAPEDHASH");
    result.replace("\\\\`", "ESCAPEDBACKTICK");
    result.replace("\\\\[", "ESCAPEDLBRACKET");
    result.replace("\\\\]", "ESCAPEDRBRACKET");
    result.replace("\\\\_", "ESCAPEDUNDERSCORE");
    result.replace("\\\\~", "ESCAPEDTILDE");
    result.replace("\\\\$", "ESCAPEDDOLLAR");
    result.replace("\\\\\\\\", "ESCAPEDBACKSLASH");
    
    return result;
}

/**
 * @brief Сохраняет блоки кода в специальные плейсхолдеры
 * @param text Текст для обработки
 * @return QString Текст с замененными блоками кода
 */
QString MarkdownParser::preserveCodeBlocks(const QString& text)
{
    QString result = text;
    
    // Сначала собираем все совпадения для code blocks, затем заменяем с конца
    QRegularExpression codeBlock("```([a-zA-Z]*)\\n?([\\s\\S]*?)```");
    QList<QPair<int, int>> codeBlockPositions;
    QList<QPair<QString, QString>> codeBlockData;
    
    QRegularExpressionMatchIterator it = codeBlock.globalMatch(text);
    int idx = 0;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        codeBlockPositions.append(qMakePair(match.capturedStart(), match.capturedLength()));
        codeBlockData.append(qMakePair(match.captured(1), match.captured(2)));
        idx++;
    }
    
    qDebug() << "[preserveCodeBlocks] Found" << codeBlockPositions.size() << "code blocks in input:" << text.left(50);
    
    // Заменяем с конца к началу чтобы позиции не сдвигались
    for (int i = codeBlockPositions.size() - 1; i >= 0; --i) {
        int pos = codeBlockPositions[i].first;
        int len = codeBlockPositions[i].second;
        QString lang = codeBlockData[i].first;
        QString code = codeBlockData[i].second;
        QString placeholder = QString("%%_CODEBLOCK_%1_%%").arg(i);
        qDebug() << "[preserveCodeBlocks] Replacing code block" << i << "at pos" << pos << "lang:" << lang << "code:" << code.left(30);
        QString htmlCode;
        if (lang.isEmpty()) {
            htmlCode = QString("<pre><code>%1</code></pre>").arg(code.toHtmlEscaped());
        } else {
            htmlCode = QString("<pre><code class=\"language-%1\">%2</code></pre>").arg(lang).arg(code.toHtmlEscaped());
        }
        codeBlocks_.insert(placeholder, htmlCode);
        result.replace(pos, len, placeholder);
    }
    
    // Сохраняем инлайн код `...` в плейсхолдеры
    QRegularExpression inlineCode("`([^`]+)`");
    QList<QPair<int, int>> inlineCodePositions;
    QList<QString> inlineCodeData;
    
    it = inlineCode.globalMatch(result);
    idx = 0;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        inlineCodePositions.append(qMakePair(match.capturedStart(), match.capturedLength()));
        inlineCodeData.append(match.captured(1));
        idx++;
    }
    
    // Заменяем с конца к началу
    for (int i = inlineCodePositions.size() - 1; i >= 0; --i) {
        int pos = inlineCodePositions[i].first;
        int len = inlineCodePositions[i].second;
        QString code = inlineCodeData[i];
        QString placeholder = QString("%%_INLINECODE_%1_%%").arg(i);
        inlineCodes_.insert(placeholder, QString("<code>%1</code>").arg(code.toHtmlEscaped()));
        result.replace(pos, len, placeholder);
    }
    
    // Сохраняем indented code blocks (4 пробела или таб)
    QStringList lines = result.split('\n');
    QString processed;
    bool inIndentedCode = false;
    QString indentedCodeContent;
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        if (line.startsWith("    ") || line.startsWith("\t")) {
            if (!inIndentedCode) {
                inIndentedCode = true;
                indentedCodeContent.clear();
            }
            // Удаляем первые 4 пробела или таб
            if (line.startsWith("    "))
                indentedCodeContent += line.mid(4) + "\n";
            else
                indentedCodeContent += line.mid(1) + "\n";
        } else {
            if (inIndentedCode) {
                // Конец блока кода
                if (!indentedCodeContent.trimmed().isEmpty()) {
                    QString placeholder = QString("%%_INDENTEDCODE_%1_%%").arg(codeBlocks_.size());
                    codeBlocks_.insert(placeholder, QString("<pre><code>%1</code></pre>").arg(indentedCodeContent.trimmed().toHtmlEscaped()));
                    processed += placeholder + "\n";
                }
                inIndentedCode = false;
                indentedCodeContent.clear();
            }
            processed += line + "\n";
        }
    }
    // Обрабатываем конец файла
    if (inIndentedCode && !indentedCodeContent.trimmed().isEmpty()) {
        QString placeholder = QString("%%_INDENTEDCODE_%1_%%").arg(codeBlocks_.size());
        codeBlocks_.insert(placeholder, QString("<pre><code>%1</code></pre>").arg(indentedCodeContent.trimmed().toHtmlEscaped()));
        processed += placeholder + "\n";
    }
    
    return processed;
}

/**
 * @brief Восстанавливает блоки кода и обрабатывает инлайн код
 * @param text Текст для обработки
 * @return QString Текст с восстановленным кодом
 */
QString MarkdownParser::restoreAndParseCode(const QString& text)
{
    QString result = text;
    
    qDebug() << "[restoreAndParseCode] Restoring" << codeBlocks_.size() << "code blocks and" << inlineCodes_.size() << "inline codes";
    
    // Восстанавливаем блоки кода
    QMapIterator<QString, QString> it(codeBlocks_);
    while (it.hasNext()) {
        it.next();
        qDebug() << "[restoreAndParseCode] Replacing" << it.key() << "with:" << it.value().left(50);
        result.replace(it.key(), it.value());
    }
    codeBlocks_.clear();
    
    // Восстанавливаем инлайн код
    QMapIterator<QString, QString> it2(inlineCodes_);
    while (it2.hasNext()) {
        it2.next();
        result.replace(it2.key(), it2.value());
    }
    inlineCodes_.clear();
    
    qDebug() << "[restoreAndParseCode] Final result:" << result.left(100);
    return result;
}

/**
 * @brief Парсит математику LaTeX
 * @param text Текст для обработки
 * @return QString Текст с обработанной математикой
 */
QString MarkdownParser::parseMath(const QString& text)
{
    QString result = text;
    
    // Блочная математика $$...$$
    QRegularExpression displayMath("\\$\\$([\\s\\S]*?)\\$\\$");
    result.replace(displayMath, "<span class=\"math-display\">$$\\1$$</span>");
    
    // Инлайн математика $...$ (но не внутри слов)
    QRegularExpression inlineMath("\\$([^$]+?)\\$");
    result.replace(inlineMath, "<span class=\"math-inline\">$\\1$</span>");
    
    return result;
}


/**
 * @brief Преобразует HTML обратно в Markdown (для синхронизации WYSIWYG -> Markdown)
 * @param html HTML текст для конвертации
 * @return QString Текст в формате Markdown
 */
QString MarkdownParser::htmlToMarkdown(const QString& html)
{
    QString result = html;
    
    // 0. Удаляем <style> и <script> блоки целиком, чтобы CSS/JS не попали в Markdown
    result.replace(QRegularExpression("<style[^>]*>[\\s\\S]*?</style>", QRegularExpression::DotMatchesEverythingOption), "");
    result.replace(QRegularExpression("<script[^>]*>[\\s\\S]*?</script>", QRegularExpression::DotMatchesEverythingOption), "");
    result.replace(QRegularExpression("<html[^>]*>|</html>", QRegularExpression::DotMatchesEverythingOption), "");
    result.replace(QRegularExpression("<head[^>]*>|</head>", QRegularExpression::DotMatchesEverythingOption), "");
    result.replace(QRegularExpression("<body[^>]*>|</body>", QRegularExpression::DotMatchesEverythingOption), "");
    
    // 1. Сохраняем блоки кода <pre><code>...</code></pre> и <pre><code class="...">...</code></pre> как ```...```
    QRegularExpression preCode("<pre><code[^>]*>([\\s\\S]*?)</code></pre>", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator preCodeIter = preCode.globalMatch(result);
    while (preCodeIter.hasNext()) {
        QRegularExpressionMatch match = preCodeIter.next();
        QString codeContent = decodeHtmlEntities(match.captured(1));
        QString replacement = "```\n" + codeContent + "\n```";
        result.replace(match.capturedStart(), match.capturedLength(), replacement);
    }
    
    // 2. Сохраняем инлайн код <code>...</code> как `...`
    QRegularExpression inlineCode("<code>([^<]+)</code>");
    QRegularExpressionMatchIterator inlineCodeIter = inlineCode.globalMatch(result);
    while (inlineCodeIter.hasNext()) {
        QRegularExpressionMatch match = inlineCodeIter.next();
        QString codeContent = decodeHtmlEntities(match.captured(1));
        QString replacement = "`" + codeContent + "`";
        result.replace(match.capturedStart(), match.capturedLength(), replacement);
    }
    
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
        
        // Парсим строки таблицы и считаем колонки
        QStringList rows;
        int colCount = 0;
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
                if (colCount == 0) {
                    colCount = cells.size();
                }
                rows << "|" + cells.join("|") + "|";
            }
        }
        
        // Формируем markdown таблицу
        if (!rows.isEmpty()) {
            QString markdownTable;
            
            for (int i = 0; i < rows.size(); ++i) {
                markdownTable += rows[i] + "\n";
                if (i == 0) {
                    // Добавляем разделитель после заголовка
                    if (colCount > 0) {
                        QStringList separators;
                        for (int c = 0; c < colCount; ++c) {
                            separators << "---";
                        }
                        markdownTable += "|" + separators.join("|") + "|\n";
                    }
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
    // Обрабатываем вложенные blockquote
    while (result.contains("<blockquote")) {
        QRegularExpression bqOpen("<blockquote[^>]*>");
        QRegularExpressionMatch openMatch = bqOpen.match(result);
        if (!openMatch.hasMatch()) break;
        
        int openPos = openMatch.capturedStart();
        
        // Находим соответствующий закрывающий тег
        int closePos = result.indexOf("</blockquote>", openPos);
        if (closePos < 0) break;
        
        // Извлекаем содержимое blockquote
        QString content = result.mid(openPos + openMatch.capturedLength(), closePos - openPos - openMatch.capturedLength());
        
        // Разбиваем содержимое на строки и добавляем > к каждой
        QStringList contentLines = content.split('\n');
        QString quotedContent;
        for (const QString& line : contentLines) {
            QString trimmed = line.trimmed();
            if (!trimmed.isEmpty()) {
                // Удаляем <br> и <p> теги из содержимого цитаты
                QString cleanLine = trimmed;
                cleanLine.replace(QRegularExpression("<br[^>]*>"), "\n");
                cleanLine.replace(QRegularExpression("<p[^>]*>"), "");
                cleanLine.replace(QRegularExpression("</p>"), "");
                quotedContent += "> " + cleanLine + "\n";
            } else {
                quotedContent += ">\n";
            }
        }
        
        result.replace(openPos, closePos + 13 - openPos, "\n" + quotedContent + "\n");
    }
    
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
    
    // 15.5. Математика - восстанавливаем LaTeX формулы
    QRegularExpression displayMath("<span class=\"math-display\">\\$\\$([\\s\\S]*?)\\$\\$</span>");
    result.replace(displayMath, "\n$$\\1$$\n");
    QRegularExpression inlineMath("<span class=\"math-inline\">\\$([^$]+?)\\$</span>");
    result.replace(inlineMath, "$\\1$");
    
    // 16. Удаляем оставшиеся HTML теги (span, div и т.д.)
    QRegularExpression remainingTags("<(?!/?(?:h[1-6]|b|strong|i|em|s|strike|del|a|img|ul|ol|li|blockquote|pre|code|table|tr|td|th)[^>]*>)[^>]+>");
    result.replace(remainingTags, "");
    
    // Также удаляем теги параграфов
    result.replace(QRegularExpression("<p[^>]*>(.+?)</p>"), "\\1\n");
    result.replace(QRegularExpression("</?p>"), "");
    
    // Удаляем маркеры списков
    result.replace(QRegularExpression("<UL_START>"), "");
    result.replace(QRegularExpression("<UL_END>"), "");
    result.replace(QRegularExpression("<OL_START>"), "");
    result.replace(QRegularExpression("<OL_END>"), "");
    
    // Восстанавливаем сохранённые таблицы
    for (int i = 0; i < tables.size(); ++i) {
        result.replace(QString("\n<TABLE_%1>\n").arg(i), "\n" + tables[i] + "\n");
    }
    
    // Очищаем лишние пустые строки (более 2 подряд)
    result.replace(QRegularExpression("\\n{3,}"), "\n\n");
    
    // Trim результата
    return result.trimmed();
}
