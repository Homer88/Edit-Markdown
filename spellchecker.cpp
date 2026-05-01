#include "spellchecker.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QByteArray>
#include <QList>

SpellChecker::SpellChecker(const QString& affPath, const QString& dicPath)
    : m_hunspell(nullptr), m_initialized(false), m_language("ru_RU")
{
    // Проверяем существование файлов словаря
    if (!QFile::exists(affPath) || !QFile::exists(dicPath)) {
        qWarning() << "Файлы словаря не найдены:" << affPath << dicPath;
        return;
    }
    
    // Инициализируем Hunspell с путями к файлам
    QByteArray affBa = affPath.toUtf8();
    QByteArray dicBa = dicPath.toUtf8();
    
    m_hunspell = new Hunspell(affBa.constData(), dicBa.constData());
    
    if (m_hunspell) {
        m_initialized = true;
        
        // Определяем язык по пути к файлу
        if (dicPath.contains("en_", Qt::CaseInsensitive)) {
            m_language = "en_US";
        } else if (dicPath.contains("ru_", Qt::CaseInsensitive)) {
            m_language = "ru_RU";
        }
    }
}

SpellChecker::~SpellChecker()
{
    // Освобождаем ресурсы Hunspell
    if (m_hunspell) {
        delete m_hunspell;
        m_hunspell = nullptr;
    }
}

bool SpellChecker::isWordCorrect(const QString& word) const
{
    if (!m_initialized || !m_hunspell) {
        return true; // Если словарь не загружен, считаем все слова правильными
    }
    
    QString clean = cleanWord(word);
    if (clean.isEmpty()) {
        return true;
    }
    
    // Преобразуем слово в UTF-8 для Hunspell
    QByteArray wordBa = clean.toUtf8();
    
    // Проверяем слово через Hunspell
    int result = m_hunspell->spell(wordBa.constData());
    
    return result != 0;
}

QStringList SpellChecker::getSuggestions(const QString& word) const
{
    QStringList suggestions;
    
    if (!m_initialized || !m_hunspell) {
        return suggestions;
    }
    
    QString clean = cleanWord(word);
    if (clean.isEmpty()) {
        return suggestions;
    }
    
    // Получаем предложения от Hunspell
    QByteArray wordBa = clean.toUtf8();
    char** slst = nullptr;
    int n = m_hunspell->suggest(&slst, wordBa.constData());
    
    if (n > 0 && slst) {
        for (int i = 0; i < n; ++i) {
            suggestions << QString::fromUtf8(slst[i]);
        }
        m_hunspell->free_list(&slst, n);
    }
    
    return suggestions;
}

QList<QPair<int, int>> SpellChecker::checkText(const QString& text) const
{
    QList<QPair<int, int>> errors;
    
    if (!m_initialized || !m_hunspell) {
        return errors;
    }
    
    // Регулярное выражение для поиска слов (кириллица и латиница)
    QRegularExpression wordRegex(R"(\b[\p{Cyrillic}\p{Latin}]+\b)");
    
    QRegularExpressionMatchIterator it = wordRegex.globalMatch(text);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString word = match.captured(0);
        
        if (!word.isEmpty() && !isWordCorrect(word)) {
            errors.append(qMakePair(match.capturedStart(0), match.capturedLength(0)));
        }
    }
    
    return errors;
}

void SpellChecker::addWordToDictionary(const QString& word)
{
    if (!m_initialized || !m_hunspell) {
        return;
    }
    
    QString clean = cleanWord(word);
    if (clean.isEmpty()) {
        return;
    }
    
    // Добавляем слово в словарь Hunspell
    QByteArray wordBa = clean.toUtf8();
    m_hunspell->add(wordBa.constData());
}

bool SpellChecker::isInitialized() const
{
    return m_initialized && (m_hunspell != nullptr);
}

QString SpellChecker::getLanguage() const
{
    return m_language;
}

QString SpellChecker::cleanWord(const QString& word) const
{
    // Удаляем лишние символы (пробелы, знаки препинания)
    QString cleaned = word.trimmed();
    
    // Удаляем Markdown разметку из слова
    cleaned.replace(QRegularExpression(R"([*_`~#])"), "");
    
    return cleaned;
}
