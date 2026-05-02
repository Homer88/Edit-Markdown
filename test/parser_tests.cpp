#include <QTest>
#include <QString>
#include <QDebug>
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>

// Подключаем реальный парсер из проекта
#include "../markdownparser.h"

QString markdownToHtml(const QString &markdown) {
    MarkdownParser parser;
    return parser.parse(markdown);
}

class ParserTests : public QObject {
    Q_OBJECT
    
private slots:
    void testHeaders_data() {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("expectedPattern");

        QTest::newRow("H1") << "# Заголовок 1" << "<h1>Заголовок 1</h1>";
        QTest::newRow("H2") << "## Заголовок 2" << "<h2>Заголовок 2</h2>";
        QTest::newRow("H3") << "### Заголовок 3" << "<h3>Заголовок 3</h3>";
        QTest::newRow("H4") << "#### Заголовок 4" << "<h4>Заголовок 4</h4>";
        QTest::newRow("H5") << "##### Заголовок 5" << "<h5>Заголовок 5</h5>";
        QTest::newRow("H6") << "###### Заголовок 6" << "<h6>Заголовок 6</h6>";
    }

    void testHeaders() {
        QFETCH(QString, input);
        QFETCH(QString, expectedPattern);

        QString result = markdownToHtml(input);
        QVERIFY2(result.contains(expectedPattern), 
                 qPrintable(QString("Expected '%1' in '%2'").arg(expectedPattern, result)));
        qDebug() << "Input:" << input;
        qDebug() << "Result:" << result;
    }

    void testNewlines_data() {
        QTest::addColumn<QString>("input");
        QTest::addColumn<int>("brCount");

        QTest::newRow("Single newline") << "line1\nline2" << 1;
        QTest::newRow("Multiple newlines") << "line1\nline2\nline3" << 2;
    }

    void testNewlines() {
        QFETCH(QString, input);
        QFETCH(int, brCount);

        QString result = markdownToHtml(input);
        int actualCount = result.count("<br>");
        QCOMPARE(actualCount, brCount);
        qDebug() << "Input:" << input;
        qDebug() << "Result:" << result;
    }

    void testParagraphs_data() {
        QTest::addColumn<QString>("input");
        QTest::addColumn<int>("pCount");

        QTest::newRow("Single paragraph") << "Text" << 1;
        QTest::newRow("Two paragraphs") << "Text1\n\nText2" << 2;
    }

    void testParagraphs() {
        QFETCH(QString, input);
        QFETCH(int, pCount);

        QString result = markdownToHtml(input);
        int actualCount = result.count("<p>");
        QCOMPARE(actualCount, pCount);
        qDebug() << "Input:" << input;
        qDebug() << "Result:" << result;
    }
    
    void testComplexContent() {
        QString input = "## Заголовок\naaaa\nasd\nasd\n\nНовый абзац";
        QString result = markdownToHtml(input);
        
        QVERIFY(result.contains("<h2>Заголовок</h2>"));
        QVERIFY(result.contains("<br>")); // Должны быть переносы строк
        QVERIFY(result.contains("<p>")); // Должны быть абзацы
        QVERIFY(result.contains("Новый абзац"));
        
        qDebug() << "Complex input:" << input;
        qDebug() << "Complex result:" << result;
    }

    // ==================== ТЕСТЫ ДЛЯ CODE ====================
    
    void testInlineCode() {
        QString input = "Это `inline code` в тексте";
        QString result = markdownToHtml(input);
        
        QVERIFY(result.contains("<code>inline code</code>"));
        qDebug() << "Inline code result:" << result;
    }
    
    void testCodeBlock() {
        QString input = "```\nint main() {\n    return 0;\n}\n```";
        QString result = markdownToHtml(input);
        
        QVERIFY(result.contains("<pre><code") || result.contains("<pre><code>"));
        QVERIFY(result.contains("int main()"));
        QVERIFY(result.contains("return 0"));
        QVERIFY(result.contains("</code></pre>"));
        qDebug() << "Code block result:" << result;
    }
    
    void testCodeBlockWithLanguage() {
        QString input = "```cpp\nint main() {\n    return 0;\n}\n```";
        QString result = markdownToHtml(input);
        
        QVERIFY(result.contains("<pre><code") || result.contains("<pre><code>"));
        QVERIFY(result.contains("cpp"));
        QVERIFY(result.contains("int main()"));
        qDebug() << "Code block with language result:" << result;
    }
    
    void testCodeBlockAndInlineCodeMixed() {
        QString input = "Текст с `inline` кодом и блоком:\n```\ncode block\n```\nи еще `inline`";
        QString result = markdownToHtml(input);
        
        // Должны быть оба типа кода
        QVERIFY(result.contains("<code>inline</code>"));
        QVERIFY(result.contains("<pre><code") || result.contains("<pre><code>"));
        QVERIFY(result.contains("code block"));
        qDebug() << "Mixed code result:" << result;
    }
    
