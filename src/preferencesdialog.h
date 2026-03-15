#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QLineEdit>
#include <QCheckBox>
#include <QKeySequenceEdit>
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
private:
    void build();
    void load();
    QSettings *m_s;
    QLineEdit *m_pathEdit;
    QCheckBox *m_autoSave;
    QCheckBox *m_tray;
    QCheckBox *m_startWithWindows;
    QMap<QString, QKeySequenceEdit*> m_scEdits;
};

#endif
