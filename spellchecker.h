#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QPoint>
#include "hunspell/hunspell.h"

/**
 * @class SpellChecker
 * @brief Класс для проверки орфографии текста с использованием библиотеки Hunspell
 * 
 * Поддерживает:
 * - Проверку отдельных слов
 * - Проверку всего текста
 * - Предложения по исправлению ошибок
 * - Добавление слов в пользовательский словарь
 * - Поддержку нескольких языков (русский, английский)
 */
class SpellChecker
{
public:
    /**
     * @brief Конструктор класса SpellChecker
     * @param affPath Путь к файлу аффиксов (.aff)
     * @param dicPath Путь к файлу словаря (.dic)
     */
    explicit SpellChecker(const QString& affPath, const QString& dicPath);
    
    /**
     * @brief Деструктор класса SpellChecker
     * Освобождает ресурсы Hunspell
     */
    ~SpellChecker();
    
    /**
     * @brief Проверка слова на правильность написания
     * @param word Слово для проверки
     * @return true если слово правильное, false если есть ошибка
     */
    bool isWordCorrect(const QString& word) const;
    
    /**
     * @brief Получение списка предложений по исправлению слова
     * @param word Слово с ошибкой
     * @return Список вариантов правильного написания
     */
    QStringList getSuggestions(const QString& word) const;
    
    /**
     * @brief Проверка всего текста и возврат позиций ошибок
     * @param text Текст для проверки
     * @return Список пар (позиция начала, длина) для каждого слова с ошибкой
     */
    QList<QPair<int, int>> checkText(const QString& text) const;
    
    /**
     * @brief Добавление слова в пользовательский словарь
     * @param word Слово для добавления
     */
    void addWordToDictionary(const QString& word);
    
    /**
     * @brief Проверка инициализации словаря
     * @return true если словарь успешно загружен
     */
    bool isInitialized() const;
    
    /**
     * @brief Получение текущего языка
     * @return Название языка
     */
    QString getLanguage() const;

private:
    Hunspell* m_hunspell;  ///< Указатель на экземпляр Hunspell
    bool m_initialized;    ///< Флаг успешной инициализации
    QString m_language;    ///< Текущий язык словаря
    
    /**
     * @brief Очистка слова от лишних символов
     * @param word Исходное слово
     * @return Очищенное слово
     */
    QString cleanWord(const QString& word) const;
};

#endif // SPELLCHECKER_H
