#include <QCoreApplication>
#include <QString>
#include <QDebug>
#include <QFile>
#include <QIODevice>
#include "markdownparser.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    MarkdownParser parser;
    
    QFile file("/workspace/test/test.md");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file";
        return 1;
    }
    
    QString markdown = file.readAll();
    file.close();
    
    QString html = parser.parse(markdown);
    
    qDebug() << "========== HTML OUTPUT ==========";
    qDebug() << html;
    
    QFile out("/workspace/test/output.html");
    if (out.open(QIODevice::WriteOnly)) {
        out.write(html.toUtf8());
        out.close();
        qDebug() << "Output saved to /workspace/test/output.html";
    }
    
    return 0;
}
