#include "settings_dialog.h"
#include "settings.h"
#include <QDialogButtonBox>
#include <QSettings>
#include <QGroupBox>
#include <QGridLayout>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Настройки"));
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

    // Масштаб интерфейса
    auto *scaleLayout = new QHBoxLayout();
    m_interfaceScaleSlider = new QSlider(Qt::Horizontal, this);
    m_interfaceScaleSlider->setRange(50, 200);
    m_interfaceScaleSlider->setValue(100);
    m_interfaceScaleSlider->setTickPosition(QSlider::TicksBelow);
    m_interfaceScaleSlider->setTickInterval(25);
    
    m_scaleValueLabel = new QLabel("100%", this);
    m_scaleValueLabel->setMinimumWidth(40);
    
    scaleLayout->addWidget(m_interfaceScaleSlider);
    scaleLayout->addWidget(m_scaleValueLabel);
    
    connect(m_interfaceScaleSlider, &QSlider::valueChanged, [this](int value) {
        m_scaleValueLabel->setText(QString::number(value) + "%");
    });
    
    formLayout->addRow(tr("Масштаб интерфейса:"), scaleLayout);

    // Цветовая схема подсветки Markdown
    m_colorSchemeCombo = new QComboBox(this);
    m_colorSchemeCombo->addItem("По умолчанию", "default");
    m_colorSchemeCombo->addItem("Тёмная", "dark");
    m_colorSchemeCombo->addItem("Светлая", "light");
    m_colorSchemeCombo->addItem("Синяя", "blue");
    m_colorSchemeCombo->addItem("Зелёная", "green");
    formLayout->addRow(tr("Цветовая схема:"), m_colorSchemeCombo);

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
    
    // Загрузить масштаб интерфейса
    QSettings appSettings("MarkdownEditor", "Markdown Editor");
    qreal interfaceScale = appSettings.value("interfaceScale", 1.0).toReal();
    int scalePercent = qRound(interfaceScale * 100);
    m_interfaceScaleSlider->setValue(scalePercent);
    m_scaleValueLabel->setText(QString::number(scalePercent) + "%");
    
    // Загрузить цветовую схему
    int schemeIndex = m_colorSchemeCombo->findData(settings.colorScheme());
    if (schemeIndex >= 0) {
        m_colorSchemeCombo->setCurrentIndex(schemeIndex);
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

qreal SettingsDialog::interfaceScale() const {
    return m_interfaceScaleSlider->value() / 100.0;
}

QString SettingsDialog::colorScheme() const {
    return m_colorSchemeCombo->currentData().toString();
}

void SettingsDialog::accept() {
    // Сохраняем настройки в синглтон
    auto& settings = Settings::instance();
    settings.setFontSize(fontSize());
    settings.setLanguage(language());
    settings.setDefaultEncoding(defaultEncoding());
    settings.setColorScheme(colorScheme());
    
    // Сохраняем в файл
    settings.saveToFile(settings.configPath());
    
    // Сохраняем масштаб интерфейса
    QSettings appSettings("MarkdownEditor", "Markdown Editor");
    appSettings.setValue("interfaceScale", interfaceScale());
    
    QDialog::accept();
}

void SettingsDialog::selectColorForElement() {
    // Эта функция может быть использована для расширения функционала
    // выбора индивидуальных цветов для каждого элемента подсветки
}

void SettingsDialog::updateColorButton(const QString& elementName, QPushButton* button, const QColor& color) {
    Q_UNUSED(elementName);
    Q_UNUSED(button);
    Q_UNUSED(color);
    // Эта функция может быть использована для обновления кнопок выбора цвета
}
