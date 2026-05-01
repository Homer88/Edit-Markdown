#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QFont>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QFile>
#include <QDir>

class Settings {
public:
    static Settings& instance();

    // Getters
    int fontSize() const { return m_fontSize; }
    QString language() const { return m_language; }
    QString defaultEncoding() const { return m_defaultEncoding; }
    QFont editorFont() const { return QFont("Consolas", m_fontSize); }

    // Setters
    void setFontSize(int size);
    void setLanguage(const QString& lang);
    void setDefaultEncoding(const QString& encoding);

    // Persistence
    bool saveToFile(const QString& filePath);
    bool loadFromFile(const QString& filePath);

private:
    Settings();
    ~Settings();
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    int m_fontSize;
    QString m_language;
    QString m_defaultEncoding;
    
    QString configPath() const;
};

#endif // SETTINGS_H
