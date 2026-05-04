#include <iostream>
#include <QString>
#include "markdownparser.h"

int main() {
    MarkdownParser parser;
    QString md = "Hello **World**\n\n- Item 1\n- Item 2\n\n# Header 1";
    QString html = parser.parse(md);
    std::cout << "HTML Output:\n" << html.toStdString() << std::endl;
    return 0;
}
