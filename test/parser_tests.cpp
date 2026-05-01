#include <QTest>
#include <QString>
#include <QDebug>
#include <QRegularExpression>

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
};

QTEST_APPLESS_MAIN(ParserTests)
#include "parser_tests.moc"
