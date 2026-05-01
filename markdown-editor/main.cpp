#include <QApplication>
#include "mainwindow.h"

/**
 * @brief Главная функция приложения
 * @param argc Количество аргументов командной строки
 * @param argv Массив аргументов командной строки
 * @return int Код возврата приложения
 * 
 * Точка входа в приложение Markdown Editor.
 * Инициализирует Qt приложение и создает главное окно.
 */
int main(int argc, char *argv[])
{
    // Создаем QApplication - основной объект приложения Qt
    QApplication app(argc, argv);
    
    // Устанавливаем организацию и имя приложения
    QCoreApplication::setOrganizationName("MarkdownEditor");
    QCoreApplication::setApplicationName("Markdown Editor");
    QCoreApplication::setApplicationVersion("1.0");
    
    // Создаем и показываем главное окно
    MainWindow window;
    window.show();
    
    // Запускаем главный цикл обработки событий
    return app.exec();
}
