# Markdown Syntax Test (Full)

## 1. Headers
# H1
## H2
### H3
#### H4
##### H5
###### H6
Alt H1
===
Alt H2
---

## 2. Paragraphs & line breaks
Line 1 (two spaces at end)  
Line 2
Empty line -> new paragraph.

## 3. Text styles
*Italic* _Italic_  
**Bold** __Bold__  
***Bold italic*** ___Bold italic___  
~~Strikethrough~~  
`inline code`  
<u>Underline</u>  
<mark>Marked</mark>

## 4. Lists
- Item 1
  - Subitem
* Asterisk
+ Plus

1. First
   1. Sub
2. Second

Task lists:
- [x] Done
- [ ] Pending

## 5. Links
[Text](https://example.com)  
[With title](https://example.com "Title")  
<https://auto.link>  
<email@example.com>  
[ref][id]  
[id]: https://example.com

## 6. Images
![Alt](https://picsum.photos/200/100 "Title")  
![ref][logo]  
[logo]: https://picsum.photos/id/1/200/100

## 7. Blockquotes
> Simple quote
>> Nested
> 
> Back
> #### Header inside
> - list
> `code`

## 8. Code

`inline`

    // indented block
    int main() {
        return 0;
    }

    plain indented block without highlighting

        def hello():
            print("Python")

## 9. Horizontal rules
---
***
___

## 10. Tables (GFM)
| Header 1 | Header 2 |
|----------|----------|
| Cell 1   | Cell 2   |
| **Bold** | *Italic* |

| Left | Center | Right |
|:-----|:------:|------:|
| text | text   | text  |

## 11. Footnotes (some parsers)
Here is a footnote.[^1]
[^1]: Footnote text.

## 12. Definition lists (PHP Markdown Extra, Pandoc)
Term 1
: Definition 1

Term 2
: Definition 2

## 13. Emojis
:joy: :heart: :rocket:  
Direct: 😊 🚀

## 14. Subscript / Superscript
H<sub>2</sub>O  
X<sup>2</sup>  
(Some: ^superscript^ and ~subscript~)

## 15. Math (LaTeX)
Inline: $E=mc^2$  
Block:
$$
\int_a^b x^2 dx = \frac{b^3-a^3}{3}
$$

## 16. HTML inside
<div style="color:red;">Red text</div>
<span style="background:yellow;">Highlight</span>
<!-- HTML comment -->

## 17. Escaping special chars
\*not italic\*  
\#not header  
\[not link\]  
\_not underscore\_  
\\ backslash

## 18. Auto links
<http://example.com>  
<user@example.com>

## 19. Inline comments (Kramdown/Pandoc)
[//]: # (comment)
[comment]: <> (another comment)

## 20. Custom anchors (some extensions)
### Header with ID {#custom-id}
[Jump to header](#custom-id)

## 21. GFM auto linking
https://example.com becomes link

## 22. Mixed example
> **Note:**  
> *Inside quote*  
> - [x] task  

    rust
    fn main() {}

## 23. Nested list with code and quote
1. Step 1
   - Sub A
   - Sub B

         echo "inside"

2. Step 2
   > Quote inside list

## 24. Spaces at line start
   This line starts with spaces (may be code)

---

**Conclusion:** This file covers CommonMark + GFM + extensions (tables, tasks, footnotes, def lists, math, emoji, etc.). Parsing depends on implementation.