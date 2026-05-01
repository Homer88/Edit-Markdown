#include "helpwindow.h"
#include <QScreen>
#include <QApplication>

/**
 * @brief Конструктор окна справки.
 * 
 * Создает и настраивает интерфейс окна:
 * - Текстовый браузер с поддержкой HTML и навигацией по ссылкам
 * - Кнопку закрытия
 * - Компоновку элементов
 * - Центрирование окна относительно экрана
 */
HelpWindow::HelpWindow(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Справка - Markdown Editor");
    setMinimumSize(800, 600);

    // Создаем текстовый браузер для отображения справки
    textBrowser = new QTextBrowser(this);
    textBrowser->setOpenExternalLinks(true); // Разрешить открытие внешних ссылок
    textBrowser->setHtml(generateHelpContent()); // Загружаем сгенерированный контент

    // Создаем кнопку закрытия
    closeButton = new QPushButton("Закрыть", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    // Создаем компоновку
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(textBrowser);
    layout->addWidget(closeButton);

    setLayout(layout);

    // Центрируем окно на экране
    if (QGuiApplication::primaryScreen()) {
        QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
        move(screenGeometry.center() - rect().center());
    }
}

/**
 * @brief Генерирует содержимое справки в формате HTML.
 * @return QString Полный текст справки с HTML-разметкой.
 * 
 * Включает разделы:
 * 1. Описание программы
 * 2. Режимы редактирования
 * 3. Описание кнопок панели инструментов
 * 4. Полная таблица синтаксиса Markdown
 * 5. Управление таблицами
 * 6. Проверка орфографии
 * 7. Горячие клавиши
 */
QString HelpWindow::generateHelpContent() const
{
    return R"(
    <html>
    <head>
        <style>
            body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; line-height: 1.6; color: #333; }
            h1 { color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 10px; }
            h2 { color: #2980b9; margin-top: 25px; }
            h3 { color: #16a085; }
            .shortcut { background-color: #f4f4f4; padding: 2px 6px; border-radius: 3px; font-family: monospace; border: 1px solid #ddd; }
            table { border-collapse: collapse; width: 100%; margin: 15px 0; }
            th, td { border: 1px solid #bdc3c7; padding: 10px; text-align: left; }
            th { background-color: #ecf0f1; color: #2c3e50; }
            tr:nth-child(even) { background-color: #f9f9f9; }
            code { background-color: #f4f4f4; padding: 2px 5px; border-radius: 3px; font-family: 'Consolas', 'Monaco', monospace; color: #e74c3c; }
            .note { background-color: #fff3cd; border-left: 4px solid #ffc107; padding: 10px; margin: 10px 0; }
            ul { margin-bottom: 10px; }
        </style>
    </head>
    <body>
        <h1>📖 Справка по Markdown Editor</h1>
        
        <h2>1. О программе</h2>
        <p>Markdown Editor — это современный редактор текста с поддержкой формата <strong>Markdown</strong>. 
        Он позволяет писать документы в легком текстовом формате и сразу видеть результат в режиме предпросмотра (WYSIWYG).</p>
        
        <div class="note">
            <strong>Принцип работы:</strong> Вы пишете текст с использованием специальных символов (Markdown), 
            а редактор автоматически преобразует его в красиво оформленный документ.
        </div>

        <h2>2. Режимы работы</h2>
        <ul>
            <li><strong>Режим редактора (Markdown):</strong> Вы видите исходный код с разметкой. Здесь вы вводите текст, используете спецсимволы.</li>
            <li><strong>Режим предпросмотра (WYSIWYG):</strong> Вы видите готовый отформатированный документ. Текст нельзя редактировать напрямую, но можно копировать.</li>
        </ul>
        <p>Переключение между режимами осуществляется кнопкой <strong>"⟷"</strong> на панели инструментов.</p>

        <h2>3. Панель инструментов</h2>
        <table>
            <tr><th>Кнопка</th><th>Назначение</th><th>Горячая клавиша</th></tr>
            <tr><td>📄 Новый</td><td>Создать новый документ</td><td><span class="shortcut">Ctrl+N</span></td></tr>
            <tr><td>📂 Открыть</td><td>Открыть файл (.md, .txt)</td><td><span class="shortcut">Ctrl+O</span></td></tr>
            <tr><td>💾 Сохранить</td><td>Сохранить текущий файл</td><td><span class="shortcut">Ctrl+S</span></td></tr>
            <tr><td>↔️ Режим</td><td>Переключить вид (Текст/Предпросмотр)</td><td><span class="shortcut">F5</span></td></tr>
            <tr><td><strong>B</strong></td><td>Выделить текст <strong>жирным</strong></td><td><span class="shortcut">Ctrl+B</span></td></tr>
            <tr><td><em>I</em></td><td>Выделить текст <em>курсивом</em></td><td><span class="shortcut">Ctrl+I</span></td></tr>
            <tr><td><s>S</s></td><td>Выделить текст зачеркнутым</td><td><span class="shortcut">Ctrl+U</span></td></tr>
            <tr><td>H1-H3</td><td>Применить стили заголовков</td><td><span class="shortcut">Ctrl+1..3</span></td></tr>
            <tr><td>🔗 Ссылка</td><td>Вставить гиперссылку</td><td><span class="shortcut">Ctrl+K</span></td></tr>
            <tr><td>🖼️ Изображение</td><td>Вставить изображение из файла</td><td>-</td></tr>
            <tr><td>{ }</td><td>Блок кода (многострочный)</td><td><span class="shortcut">Ctrl+Alt+C</span></td></tr>
            <tr><td>` `</td><td>Код внутри строки</td><td>-</td></tr>
            <tr><td>☑ Задача</td><td>Создать элемент списка задач</td><td>-</td></tr>
            <tr><td>❝ Цитата</td><td>Оформить текст как цитату</td><td><span class="shortcut">Ctrl+Q</span></td></tr>
            <tr><td>≡ Таблица ▼</td><td>Вставка таблицы и управление ею</td><td>-</td></tr>
            <tr><td>Ω Символы</td><td>Вставка специальных символов</td><td>-</td></tr>
            <tr><td>ABC ✓</td><td>Проверка орфографии</td><td><span class="shortcut">F7</span></td></tr>
            <tr><td>❓ Справка</td><td>Открыть это окно</td><td><span class="shortcut">F1</span></td></tr>
        </table>

        <h2>4. Синтаксис Markdown</h2>
        <h3>Текстовое форматирование</h3>
        <table>
            <tr><th>Элемент</th><th>Синтаксис</th><th>Пример ввода</th><th>Результат</th></tr>
            <tr><td>Жирный</td><td><code>**текст**</code> или <code>__текст__</code></td><td><code>**важно**</code></td><td><strong>важно</strong></td></tr>
            <tr><td>Курсив</td><td><code>*текст*</code> или <code>_текст_</code></td><td><code>*акцент*</code></td><td><em>акцент</em></td></tr>
            <tr><td>Зачеркнутый</td><td><code>~~текст~~</code></td><td><code>~~ошибка~~</code></td><td><s>ошибка</s></td></tr>
            <tr><td>Жирный курсив</td><td><code>***текст***</code></td><td><code>***выделение***</code></td><td><strong><em>выделение</em></strong></td></tr>
        </table>

        <h3>Заголовки</h3>
        <table>
            <tr><th>Уровень</th><th>Синтаксис</th><th>Пример</th></tr>
            <tr><td>H1 (Главный)</td><td><code># Заголовок</code></td><td><code># Глава 1</code></td></tr>
            <tr><td>H2 (Подзаголовок)</td><td><code>## Заголовок</code></td><td><code>## Параграф 1.1</code></td></tr>
            <tr><td>H3 (Раздел)</td><td><code>### Заголовок</code></td><td><code>### Детали</code></td></tr>
            <tr><td>H4-H6</td><td><code>#### ... ######</code></td><td>До 6 уровней вложенности</td></tr>
        </table>

        <h3>Списки</h3>
        <table>
            <tr><th>Тип</th><th>Синтаксис</th><th>Пример</th></tr>
            <tr><td>Маркированный</td><td><code>-</code>, <code>*</code> или <code>+</code> перед элементом</td><td><code>- Элемент 1</code></td></tr>
            <tr><td>Нумерованный</td><td><code>1.</code>, <code>2.</code>...</td><td><code>1. Первый шаг</code></td></tr>
            <tr><td>Вложенность</td><td>Добавьте 2 пробела или табуляцию перед маркером</td><td><code>  - Подпункт</code></td></tr>
            <tr><td>Задачи (Checklist)</td><td><code>- [ ]</code> (пусто) или <code>- [x]</code> (готово)</td><td><code>- [x] Сделать</code></td></tr>
        </table>

        <h3>Код</h3>
        <ul>
            <li><strong>Встроенный код:</strong> Используйте обратные кавычки <code>`код`</code>. Пример: <code>`print("Hello")`</code>.</li>
            <li><strong>Блок кода:</strong> Оберните код тройными обратными кавычками. Можно указать язык для подсветки.</li>
        </ul>
        <pre style="background:#f4f4f4; padding:10px; border-radius:5px;">
```python
def hello():
    print("Привет, мир!")
```
        </pre>

        <h3>Ссылки и изображения</h3>
        <table>
            <tr><th>Элемент</th><th>Синтаксис</th><th>Пример</th></tr>
            <tr><td>Ссылка</td><td><code>[Текст](URL)</code></td><td><code>[Google](https://google.com)</code></td></tr>
            <tr><td>Изображение</td><td><code>![Описание](путь_к_файлу)</code></td><td><code>![Лого](logo.png)</code></td></tr>
        </table>

        <h3>Другие элементы</h3>
        <ul>
            <li><strong>Цитата:</strong> Используйте символ <code>></code> в начале строки.</li>
            <li><strong>Горизонтальная линия:</strong> Три дефиса <code>---</code> или звездочки <code>***</code>.</li>
            <li><strong>Таблица:</strong> См. раздел ниже.</li>
        </ul>

        <h2>5. Работа с таблицами</h2>
        <p>Таблицы в Markdown создаются с использованием вертикальных черточек <code>|</code> и дефисов <code>-</code>.</p>
        
        <h3>Создание таблицы</h3>
        <p>Нажмите кнопку <strong>"Таблица"</strong>, чтобы вставить шаблон:</p>
        <pre style="background:#f4f4f4; padding:10px;">
| Заголовок 1 | Заголовок 2 |
|-------------|-------------|
| Ячейка 1    | Ячейка 2    |
        </pre>
        
        <h3>Управление таблицами</h3>
        <p>Используйте меню кнопки <strong>"Таблица ▼"</strong> для модификации:</p>
        <ol>
            <li>Установите курсор внутрь любой ячейки таблицы.</li>
            <li>Выберите действие:
                <ul>
                    <li><strong>Вставить строку</strong> — добавляет новую строку под текущей.</li>
                    <li><strong>Вставить столбец</strong> — добавляет новый столбец справа от текущего.</li>
                    <li><strong>Удалить строку</strong> — удаляет строку, где стоит курсор.</li>
                    <li><strong>Удалить столбец</strong> — удаляет столбец, где стоит курсор.</li>
                </ul>
            </li>
        </ol>
        <div class="note">
            ⚠️ <strong>Важно:</strong> Для корректной работы функций удаления/добавления курсор должен находиться внутри существующей таблицы.
        </div>

        <h2>6. Проверка орфографии</h2>
        <p>Редактор поддерживает проверку правописания на русском и английском языках.</p>
        <ul>
            <li>Нажмите кнопку <strong>"ABC ✓"</strong> или клавишу <span class="shortcut">F7</span>.</li>
            <li>При обнаружении ошибки появится контекстное меню с вариантами исправления.</li>
            <li>Вы можете пропустить слово, заменить его или добавить в пользовательский словарь.</li>
        </ul>

        <h2>7. Горячие клавиши</h2>
        <table>
            <tr><th>Действие</th><th>Комбинация</th></tr>
            <tr><td>Новый файл</td><td><span class="shortcut">Ctrl + N</span></td></tr>
            <tr><td>Открыть файл</td><td><span class="shortcut">Ctrl + O</span></td></tr>
            <tr><td>Сохранить файл</td><td><span class="shortcut">Ctrl + S</span></td></tr>
            <tr><td>Печать / Экспорт</td><td><span class="shortcut">Ctrl + P</span></td></tr>
            <tr><td>Переключение режима</td><td><span class="shortcut">F5</span></td></tr>
            <tr><td>Жирный текст</td><td><span class="shortcut">Ctrl + B</span></td></tr>
            <tr><td>Курсив</td><td><span class="shortcut">Ctrl + I</span></td></tr>
            <tr><td>Подчеркнутый/Зачеркнутый</td><td><span class="shortcut">Ctrl + U</span></td></tr>
            <tr><td>Вставка ссылки</td><td><span class="shortcut">Ctrl + K</span></td></tr>
            <tr><td>Заголовок 1 уровня</td><td><span class="shortcut">Ctrl + 1</span></td></tr>
            <tr><td>Заголовок 2 уровня</td><td><span class="shortcut">Ctrl + 2</span></td></tr>
            <tr><td>Заголовок 3 уровня</td><td><span class="shortcut">Ctrl + 3</span></td></tr>
            <tr><td>Блок кода</td><td><span class="shortcut">Ctrl + Alt + C</span></td></tr>
            <tr><td>Цитата</td><td><span class="shortcut">Ctrl + Q</span></td></tr>
            <tr><td>Проверка орфографии</td><td><span class="shortcut">F7</span></td></tr>
            <tr><td>Справка</td><td><span class="shortcut">F1</span></td></tr>
            <tr><td>Выход</td><td><span class="shortcut">Alt + F4</span></td></tr>
        </table>

        <hr>
        <p style="text-align: center; color: #7f8c8d; font-size: 0.9em;">
            Markdown Editor v1.0<br>
            Создано с использованием Qt6 и C++
        </p>
    </body>
    </html>
    )";
}
