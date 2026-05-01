#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSlider>
#include <QColorDialog>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    
    int fontSize() const;
    QString language() const;
    QString defaultEncoding() const;
    qreal interfaceScale() const;
    QString colorScheme() const;

private slots:
    void accept() override;
    void selectColorForElement();

private:
    void setupUI();
    void loadCurrentSettings();
    void updateColorButton(const QString& elementName, QPushButton* button, const QColor& color);

    QSpinBox *m_fontSizeSpin;
    QComboBox *m_languageCombo;
    QComboBox *m_encodingCombo;
    QSlider *m_interfaceScaleSlider;
    QLabel *m_scaleValueLabel;
    QComboBox *m_colorSchemeCombo;
};

#endif // SETTINGS_DIALOG_H
