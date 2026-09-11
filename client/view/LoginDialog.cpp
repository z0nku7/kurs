#include "LoginDialog.h"
#include "NetworkClient.h"

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Система управления заказами — Авторизация");
    setMinimumSize(450, 520);
    setupUi();
}

void LoginDialog::setupUi() {
    auto *main = new QVBoxLayout(this);
    auto *title = new QLabel("Информационная система\nведения заказов");
    title->setAlignment(Qt::AlignCenter);
    QFont f; f.setPointSize(16); f.setBold(true);
    title->setFont(f);
    main->addWidget(title);
    main->addSpacing(10);

    auto *tabs = new QTabWidget;

    // Login tab
    auto *loginTab = new QWidget;
    auto *ll = new QVBoxLayout(loginTab);
    auto *lf = new QFormLayout;
    m_loginEmail = new QLineEdit; m_loginEmail->setPlaceholderText("example@mail.com");
    lf->addRow("Email:", m_loginEmail);
    m_loginPassword = new QLineEdit; m_loginPassword->setEchoMode(QLineEdit::Password);
    lf->addRow("Пароль:", m_loginPassword);
    ll->addLayout(lf);
    ll->addSpacing(20);
    auto *loginBtn = new QPushButton("Войти");
    loginBtn->setMinimumHeight(40);
    connect(loginBtn, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(m_loginPassword, &QLineEdit::returnPressed, loginBtn, &QPushButton::click);
    ll->addWidget(loginBtn);
    ll->addStretch();
    tabs->addTab(loginTab, "Вход");

    // Register tab
    auto *regTab = new QWidget;
    auto *rl = new QVBoxLayout(regTab);
    auto *rf = new QFormLayout;
    m_regEmail = new QLineEdit; m_regEmail->setPlaceholderText("example@mail.com");
    rf->addRow("Email:", m_regEmail);
    m_regPassword = new QLineEdit; m_regPassword->setEchoMode(QLineEdit::Password);
    m_regPassword->setPlaceholderText("Мин. 8 символов, A-Z, 0-9, спецсимвол");
    rf->addRow("Пароль:", m_regPassword);
    m_regPasswordConfirm = new QLineEdit; m_regPasswordConfirm->setEchoMode(QLineEdit::Password);
    rf->addRow("Подтверждение:", m_regPasswordConfirm);

    auto *sep = new QLabel("— Данные организации —");
    sep->setAlignment(Qt::AlignCenter);
    rf->addRow(sep);
    m_regOrgName = new QLineEdit; m_regOrgName->setPlaceholderText("ООО \"Компания\"");
    rf->addRow("Организация:", m_regOrgName);
    m_regContact = new QLineEdit; m_regContact->setPlaceholderText("Иванов И.И.");
    rf->addRow("Контактное лицо:", m_regContact);
    m_regAddress = new QLineEdit; m_regAddress->setPlaceholderText("г. Москва, ул. ...");
    rf->addRow("Адрес:", m_regAddress);
    m_regPhone = new QLineEdit; m_regPhone->setPlaceholderText("+7 (999) 123-45-67");
    rf->addRow("Телефон:", m_regPhone);

    rl->addLayout(rf);
    rl->addSpacing(10);
    auto *regBtn = new QPushButton("Зарегистрироваться");
    regBtn->setMinimumHeight(40);
    connect(regBtn, &QPushButton::clicked, this, &LoginDialog::onRegister);
    rl->addWidget(regBtn);
    rl->addStretch();
    tabs->addTab(regTab, "Регистрация");

    main->addWidget(tabs);
}

void LoginDialog::onLogin() {
    QJsonObject params;
    params["email"] = m_loginEmail->text().trimmed();
    params["password"] = m_loginPassword->text();
    if (params["email"].toString().isEmpty() || params["password"].toString().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля."); return;
    }
    QJsonObject resp = NetworkClient::instance().sendRequest("login", params);
    if (resp["success"].toBool()) {
        NetworkClient::instance().setSession(resp["data"].toObject());
        accept();
    } else {
        QMessageBox::warning(this, "Ошибка входа", resp["error"].toString());
    }
}

void LoginDialog::onRegister() {
    if (m_regPassword->text() != m_regPasswordConfirm->text()) {
        QMessageBox::warning(this, "Ошибка", "Пароли не совпадают."); return;
    }
    QJsonObject params;
    params["email"] = m_regEmail->text().trimmed();
    params["password"] = m_regPassword->text();
    params["organization_name"] = m_regOrgName->text().trimmed();
    params["contact_person"] = m_regContact->text().trimmed();
    params["address"] = m_regAddress->text().trimmed();
    params["phone"] = m_regPhone->text().trimmed();

    QJsonObject resp = NetworkClient::instance().sendRequest("register", params);
    if (resp["success"].toBool()) {
        QMessageBox::information(this, "Успех", "Регистрация прошла успешно! Теперь войдите.");
        m_regEmail->clear(); m_regPassword->clear(); m_regPasswordConfirm->clear();
        m_regOrgName->clear(); m_regContact->clear(); m_regAddress->clear(); m_regPhone->clear();
    } else {
        QMessageBox::warning(this, "Ошибка регистрации", resp["error"].toString());
    }
}
