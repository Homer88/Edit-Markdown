Пишем программу для работы с файлами md. запрещено писать тесты на python. все пишем на С++
есть два варианта интерфейса:
WYSINYG  видим, что и  делаем как в word 
Markdown это классический текстовый редактор с   предпросмотром.
Сделано:
[done]нарисован интерфейса
[done]Созданы меню интерфейса
[done] работа с кодировкой 
[done] проверка на ошибками в словах (орфография)
[done] Для windows  мы копируем dll  для запуска где нету qt (частично появляются баги) невсегда все библиотеки копируются. Требуется проверки. 

критические баги:
[] переход с одного  типа интерфейса Markdown->WYSINYG->Markdown не восстанавливается    
[] переход с одного  типа интерфейса Markdown->WYSINYG->Markdown  пропадает окно простора
[] переход с одного  типа интерфейса Markdown->WYSINYG->Markdown ломается текст 
[] переход с одного  типа интерфейса Markdown->WYSINYG->Markdown->Markdown->WYSINYG->Markdown ломается текст стекает в одну строчку
[] переход с одного  типа интерфейса Markdown->WYSINYG->Markdown ломается цветовая схема.
[] переход с одного  типа интерфейса Markdown->WYSINYG->Markdown ломается таблицы.

баги с пасером:
[]багов сильно много.

результаты тестов (запуск от 2024):
=====================================
parser_tests: 29 passed, 5 failed
roundtrip_tests: 4 passed, 0 failed (но 1 секция потеряна)

найденные баги парсера:
-----------------------
1. [BUG] Code blocks не парсятся - тег <pre><code> не генерируется
   - testCodeBlock() - FAIL
   - testCodeBlockWithLanguage() - FAIL  
   - testCodeBlockAndInlineCodeMixed() - FAIL
   - testMultipleCodeBlocks() - FAIL (0 вместо 2 блоков)

2. [BUG] Потеря контента при round-trip конвертации
   - Секция "Code" теряется при Markdown->HTML->Markdown
   - В HTML появляется некорректная структура: <pre><code class="language-">code block\n<p></code></pre></p>

3. [BUG] Blockquote форматирование некорректное
   - В HTML появляется лишняя обертка <p> внутри <blockquote>
   - Пример: <blockquote>Это цитата\n<p></blockquote></p>

4. [BUG] Таблицы не преобразуются в HTML таблицу
   - Оставляются как текст с | разделителями внутри <p>
   - testSimpleTable() и testTableWithFormatting() проходят, но таблицы не становятся настоящими HTML таблицами

5. [BUG] Nested blockquotes не работают
   - >> Уровень 2 остается как текст "> Уровень 2" вместо вложенного <blockquote>

6. [BUG] Math/LaTeX формулы повреждаются при round-trip
   - Inline формула $E=mc^2$ превращается в <span class="math-inline">$E=mc^2$</span>
   - Block формулы получают двойную обертку <span class="math-display">

критические баги (из work.md):
------------------------------
[ACTIVE] переход Markdown->WYSINYG->Markdown не восстанавливается полностью
[ACTIVE] переход Markdown->WYSINYG->Markdown пропадает окно просмотра  
[ACTIVE] переход Markdown->WYSINYG->Markdown ломается текст
[ACTIVE] переход Markdown->WYSINYG->Markdown->Markdown->WYSINYG->Markdown текст стекает в одну строчку
[ACTIVE] переход Markdown->WYSINYG->Markdown ломается цветовая схема
[ACTIVE] переход Markdown->WYSINYG->Markdown ломается таблицы
[NEW] Code blocks полностью теряются при конвертации
[NEW] Blockquote HTML структура некорректна

тесты:
тесты написаны  и храниться в папке test. требуется написать еще тест. Файл для теста тоже лежит в папке test  и называется test.md  
Особенности тестов. Оригинальный файл сравниваем с полученным файлом двумя способами:
1. по контрольной сумме. 
2. по символьное. 