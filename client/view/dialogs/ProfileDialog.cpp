#include "ProfileDialog.h"
#include "NetworkClient.h"
#include <QHBoxLayout>

ProfileDialog::ProfileDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Профиль пользователя");
    setMinimumWidth(450);

    auto *mainLayout = new QVBoxLayout(this);

    // Account Information Group
    auto *accountGroup = new QGroupBox("Данные учетной записи", this);
    auto *accountForm = new QFormLayout(accountGroup);

    m_roleLabel = new QLabel(this);
    accountForm->addRow("Роль в системе:", m_roleLabel);

    m_emailEdit = new QLineEdit(this);
    accountForm->addRow("Email:", m_emailEdit);

    mainLayout->addWidget(accountGroup);

    // Password Change Group
    auto *pwdGroup = new QGroupBox("Смена пароля (оставьте пустым, если не меняете)", this);
    auto *pwdForm = new QFormLayout(pwdGroup);

    m_oldPasswordEdit = new QLineEdit(this);
    m_oldPasswordEdit->setEchoMode(QLineEdit::Password);
    m_oldPasswordEdit->setPlaceholderText("Текущий пароль");
    pwdForm->addRow("Старый пароль:", m_oldPasswordEdit);

    m_newPasswordEdit = new QLineEdit(this);
    m_newPasswordEdit->setEchoMode(QLineEdit::Password);
    m_newPasswordEdit->setPlaceholderText("Мин. 8 знаков, заглавная, цифра, спецсимвол");
    pwdForm->addRow("Новый пароль:", m_newPasswordEdit);

    m_confirmPasswordEdit = new QLineEdit(this);
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPasswordEdit->setPlaceholderText("Повторите новый пароль");
    pwdForm->addRow("Подтверждение:", m_confirmPasswordEdit);

    mainLayout->addWidget(pwdGroup);

    // Customer Profile Group
    m_customerGroup = new QGroupBox("Данные организации / заказчика", this);
    auto *custForm = new QFormLayout(m_customerGroup);

    m_orgNameEdit = new QLineEdit(this);
    custForm->addRow("Организация:", m_orgNameEdit);

    m_contactPersonEdit = new QLineEdit(this);
    custForm->addRow("Контактное лицо:", m_contactPersonEdit);

    m_addressEdit = new QLineEdit(this);
    custForm->addRow("Адрес доставки:", m_addressEdit);

    m_phoneEdit = new QLineEdit(this);
    custForm->addRow("Телефон:", m_phoneEdit);

    mainLayout->addWidget(m_customerGroup);

    // Buttons
    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();

    m_saveBtn = new QPushButton("Сохранить", this);
    m_saveBtn->setDefault(true);
    connect(m_saveBtn, &QPushButton::clicked, this, &ProfileDialog::onSave);
    btnLayout->addWidget(m_saveBtn);

    m_cancelBtn = new QPushButton("Отмена", this);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(m_cancelBtn);

    mainLayout->addLayout(btnLayout);

    loadData();
}

void ProfileDialog::loadData()
{
    NetworkClient &net = NetworkClient::instance();
    bool isAdmin = net.isAdmin();
    QJsonObject user = net.currentUser();
    QJsonObject cust = net.currentCustomer();

    m_roleLabel->setText(isAdmin ? "<b>Администратор</b>" : "<b>Заказчик (Пользователь)</b>");
    m_emailEdit->setText(user["email"].toString());

    if (!isAdmin && !cust.isEmpty()) {
        m_customerGroup->setVisible(true);
        m_orgNameEdit->setText(cust["organization_name"].toString());
        m_contactPersonEdit->setText(cust["contact_person"].toString());
        m_addressEdit->setText(cust["address"].toString());
        m_phoneEdit->setText(cust["phone"].toString());
    } else {
        m_customerGroup->setVisible(false);
    }
}

void ProfileDialog::onSave()
{
    NetworkClient &net = NetworkClient::instance();
    QJsonObject params;

    QString currentEmail = net.currentUser()["email"].toString();
    QString newEmail = m_emailEdit->text().trimmed();
    if (newEmail.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Email не может быть пустым.");
        return;
    }
    if (newEmail != currentEmail) {
        params["new_email"] = newEmail;
    }

    QString oldPwd = m_oldPasswordEdit->text();
    QString newPwd = m_newPasswordEdit->text();
    QString confirmPwd = m_confirmPasswordEdit->text();

    if (!newPwd.isEmpty() || !oldPwd.isEmpty()) {
        if (oldPwd.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Введите старый пароль для смены пароля.");
            return;
        }
        if (newPwd.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Введите новый пароль.");
            return;
        }
        if (newPwd != confirmPwd) {
            QMessageBox::warning(this, "Ошибка", "Новые пароли не совпадают.");
            return;
        }
        params["old_password"] = oldPwd;
        params["new_password"] = newPwd;
    }

    if (m_customerGroup->isVisible()) {
        params["organization_name"] = m_orgNameEdit->text().trimmed();
        params["contact_person"] = m_contactPersonEdit->text().trimmed();
        params["address"] = m_addressEdit->text().trimmed();
        params["phone"] = m_phoneEdit->text().trimmed();
    }

    if (params.isEmpty()) {
        QMessageBox::information(this, "Информация", "Изменений не внесено.");
        accept();
        return;
    }

    auto resp = net.sendRequest("update_profile", params);
    if (resp["success"].toBool()) {
        QJsonObject data = resp["data"].toObject();
        // Update session state in NetworkClient
        net.setSession(data);
        QMessageBox::information(this, "Успех", "Профиль успешно обновлен.");
        accept();
    } else {
        QMessageBox::warning(this, "Ошибка обновления", resp["error"].toString());
    }
}
