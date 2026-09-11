#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>

class ProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProfileDialog(QWidget *parent = nullptr);

private slots:
    void onSave();

private:
    void loadData();

    QLabel *m_roleLabel;
    QLineEdit *m_emailEdit;
    QLineEdit *m_oldPasswordEdit;
    QLineEdit *m_newPasswordEdit;
    QLineEdit *m_confirmPasswordEdit;

    // Customer fields
    QGroupBox *m_customerGroup;
    QLineEdit *m_orgNameEdit;
    QLineEdit *m_contactPersonEdit;
    QLineEdit *m_addressEdit;
    QLineEdit *m_phoneEdit;

    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
};

#endif // PROFILEDIALOG_H
