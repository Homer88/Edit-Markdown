Markdown Syntax Test (Full)
1. Headers
H1
H2
H3
H4
H5
H6
Alt H1
===
Alt H2\n\n2. Paragraphs & line breaks
Line 1 (two spaces at end)
Line 2
Empty line -> new paragraph.
3. Text styles
Italic Italic
Bold Bold
Bold italic Bold italic
Strikethrough
inline code
Underline
Marked
4. Lists
Item 1
Subitem
Asterisk
Plus
First
Sub
Second
Task lists:
[x] Done
[ ] Pending
5. Links
Text
With title \n\n[ref][id]
[id]: https://example.com
6. Images
￼
![ref][logo]
[logo]: https://picsum.photos/id/1/200/100
7. Blockquotes
Simple quote \n\n> Nested\n\nBack\n\nHeader inside\n\n- list\n\ncode\n\n8. Code
inline
// indented block
int main() {
return 0;
}
plain indented block without highlighting
def hello():
print("Python")
9. Horizontal rules\n\n10. Tables (GFM)
| Header 1 | Header 2 |
|----------|----------|
| Cell 1 | Cell 2 |
| Bold | Italic |
| Left | Center | Right |
|:-----|:------:|------:|
| text | text | text |
11. Footnotes (some parsers)
Here is a footnote.[^1]
[^1]: Footnote text.
12. Definition lists (PHP Markdown Extra, Pandoc)
Term 1
: Definition 1
Term 2
: Definition 2
13. Emojis
:joy: :heart: :rocket:
Direct: 😊 🚀
14. Subscript / Superscript
H2O
X2
(Some: ^superscript^ and ~subscript~)
15. Math (LaTeX)
Inline: $E=mc^2$
Block:
$$
\int_a^b x^2 dx = \frac{b^3-a^3}{3}
$$
16. HTML inside
Red text
Highlight\n\n17. Escaping special chars
\not italic\
\#not header
\[not link\]
\not underscore\
\\ backslash
18. Auto links 
19. Inline comments (Kramdown/Pandoc)
[//]: # (comment)
[comment]: (another comment)
20. Custom anchors (some extensions)
Header with ID {#custom-id}
Jump to header
21. GFM auto linking
https://example.com becomes link
22. Mixed example
Note: \n\nInside quote\n\n- [x] task\n\nrust
fn main() {} 
23. Nested list with code and quote
Step 1
Sub A
Sub B
echo "inside"
Step 2
> Quote inside list 
24. Spaces at line start
This line starts with spaces (may be code)\n\nConclusion: This file covers CommonMark + GFM + extensions (tables, tasks, footnotes, def lists, math, emoji, etc.). Parsing depends on implementation.