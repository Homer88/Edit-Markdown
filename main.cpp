#include <QApplication>
#include "mainwindow.h"
#include <QSettings>
#include <QScreen>

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
    // Включаем поддержку High DPI до создания QApplication
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    
    // Создаем QApplication - основной объект приложения Qt
    QApplication app(argc, argv);
    
    // Устанавливаем организацию и имя приложения
    QCoreApplication::setOrganizationName("MarkdownEditor");
    QCoreApplication::setApplicationName("Markdown Editor");
    QCoreApplication::setApplicationVersion("1.0");
    
    // Загружаем пользовательский масштаб интерфейса из настроек
    QSettings settings("MarkdownEditor", "Markdown Editor");
    qreal interfaceScale = settings.value("interfaceScale", 1.0).toReal();
    
    // Если масштаб не равен 1.0, применяем его
    if (interfaceScale != 1.0) {
        app.setFont(QFont(app.font().family(), qRound(app.font().pointSize() * interfaceScale)));
    }
    
    // Создаем и показываем главное окно
    MainWindow window;
    window.show();
    
    // Запускаем главный цикл обработки событий
    return app.exec();
}
