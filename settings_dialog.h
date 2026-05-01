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

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    
    int fontSize() const;
    QString language() const;
    QString defaultEncoding() const;
    qreal interfaceScale() const;

private slots:
    void accept() override;

private:
    void setupUI();
    void loadCurrentSettings();

    QSpinBox *m_fontSizeSpin;
    QComboBox *m_languageCombo;
    QComboBox *m_encodingCombo;
    QSlider *m_interfaceScaleSlider;
    QLabel *m_scaleValueLabel;
};

#endif // SETTINGS_DIALOG_H
