#include "settings.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

Settings::Settings() 
    : m_fontSize(12)
    , m_language("ru")
    , m_defaultEncoding("UTF-8")
    , m_colorScheme("default")
{
}

Settings::~Settings() = default;

Settings& Settings::instance() {
    static Settings instance;
    return instance;
}

void Settings::setFontSize(int size) {
    if (size > 0 && size <= 72) {
        m_fontSize = size;
    }
}

void Settings::setLanguage(const QString& lang) {
    m_language = lang;
}

void Settings::setDefaultEncoding(const QString& encoding) {
    m_defaultEncoding = encoding;
}

void Settings::setColorScheme(const QString& scheme) {
    m_colorScheme = scheme;
}

QColor Settings::headerColor() const {
    if (m_colorScheme == "dark") return QColor(255, 100, 100);
    if (m_colorScheme == "light") return QColor(200, 0, 0);
    if (m_colorScheme == "blue") return QColor(0, 100, 255);
    if (m_colorScheme == "green") return QColor(0, 150, 0);
    // default
    return QColor(150, 0, 0);
}

QColor Settings::boldColor() const {
    if (m_colorScheme == "dark") return QColor(255, 255, 255);
    if (m_colorScheme == "light") return QColor(0, 0, 0);
    if (m_colorScheme == "blue") return QColor(50, 50, 100);
    if (m_colorScheme == "green") return QColor(0, 80, 0);
    // default
    return QColor(0, 0, 0);
}

QColor Settings::italicColor() const {
    if (m_colorScheme == "dark") return QColor(200, 200, 200);
    if (m_colorScheme == "light") return QColor(80, 80, 80);
    if (m_colorScheme == "blue") return QColor(100, 100, 150);
    if (m_colorScheme == "green") return QColor(50, 100, 50);
    // default
    return QColor(80, 80, 80);
}

QColor Settings::codeColor() const {
    if (m_colorScheme == "dark") return QColor(100, 255, 100);
    if (m_colorScheme == "light") return QColor(0, 128, 0);
    if (m_colorScheme == "blue") return QColor(0, 150, 200);
    if (m_colorScheme == "green") return QColor(0, 180, 0);
    // default
    return QColor(0, 128, 0);
}

QColor Settings::codeBackgroundColor() const {
    if (m_colorScheme == "dark") return QColor(40, 40, 40);
    if (m_colorScheme == "light") return QColor(245, 245, 245);
    if (m_colorScheme == "blue") return QColor(230, 240, 250);
    if (m_colorScheme == "green") return QColor(235, 250, 235);
    // default
    return QColor(245, 245, 245);
}

QColor Settings::linkColor() const {
    if (m_colorScheme == "dark") return QColor(100, 150, 255);
    if (m_colorScheme == "light") return QColor(0, 0, 255);
    if (m_colorScheme == "blue") return QColor(0, 50, 200);
    if (m_colorScheme == "green") return QColor(0, 120, 120);
    // default
    return QColor(0, 0, 255);
}

QColor Settings::quoteColor() const {
    if (m_colorScheme == "dark") return QColor(180, 180, 180);
    if (m_colorScheme == "light") return QColor(100, 100, 100);
    if (m_colorScheme == "blue") return QColor(80, 80, 120);
    if (m_colorScheme == "green") return QColor(80, 100, 80);
    // default
    return QColor(100, 100, 100);
}

QColor Settings::keywordColor() const {
    if (m_colorScheme == "dark") return QColor(200, 150, 50);
    if (m_colorScheme == "light") return QColor(0, 100, 200);
    if (m_colorScheme == "blue") return QColor(0, 80, 180);
    if (m_colorScheme == "green") return QColor(50, 120, 50);
    // default
    return QColor(0, 100, 200);
}

QString Settings::configPath() const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return configDir + "/settings.xml";
}

bool Settings::saveToFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open settings file for writing:" << filePath;
        return false;
    }

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("Settings");
    
    writer.writeTextElement("FontSize", QString::number(m_fontSize));
    writer.writeTextElement("Language", m_language);
    writer.writeTextElement("DefaultEncoding", m_defaultEncoding);
    writer.writeTextElement("ColorScheme", m_colorScheme);
    
    writer.writeEndElement(); // Settings
    writer.writeEndDocument();
    
    file.close();
    return true;
}

bool Settings::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Файл не существует, используем значения по умолчанию
        return false;
    }

    QXmlStreamReader reader(&file);
    
    while (!reader.atEnd() && !reader.hasError()) {
        QXmlStreamReader::TokenType token = reader.readNext();
        
        if (token == QXmlStreamReader::StartElement) {
            if (reader.name() == "FontSize") {
                m_fontSize = reader.readElementText().toInt();
            } else if (reader.name() == "Language") {
                m_language = reader.readElementText();
            } else if (reader.name() == "DefaultEncoding") {
                m_defaultEncoding = reader.readElementText();
            } else if (reader.name() == "ColorScheme") {
                m_colorScheme = reader.readElementText();
            }
        }
    }
    
    if (reader.hasError()) {
        qWarning() << "Error reading settings file:" << reader.errorString();
        file.close();
        return false;
    }
    
    file.close();
    return true;
}
