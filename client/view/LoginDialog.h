#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
private slots:
    void onLogin();
    void onRegister();
private:
    void setupUi();
    QLineEdit *m_loginEmail, *m_loginPassword;
    QLineEdit *m_regEmail, *m_regPassword, *m_regPasswordConfirm;
    QLineEdit *m_regOrgName, *m_regContact, *m_regAddress, *m_regPhone;
};

#endif // LOGINDIALOG_H
