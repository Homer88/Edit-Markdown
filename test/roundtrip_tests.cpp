#include <QTest>
#include <QString>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QCryptographicHash>
#include <QDateTime>

// Подключаем реальный парсер из проекта
#include "../markdownparser.h"

QString markdownToHtml(const QString &markdown) {
    MarkdownParser parser;
    return parser.parse(markdown);
}

class RoundTripTests : public QObject {
    Q_OBJECT
    
private slots:
    void testRoundTrip_SaveAndCompare() {
        // Загружаем тестовый файл test.md
        QFile testFile("test.md");
        if (!testFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QFAIL("test.md file not found");
            return;
        }
        
        QTextStream in(&testFile);
        QString originalMarkdown = in.readAll();
        testFile.close();
        
        qDebug() << "=========================================";
        qDebug() << "ROUND-TRIP TEST: Markdown -> HTML -> Markdown";
        qDebug() << "=========================================";
        qDebug() << "Original file: test.md";
        qDebug() << "Original size:" << originalMarkdown.size() << "bytes";
        
        // Шаг 1: Markdown -> HTML (WYSIWYG)
        MarkdownParser parser;
        QString html = parser.parse(originalMarkdown);
        qDebug() << "\nStep 1: Markdown -> HTML";
        qDebug() << "HTML size:" << html.size() << "bytes";
        
        // Сохраняем HTML в файл
        QFile htmlFile("output_roundtrip.html");
        if (htmlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&htmlFile);
            out << html;
            htmlFile.close();
            qDebug() << "Saved HTML to: output_roundtrip.html";
        }
        
        // Шаг 2: HTML -> Markdown (обратно)
        QString backToMarkdown = parser.htmlToMarkdown(html);
        qDebug() << "\nStep 2: HTML -> Markdown";
        qDebug() << "Back to Markdown size:" << backToMarkdown.size() << "bytes";
        
