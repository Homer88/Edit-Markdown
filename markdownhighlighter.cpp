#include "markdownhighlighter.h"
#include <QTextDocument>
#include <QColor>
#include <QFont>

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // Настройка форматов
    keywordFormat.setForeground(QColor(0, 100, 200));
    keywordFormat.setFontWeight(QFont::Bold);

    headerFormat.setForeground(QColor(150, 0, 0));
    headerFormat.setFontWeight(QFont::Bold);
    headerFormat.setFontPointSize(14);

    boldFormat.setFontWeight(QFont::Bold);
    boldFormat.setForeground(QColor(0, 0, 0));

    italicFormat.setFontItalic(true);
    italicFormat.setForeground(QColor(80, 80, 80));

    codeFormat.setForeground(QColor(0, 128, 0));
    codeFormat.setFontFamily("Courier New");
    codeFormat.setBackground(QColor(245, 245, 245));

    linkFormat.setForeground(QColor(0, 0, 255));
    linkFormat.setFontUnderline(true);

    quoteFormat.setForeground(QColor(100, 100, 100));
    quoteFormat.setFontItalic(true);

    // Правила подсветки
    
    // Заголовки (# Header)
    HighlightingRule rule;
    rule.pattern = QRegularExpression("^#{1,6}\\s.*$");
    rule.format = headerFormat;
    highlightingRules.append(rule);

    // Жирный текст (**text** или __text__)
    rule.pattern = QRegularExpression("\\*\\*[^*]+\\*\\*");
    rule.format = boldFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("__[^_]+__");
    rule.format = boldFormat;
    highlightingRules.append(rule);

    // Курсив (*text* или _text_)
    rule.pattern = QRegularExpression("\\*[^*]+\\*");
    rule.format = italicFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("_[^_]+_");
    rule.format = italicFormat;
    highlightingRules.append(rule);

    // Код (`code`)
    rule.pattern = QRegularExpression("`[^`]+`");
    rule.format = codeFormat;
    highlightingRules.append(rule);

    // Ссылки ([text](url))
    rule.pattern = QRegularExpression("\\[([^\\]]+)\\]\\(([^)]+)\\)");
    rule.format = linkFormat;
    highlightingRules.append(rule);

    // Цитаты (> quote)
    rule.pattern = QRegularExpression("^>.*$");
    rule.format = quoteFormat;
    highlightingRules.append(rule);

    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");
}

void MarkdownHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, keywordFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}
