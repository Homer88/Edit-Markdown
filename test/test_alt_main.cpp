#include <QCoreApplication>
#include <QDebug>
#include "../markdownparser.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    MarkdownParser parser;
    
    QString input = "Alt H1\n===\nAlt H2\n---";
    qDebug() << "Input:" << input;
    
    QString html = parser.parse(input);
    qDebug() << "HTML:" << html;
    
    QString backToMd = parser.htmlToMarkdown(html);
    qDebug() << "Back to Markdown:" << backToMd;
    
    return 0;
}