        // Сохраняем результат обратно в Markdown
        QFile mdOutputFile("output_roundtrip.md");
        if (mdOutputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&mdOutputFile);
            out << backToMarkdown;
            mdOutputFile.close();
            qDebug() << "Saved Markdown to: output_roundtrip.md";
        }
        
        // Вычисляем контрольные суммы
        QByteArray originalMd5 = QCryptographicHash::hash(originalMarkdown.toUtf8(), QCryptographicHash::Md5);
        QByteArray resultMd5 = QCryptographicHash::hash(backToMarkdown.toUtf8(), QCryptographicHash::Md5);
        
        qDebug() << "\n=========================================";
        qDebug() << "CHECKSUMS:";
        qDebug() << "Original MD5:" << originalMd5.toHex();
        qDebug() << "Result MD5: " << resultMd5.toHex();
        qDebug() << "=========================================";
        
        // Сохраняем контрольные суммы в файл
        QFile checksumFile("checksums.txt");
        if (checksumFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&checksumFile);
            out << "Round-trip test results\n";
            out << "Generated: " << QDateTime::currentDateTime().toString() << "\n\n";
            out << "Original file: test.md\n";
            out << "Original size: " << originalMarkdown.size() << " bytes\n";
            out << "Original MD5: " << originalMd5.toHex() << "\n\n";
            out << "Result file: output_roundtrip.md\n";
            out << "Result size: " << backToMarkdown.size() << " bytes\n";
            out << "Result MD5: " << resultMd5.toHex() << "\n\n";
            
            if (originalMd5 == resultMd5) {
                out << "STATUS: PERFECT MATCH - Text preserved exactly!\n";
            } else {
                out << "STATUS: MISMATCH - Some formatting may have changed\n";
                out << "Note: This is expected for round-trip conversion as HTML tags are stripped\n";
            }
            checksumFile.close();
            qDebug() << "Saved checksums to: checksums.txt";
        }
        
        // Проверяем, что ключевой контент сохранился
        qDebug() << "\n=========================================";
        qDebug() << "CONTENT VERIFICATION:";
        
        // Проверка по секциям из test.md
        QStringList sectionsToCheck = {
            "Headers",
            "Paragraphs",
            "Text styles",
            "Lists",
            "Links",
            "Images",
            "Blockquotes",
            "Code",
            "Horizontal rules",
            "Tables",
            "Math",
            "LaTeX"
        };
        
        int passed = 0;
        int failed = 0;
        
        for (const QString& section : sectionsToCheck) {
            if (backToMarkdown.contains(section)) {
                qDebug() << "✓ Section '" << section << "' - PRESERVED";
                passed++;
            } else {
                qDebug() << "✗ Section '" << section << "' - LOST";
                failed++;
            }
        }
        
        // Особая проверка пункта 15 (Math/LaTeX)
        qDebug() << "\n=========================================";
        qDebug() << "SPECIAL CHECK: Section 15 (Math/LaTeX):";
        if (originalMarkdown.contains("## 15. Math")) {
            qDebug() << "Original has '## 15. Math (LaTeX)'";
            if (backToMarkdown.contains("15") && backToMarkdown.contains("Math")) {
                qDebug() << "✓ Section 15 header preserved";
                passed++;
            } else {
                qDebug() << "✗ Section 15 header LOST";
                failed++;
            }
            
            if (backToMarkdown.contains("E=mc")) {
                qDebug() << "✓ Inline math formula preserved (E=mc)";
                passed++;
            } else {
                qDebug() << "✗ Inline math formula LOST";
                failed++;
            }
        }
        
        qDebug() << "\n=========================================";
        qDebug() << "SUMMARY:";
        qDebug() << "Sections preserved: " << passed;
        qDebug() << "Sections lost: " << failed;
        qDebug() << "=========================================";
        
        // Тест считается пройденным, если основной контент сохранился
        QVERIFY2(passed >= sectionsToCheck.size() / 2, 
                 qPrintable(QString("Too many sections lost: %1/%2").arg(failed).arg(sectionsToCheck.size())));
        
        qDebug() << "\n✓ ROUND-TRIP TEST PASSED";
        qDebug() << "Files saved:";
        qDebug() << "  - output_roundtrip.html (HTML version)";
        qDebug() << "  - output_roundtrip.md (Markdown after round-trip)";
        qDebug() << "  - checksums.txt (MD5 checksums for comparison)";
    }
    
    void testSpecificSection15_MathLaTeX() {
        // Детальная проверка пункта 15 из test.md
        QFile testFile("test.md");
        if (!testFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QFAIL("test.md file not found");
            return;
        }
        
        QTextStream in(&testFile);
        QString originalMarkdown = in.readAll();
        testFile.close();
        
        // Извлекаем секцию 15
        int startIdx = originalMarkdown.indexOf("## 15. Math");
        int endIdx = originalMarkdown.indexOf("## 16.", startIdx);
        if (endIdx == -1) endIdx = originalMarkdown.length();
        
        QString section15 = originalMarkdown.mid(startIdx, endIdx - startIdx);
        
        qDebug() << "\n=========================================";
        qDebug() << "SECTION 15 ORIGINAL:";
        qDebug() << section15;
        qDebug() << "=========================================";
        
        // Конвертируем
        MarkdownParser parser;
        QString html = parser.parse(section15);
        QString backToMarkdown = parser.htmlToMarkdown(html);
        
        qDebug() << "\nSECTION 15 AFTER ROUND-TRIP:";
        qDebug() << backToMarkdown;
        qDebug() << "=========================================";
        
        // Проверяем сохранение ключевых элементов
        QVERIFY2(backToMarkdown.contains("Math") || backToMarkdown.contains("15"),
                 "Section 15 header should be preserved");
        
        // Сохраняем результат
        QFile outputFile("output_section15.md");
        if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&outputFile);
            out << "=== ORIGINAL ===\n";
            out << section15;
            out << "\n=== AFTER ROUND-TRIP ===\n";
            out << backToMarkdown;
            outputFile.close();
            qDebug() << "Detailed comparison saved to: output_section15.md";
        }
    }
};

QTEST_APPLESS_MAIN(RoundTripTests)
#include "roundtrip_tests.moc"