    void testMultipleCodeBlocks() {
        QString input = "```\nblock 1\n```\nтекст между\n```\nblock 2\n```";
        QString result = markdownToHtml(input);
        
        int preCount = result.count("<pre><code");
        QCOMPARE(preCount, 2);
        QVERIFY(result.contains("block 1"));
        QVERIFY(result.contains("block 2"));
        qDebug() << "Multiple code blocks result:" << result;
    }

    // ==================== ТЕСТЫ ДЛЯ BLOCKQUOTES ====================
    
    void testSimpleBlockquote() {
        QString input = "> Это цитата";
        QString result = markdownToHtml(input);
        
        QVERIFY(result.contains("<blockquote>"));
        QVERIFY(result.contains("Это цитата"));
        QVERIFY(result.contains("</blockquote>"));
        qDebug() << "Simple blockquote result:" << result;
    }
    
    void testMultilineBlockquote() {
        QString input = "> Строка 1\n> Строка 2\n> Строка 3";
        QString result = markdownToHtml(input);
        
        QVERIFY(result.contains("<blockquote>"));
        QVERIFY(result.contains("Строка 1"));
        QVERIFY(result.contains("Строка 2"));
        QVERIFY(result.contains("Строка 3"));
        // Проверяем, что строки цитаты разделены <br>
        QVERIFY(result.contains("<br>"));
        qDebug() << "Multiline blockquote result:" << result;
    }
    
    void testBlockquoteWithEmptyLine() {
        QString input = "> Цитата 1\n>\n> Цитата 2";
        QString result = markdownToHtml(input);
        
        QVERIFY(result.contains("<blockquote>"));
        QVERIFY(result.contains("Цитата 1"));
        QVERIFY(result.contains("Цитата 2"));
        qDebug() << "Blockquote with empty line result:" << result;
    }
    
    void testBlockquoteWithHeader() {
        QString input = "> #### Заголовок в цитате\n> текст";
        QString result = markdownToHtml(input);
        
        QVERIFY(result.contains("<blockquote>"));
        QVERIFY(result.contains("<h4>Заголовок в цитате</h4>"));
        qDebug() << "Blockquote with header result:" << result;
    }
    
    void testNestedBlockquotes() {
        QString input = "> Уровень 1\n>> Уровень 2";
        QString result = markdownToHtml(input);
        
        QVERIFY(result.contains("<blockquote>"));
        QVERIFY(result.contains("Уровень 1"));
        QVERIFY(result.contains("Уровень 2"));
        qDebug() << "Nested blockquotes result:" << result;
    }
    
    void testBlockquoteWithCode() {
        QString input = "> Код в цитате: `code`";
        QString result = markdownToHtml(input);
        
        QVERIFY(result.contains("<blockquote>"));
        QVERIFY(result.contains("<code>code</code>"));
        qDebug() << "Blockquote with code result:" << result;
    }

    // ==================== ТЕСТЫ ДЛЯ TABLES ====================
    
    void testSimpleTable() {
        QString input = "| Header 1 | Header 2 |\n|----------|----------|\n| Cell 1   | Cell 2   |";
        QString result = markdownToHtml(input);
        
        // Таблицы должны сохраняться в HTML
        QVERIFY(result.contains("Header 1"));
        QVERIFY(result.contains("Header 2"));
        QVERIFY(result.contains("Cell 1"));
        QVERIFY(result.contains("Cell 2"));
        qDebug() << "Simple table result:" << result;
    }
    
    void testTableWithFormatting() {
        QString input = "| Left | Right |\n|:-----|------:|\n| **bold** | *italic* |";
        QString result = markdownToHtml(input);
        
        QVERIFY(result.contains("<b>bold</b>"));
        QVERIFY(result.contains("<i>italic</i>"));
        qDebug() << "Table with formatting result:" << result;
    }

    // ==================== ТЕСТЫ ДЛЯ MARKDOWN->WYSIWYG->MARKDOWN ====================
    
    void testMarkdownToHtmlToMarkdown_SimpleText() {
        QString originalMarkdown = "Простой текст без форматирования";
        
        // Markdown -> HTML (WYSIWYG)
        MarkdownParser parser;
        QString html = parser.parse(originalMarkdown);
        QVERIFY(!html.isEmpty());
        
        // HTML -> Markdown (обратно)
        QString backToMarkdown = parser.htmlToMarkdown(html);
        
        // Текст должен сохраниться
        QVERIFY(backToMarkdown.contains("Простой текст без форматирования"));
        qDebug() << "Original:" << originalMarkdown;
        qDebug() << "HTML:" << html;
        qDebug() << "Back to Markdown:" << backToMarkdown;
    }
    
    void testMarkdownToHtmlToMarkdown_WithHeaders() {
        QString originalMarkdown = "# Заголовок 1\n## Заголовок 2\nТекст под заголовком";
        
        MarkdownParser parser;
        QString html = parser.parse(originalMarkdown);
        QString backToMarkdown = parser.htmlToMarkdown(html);
        
        // Основной текст должен сохраниться
        QVERIFY(backToMarkdown.contains("Заголовок 1"));
        QVERIFY(backToMarkdown.contains("Заголовок 2"));
        QVERIFY(backToMarkdown.contains("Текст под заголовком"));
        qDebug() << "Original:" << originalMarkdown;
        qDebug() << "HTML:" << html;
        qDebug() << "Back to Markdown:" << backToMarkdown;
    }
    
