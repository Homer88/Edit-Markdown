#include "settings_dialog.h"
#include "settings.h"
#include <QDialogButtonBox>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Основные настройки"));
    setModal(true);
    setupUI();
    loadCurrentSettings();
}

void SettingsDialog::setupUI() {
    auto *layout = new QVBoxLayout(this);
    auto *formLayout = new QFormLayout();

    // Размер шрифта
    m_fontSizeSpin = new QSpinBox(this);
    m_fontSizeSpin->setRange(8, 72);
    m_fontSizeSpin->setValue(12);
    formLayout->addRow(tr("Размер шрифта:"), m_fontSizeSpin);

    // Язык
    m_languageCombo = new QComboBox(this);
    m_languageCombo->addItem("Русский", "ru");
    m_languageCombo->addItem("English", "en");
    formLayout->addRow(tr("Язык интерфейса:"), m_languageCombo);

    // Кодировка по умолчанию
    m_encodingCombo = new QComboBox(this);
    m_encodingCombo->addItem("UTF-8", "UTF-8");
    m_encodingCombo->addItem("UTF-8 with BOM", "UTF-8 with BOM");
    m_encodingCombo->addItem("Windows-1251", "Windows-1251");
    m_encodingCombo->addItem("KOI8-R", "KOI8-R");
    formLayout->addRow(tr("Кодировка по умолчанию:"), m_encodingCombo);

    layout->addLayout(formLayout);

    // Кнопки
    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(buttonBox);
}

void SettingsDialog::loadCurrentSettings() {
    const auto& settings = Settings::instance();
    m_fontSizeSpin->setValue(settings.fontSize());
    
    // Найти индекс для языка
    int langIndex = m_languageCombo->findData(settings.language());
    if (langIndex >= 0) {
        m_languageCombo->setCurrentIndex(langIndex);
    }
    
    // Найти индекс для кодировки
    int encIndex = m_encodingCombo->findData(settings.defaultEncoding());
    if (encIndex >= 0) {
        m_encodingCombo->setCurrentIndex(encIndex);
    }
}

int SettingsDialog::fontSize() const {
    return m_fontSizeSpin->value();
}

QString SettingsDialog::language() const {
    return m_languageCombo->currentData().toString();
}

QString SettingsDialog::defaultEncoding() const {
    return m_encodingCombo->currentData().toString();
}

void SettingsDialog::accept() {
    // Сохраняем настройки в синглтон
    auto& settings = Settings::instance();
    settings.setFontSize(fontSize());
    settings.setLanguage(language());
    settings.setDefaultEncoding(defaultEncoding());
    
    // Сохраняем в файл
    settings.saveToFile(settings.configPath());
    
    QDialog::accept();
}
