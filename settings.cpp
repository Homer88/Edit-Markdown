#include "settings.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

Settings::Settings() 
    : m_fontSize(12)
    , m_language("ru")
    , m_defaultEncoding("UTF-8")
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
