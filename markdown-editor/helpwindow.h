#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <QDialog>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>

/**
 * @brief Класс окна справки (HelpWindow).
 * 
 * Отображает подробную информацию о возможностях редактора,
 * синтаксисе Markdown и горячих клавишах в формате HTML.
 * Содержит текстовый браузер для отображения контента и кнопку закрытия.
 */
class HelpWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор окна справки.
     * @param parent Родительский виджет.
     * 
     * Инициализирует интерфейс, задает заголовок окна,
     * минимальный размер и заполняет контент справочной информацией.
     */
    explicit HelpWindow(QWidget *parent = nullptr);

private:
    QTextBrowser *textBrowser; ///< Виджет для отображения HTML-контента справки
    QPushButton *closeButton;  ///< Кнопка закрытия окна

    /**
     * @brief Генерирует полный текст справки в формате HTML.
     * @return QString Строка с HTML-разметкой содержания справки.
     * 
     * Формирует структурированный документ с разделами:
     * - Общие сведения
     * - Режимы работы
     * - Панель инструментов
     * - Синтаксис Markdown
     * - Работа с таблицами
     * - Орфография
     * - Горячие клавиши
     */
    QString generateHelpContent() const;
};

#endif // HELPWINDOW_H
