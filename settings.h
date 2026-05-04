#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QFont>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QFile>
#include <QDir>
#include <QColor>
#include <QStringList>

class Settings {
public:
    static Settings& instance();

    // Getters
    int fontSize() const { return m_fontSize; }
    QString language() const { return m_language; }
    QString defaultEncoding() const { return m_defaultEncoding; }
    QFont editorFont() const { return QFont("Consolas", m_fontSize); }
    QString colorScheme() const { return m_colorScheme; }
    QStringList recentFiles() const { return m_recentFiles; }
    int maxRecentFiles() const { return 10; }
    
    // Цвета для подсветки Markdown
    QColor headerColor() const;
    QColor boldColor() const;
    QColor italicColor() const;
    QColor codeColor() const;
    QColor codeBackgroundColor() const;
    QColor linkColor() const;
    QColor quoteColor() const;
    QColor keywordColor() const;

    // Setters
    void setFontSize(int size);
    void setLanguage(const QString& lang);
    void setDefaultEncoding(const QString& encoding);
    void setColorScheme(const QString& scheme);
    void addRecentFile(const QString& filePath);
    void clearRecentFiles();

    // Persistence
    bool saveToFile(const QString& filePath);
    bool loadFromFile(const QString& filePath);
    QString configPath() const;

private:
    Settings();
    ~Settings();
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    int m_fontSize;
    QString m_language;
    QString m_defaultEncoding;
    QString m_colorScheme;
    QStringList m_recentFiles;
};

#endif // SETTINGS_H
