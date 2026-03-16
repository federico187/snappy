#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QLineEdit>
#include <QCheckBox>
#include <QKeySequenceEdit>
#include <QSpinBox>
#include <QToolButton>
#include <QMap>

class PreferencesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(QSettings *settings, QWidget *parent = nullptr);
private slots:
    void browseFolder();
    void apply();
    void resetShortcuts();
    void pickDefaultColor();
private:
    void build();
    void load();
    void updateColorSwatch();
    QSettings *m_s;
    QLineEdit *m_pathEdit;
    QCheckBox *m_autoSave;
    QCheckBox *m_tray;
    QCheckBox *m_startWithWindows;
    QMap<QString, QKeySequenceEdit*> m_scEdits;

    // Defaults
    QToolButton *m_colorSwatch;
    QColor m_defaultColor;
    QSpinBox *m_widthArrow;
    QSpinBox *m_widthLine;
    QSpinBox *m_widthShape;
    QSpinBox *m_widthText;
};

#endif