    void testMarkdownToHtmlToMarkdown_MultipleRounds() {
        QString originalMarkdown = "# Заголовок\nТекст\n\n## Другой заголовок\nЕще текст";
        
        MarkdownParser parser;
        QString current = originalMarkdown;
        
        // Многократное переключение Markdown->HTML->Markdown
        for (int i = 0; i < 3; ++i) {
            QString html = parser.parse(current);
            current = parser.htmlToMarkdown(html);
            qDebug() << "Round" << i+1 << "Markdown:" << current;
        }
        
        // Ключевые слова должны сохраниться после всех переключений
        QVERIFY(current.contains("Заголовок"));
        QVERIFY(current.contains("Текст"));
        QVERIFY(current.contains("Другой заголовок"));
        QVERIFY(current.contains("Еще текст"));
    }
    
    void testMarkdownToHtmlToMarkdown_WithLists() {
        QString originalMarkdown = "- Item 1\n- Item 2\n- Item 3";
        
        MarkdownParser parser;
        QString html = parser.parse(originalMarkdown);
        QString backToMarkdown = parser.htmlToMarkdown(html);
        
        // Элементы списка должны сохраниться
        QVERIFY(backToMarkdown.contains("Item 1"));
        QVERIFY(backToMarkdown.contains("Item 2"));
        QVERIFY(backToMarkdown.contains("Item 3"));
        qDebug() << "Original:" << originalMarkdown;
        qDebug() << "HTML:" << html;
        qDebug() << "Back to Markdown:" << backToMarkdown;
    }
    
    void testMarkdownToHtmlToMarkdown_WithCode() {
        QString originalMarkdown = "Код: `inline` и блок\n```\ncode block\n```";
        
        MarkdownParser parser;
        QString html = parser.parse(originalMarkdown);
        QString backToMarkdown = parser.htmlToMarkdown(html);
        
        // Код должен сохраниться (хотя бы текст)
        QVERIFY(backToMarkdown.contains("inline"));
        QVERIFY(backToMarkdown.contains("code block"));
        qDebug() << "Original:" << originalMarkdown;
        qDebug() << "HTML:" << html;
        qDebug() << "Back to Markdown:" << backToMarkdown;
    }
    
    void testMarkdownToHtmlToMarkdown_WithBlockquotes() {
        QString originalMarkdown = "> Цитата 1\n> Цитата 2";
        
        MarkdownParser parser;
        QString html = parser.parse(originalMarkdown);
        QString backToMarkdown = parser.htmlToMarkdown(html);
        
        // Текст цитаты должен сохраниться
        QVERIFY(backToMarkdown.contains("Цитата 1"));
        QVERIFY(backToMarkdown.contains("Цитата 2"));
        qDebug() << "Original:" << originalMarkdown;
        qDebug() << "HTML:" << html;
        qDebug() << "Back to Markdown:" << backToMarkdown;
    }
    
    void testMarkdownToHtmlToMarkdown_ComplexDocument() {
        // Загружаем тестовый файл
        QFile testFile("test.md");
        if (testFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&testFile);
            QString originalMarkdown = in.readAll();
            testFile.close();
            
            MarkdownParser parser;
            QString html = parser.parse(originalMarkdown);
            QString backToMarkdown = parser.htmlToMarkdown(html);
            
            // Основные элементы должны сохраниться
            QVERIFY(!backToMarkdown.isEmpty());
            QVERIFY(backToMarkdown.contains("Headers"));
            QVERIFY(backToMarkdown.contains("Lists"));
            QVERIFY(backToMarkdown.contains("Code"));
            
            qDebug() << "Complex document - Original length:" << originalMarkdown.length();
            qDebug() << "Complex document - HTML length:" << html.length();
            qDebug() << "Complex document - Back to Markdown length:" << backToMarkdown.length();
        } else {
            QSKIP("test.md file not found");
        }
    }
    
    void testMarkdownToHtmlToMarkdown_NoTextLoss() {
        // Тест на то, что текст не слипается в одну строку
        QString originalMarkdown = "Строка 1\nСтрока 2\nСтрока 3";
        
        MarkdownParser parser;
        QString html = parser.parse(originalMarkdown);
        QString backToMarkdown = parser.htmlToMarkdown(html);
        
        // Все строки должны присутствовать
        QVERIFY(backToMarkdown.contains("Строка 1"));
        QVERIFY(backToMarkdown.contains("Строка 2"));
        QVERIFY(backToMarkdown.contains("Строка 3"));
        
        // Текст не должен быть слит в одну строку без разделителей
        QVERIFY(!backToMarkdown.contains("Строка 1Строка 2"));
        qDebug() << "Original:" << originalMarkdown;
        qDebug() << "Back to Markdown:" << backToMarkdown;
    }
};

QTEST_APPLESS_MAIN(ParserTests)
#include "parser_tests.moc"
