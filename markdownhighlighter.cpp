#include "markdownhighlighter.h"
#include <QTextDocument>
#include <QColor>
#include <QFont>
#include "settings.h"

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    updateColors();
    
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
}

void MarkdownHighlighter::updateColors()
{
    const auto& settings = Settings::instance();
    
    keywordFormat.setForeground(settings.keywordColor());
    keywordFormat.setFontWeight(QFont::Bold);

    headerFormat.setForeground(settings.headerColor());
    headerFormat.setFontWeight(QFont::Bold);
    headerFormat.setFontPointSize(14);

    boldFormat.setFontWeight(QFont::Bold);
    boldFormat.setForeground(settings.boldColor());

    italicFormat.setFontItalic(true);
    italicFormat.setForeground(settings.italicColor());

    codeFormat.setForeground(settings.codeColor());
    codeFormat.setFontFamily("Courier New");
    codeFormat.setBackground(settings.codeBackgroundColor());

    linkFormat.setForeground(settings.linkColor());
    linkFormat.setFontUnderline(true);

    quoteFormat.setForeground(settings.quoteColor());
    quoteFormat.setFontItalic(true);
    
    rehighlight();
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
}
