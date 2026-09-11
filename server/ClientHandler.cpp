#include "ClientHandler.h"
#include "DatabaseManager.h"
#include "Protocol.h"
#include <QRegularExpression>

ClientHandler::ClientHandler(qintptr socketDescriptor, QObject *parent)
    : QObject(parent)
{
    m_socket = new QTcpSocket(this);
    m_socket->setSocketDescriptor(socketDescriptor);
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientHandler::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientHandler::onDisconnected);
}

void ClientHandler::onReadyRead()
{
    m_buffer.append(m_socket->readAll());
    while (m_buffer.contains('\n')) {
        int idx = m_buffer.indexOf('\n');
        QByteArray line = m_buffer.left(idx);
        m_buffer.remove(0, idx + 1);
        processRequest(line);
    }
}

void ClientHandler::onDisconnected()
{
    qDebug() << "Client disconnected";
    deleteLater();
}

void ClientHandler::sendResponse(const QByteArray &response)
{
    m_socket->write(response);
    m_socket->flush();
}

void ClientHandler::processRequest(const QByteArray &data)
{
    QJsonObject req = Protocol::parse(data);
    QString action = req["action"].toString();
    QJsonObject params = req["params"].toObject();

    QByteArray response;

    // Auth-free actions
    if (action == "login")           { response = handleLogin(params); }
    else if (action == "register")   { response = handleRegister(params); }
    // Auth-required actions
    else if (!isAuthenticated())     { response = Protocol::makeError("Необходима авторизация."); }
    else if (action == "update_profile")       { response = handleUpdateProfile(params); }
    else if (action == "get_products")         { response = handleGetProducts(params); }
    else if (action == "create_product")       { response = handleCreateProduct(params); }
    else if (action == "update_product")       { response = handleUpdateProduct(params); }
    else if (action == "delete_product")       { response = handleDeleteProduct(params); }
    else if (action == "get_customers")        { response = handleGetCustomers(params); }
    else if (action == "create_customer")      { response = handleCreateCustomer(params); }
    else if (action == "update_customer")      { response = handleUpdateCustomer(params); }
    else if (action == "delete_customer")      { response = handleDeleteCustomer(params); }
    else if (action == "get_orders")           { response = handleGetOrders(params); }
    else if (action == "get_my_orders")        { response = handleGetMyOrders(params); }
    else if (action == "create_order")         { response = handleCreateOrder(params); }
    else if (action == "update_order")         { response = handleUpdateOrder(params); }
    else if (action == "delete_order")         { response = handleDeleteOrder(params); }
    else if (action == "get_order_items")      { response = handleGetOrderItems(params); }
    else if (action == "create_order_item")    { response = handleCreateOrderItem(params); }
    else if (action == "delete_order_item")    { response = handleDeleteOrderItem(params); }
    else if (action == "get_delivery_methods") { response = handleGetDeliveryMethods(params); }
    else if (action == "create_delivery_method") { response = handleCreateDeliveryMethod(params); }
    else if (action == "update_delivery_method") { response = handleUpdateDeliveryMethod(params); }
    else if (action == "delete_delivery_method") { response = handleDeleteDeliveryMethod(params); }
    else if (action == "get_delivery_tariffs") { response = handleGetDeliveryTariffs(params); }
    else if (action == "create_delivery_tariff") { response = handleCreateDeliveryTariff(params); }
    else if (action == "update_delivery_tariff") { response = handleUpdateDeliveryTariff(params); }
    else if (action == "delete_delivery_tariff") { response = handleDeleteDeliveryTariff(params); }
    else if (action == "get_users")            { response = handleGetUsers(params); }
    else if (action == "create_user")          { response = handleCreateUser(params); }
    else if (action == "update_user")          { response = handleUpdateUser(params); }
    else if (action == "delete_user")          { response = handleDeleteUser(params); }
    else if (action == "get_roles")            { response = handleGetRoles(params); }
    else { response = Protocol::makeError("Неизвестное действие: " + action); }

    sendResponse(response);
}

bool ClientHandler::isAuthenticated() const { return m_authenticated; }
bool ClientHandler::isAdmin() const { return m_authenticated && m_currentRole.roleName() == "admin"; }

bool ClientHandler::isPasswordValid(const QString &password) {
    QRegularExpression regex(R"(^(?=.*[A-Z])(?=.*\d)(?=.*[!@#$%^&*()\-_=+\[\]{};':"\\|,.<>\/?]).{8,}$)");
    return regex.match(password).hasMatch();
}

// ====================== AUTH ======================

QByteArray ClientHandler::handleLogin(const QJsonObject &params) {
    DatabaseManager &db = DatabaseManager::instance();
    QString email = params["email"].toString();
    QString password = params["password"].toString();

    User user = db.getUserByEmail(email);
    if (!user.isValid()) return Protocol::makeError("Пользователь с таким email не найден.");

    if (user.passwordHash() != DatabaseManager::hashPassword(password))
        return Protocol::makeError("Неверный пароль.");

    m_currentUser = user;
    m_currentRole = db.getRoleById(user.roleId());
    m_currentCustomer = db.getCustomerByUserId(user.userId());
    m_authenticated = true;

    QJsonObject data;
    data["user"] = user.toJson();
    data["role"] = m_currentRole.toJson();
    if (m_currentCustomer.isValid())
        data["customer"] = m_currentCustomer.toJson();
    return Protocol::makeSuccess(data);
}

QByteArray ClientHandler::handleRegister(const QJsonObject &params) {
    QString email = params["email"].toString();
    QString password = params["password"].toString();
    QString orgName = params["organization_name"].toString();
    QString contact = params["contact_person"].toString();
    QString address = params["address"].toString();
    QString phone = params["phone"].toString();

    if (email.isEmpty() || password.isEmpty() || orgName.isEmpty() ||
        contact.isEmpty() || address.isEmpty() || phone.isEmpty())
        return Protocol::makeError("Заполните все поля.");

    if (!isPasswordValid(password))
        return Protocol::makeError("Пароль должен содержать не менее 8 символов, хотя бы одну заглавную букву, одну цифру и один специальный символ.");

    DatabaseManager &db = DatabaseManager::instance();
    if (db.getUserByEmail(email).isValid())
        return Protocol::makeError("Пользователь с таким email уже существует.");

    Role userRole = db.getRoleByName("user");
    if (!userRole.isValid()) userRole = db.getRoleByName("customer");
    if (!userRole.isValid()) return Protocol::makeError("Роль 'user' или 'customer' не найдена.");

    QString hash = DatabaseManager::hashPassword(password);
    int userId = db.createUser(userRole.roleId(), email, hash);
    if (userId < 0) return Protocol::makeError("Ошибка создания пользователя: " + db.lastError());

    int custId = db.createCustomer(userId, orgName, contact, address, phone);
    if (custId < 0) {
        db.deleteUser(userId);
        return Protocol::makeError("Ошибка создания профиля: " + db.lastError());
    }

    return Protocol::makeSuccess(QJsonObject{{"message", "Регистрация успешна."}});
}

QByteArray ClientHandler::handleUpdateProfile(const QJsonObject &params) {
    DatabaseManager &db = DatabaseManager::instance();

    if (params.contains("new_email")) {
        QString newEmail = params["new_email"].toString();
        User existing = db.getUserByEmail(newEmail);
        if (existing.isValid() && existing.userId() != m_currentUser.userId())
            return Protocol::makeError("Этот email уже используется.");
        db.updateUser(m_currentUser.userId(), m_currentUser.roleId(), newEmail, m_currentUser.passwordHash());
        m_currentUser.setEmail(newEmail);
    }

    if (params.contains("old_password") && params.contains("new_password")) {
        QString oldHash = DatabaseManager::hashPassword(params["old_password"].toString());
        if (oldHash != m_currentUser.passwordHash())
            return Protocol::makeError("Старый пароль указан неверно.");
        QString newPwd = params["new_password"].toString();
        if (!isPasswordValid(newPwd))
            return Protocol::makeError("Новый пароль не соответствует требованиям безопасности.");
        QString newHash = DatabaseManager::hashPassword(newPwd);
        db.updateUser(m_currentUser.userId(), m_currentUser.roleId(), m_currentUser.email(), newHash);
        m_currentUser.setPasswordHash(newHash);
    }

    if (m_currentCustomer.isValid()) {
        if (params.contains("organization_name")) {
            db.updateCustomer(m_currentCustomer.customerId(),
                params["organization_name"].toString(),
                params.contains("contact_person") ? params["contact_person"].toString() : m_currentCustomer.contactPerson(),
                params.contains("address") ? params["address"].toString() : m_currentCustomer.address(),
                params.contains("phone") ? params["phone"].toString() : m_currentCustomer.phone());
            m_currentCustomer = db.getCustomerById(m_currentCustomer.customerId());
        }
    }

    QJsonObject data;
    data["user"] = m_currentUser.toJson();
    if (m_currentCustomer.isValid()) data["customer"] = m_currentCustomer.toJson();
    return Protocol::makeSuccess(data);
}

// ====================== PRODUCTS ======================

QByteArray ClientHandler::handleGetProducts(const QJsonObject &) {
    auto products = DatabaseManager::instance().getAllProducts();
    QJsonArray arr;
    for (const auto &p : products) arr.append(p.toJson());
    return Protocol::makeSuccess(arr);
}

QByteArray ClientHandler::handleCreateProduct(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    DatabaseManager &db = DatabaseManager::instance();
    int id = db.createProduct(params["product_name"].toString(), params["price"].toDouble(),
                               params["reference_info"].toString(), params["is_delivery_available"].toBool(true));
    if (id < 0) return Protocol::makeError(db.lastError());
    return Protocol::makeSuccess(QJsonObject{{"product_id", id}});
}

QByteArray ClientHandler::handleUpdateProduct(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    DatabaseManager &db = DatabaseManager::instance();
    if (!db.updateProduct(params["product_id"].toInt(), params["product_name"].toString(),
                           params["price"].toDouble(), params["reference_info"].toString(),
                           params["is_delivery_available"].toBool()))
        return Protocol::makeError(db.lastError());
    return Protocol::makeSuccess();
}

QByteArray ClientHandler::handleDeleteProduct(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    DatabaseManager &db = DatabaseManager::instance();
    if (!db.deleteProduct(params["product_id"].toInt()))
        return Protocol::makeError(db.lastError());
    return Protocol::makeSuccess();
}

// ====================== CUSTOMERS ======================

QByteArray ClientHandler::handleGetCustomers(const QJsonObject &) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    auto customers = DatabaseManager::instance().getAllCustomers();
    QJsonArray arr;
    for (const auto &c : customers) arr.append(c.toJson());
    return Protocol::makeSuccess(arr);
}

QByteArray ClientHandler::handleCreateCustomer(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    DatabaseManager &db = DatabaseManager::instance();
    int id = db.createCustomer(params["user_id"].toInt(), params["organization_name"].toString(),
                                params["contact_person"].toString(), params["address"].toString(),
                                params["phone"].toString());
    if (id < 0) return Protocol::makeError(db.lastError());
    return Protocol::makeSuccess(QJsonObject{{"customer_id", id}});
}

QByteArray ClientHandler::handleUpdateCustomer(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    DatabaseManager &db = DatabaseManager::instance();
    if (!db.updateCustomer(params["customer_id"].toInt(), params["organization_name"].toString(),
                            params["contact_person"].toString(), params["address"].toString(),
                            params["phone"].toString()))
        return Protocol::makeError(db.lastError());
    return Protocol::makeSuccess();
}

QByteArray ClientHandler::handleDeleteCustomer(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    if (!DatabaseManager::instance().deleteCustomer(params["customer_id"].toInt()))
        return Protocol::makeError(DatabaseManager::instance().lastError());
    return Protocol::makeSuccess();
}

// ====================== ORDERS ======================

QByteArray ClientHandler::handleGetOrders(const QJsonObject &) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    auto orders = DatabaseManager::instance().getAllOrders();
    QJsonArray arr;
    for (const auto &o : orders) arr.append(o.toJson());
    return Protocol::makeSuccess(arr);
}

QByteArray ClientHandler::handleGetMyOrders(const QJsonObject &) {
    if (!m_currentCustomer.isValid())
        return Protocol::makeError("Профиль заказчика не найден.");
    auto orders = DatabaseManager::instance().getOrdersByCustomer(m_currentCustomer.customerId());
    QJsonArray arr;
    for (const auto &o : orders) arr.append(o.toJson());
    return Protocol::makeSuccess(arr);
}

QByteArray ClientHandler::handleCreateOrder(const QJsonObject &params) {
    int customerId = params["customer_id"].toInt();
    if (!isAdmin()) {
        if (!m_currentCustomer.isValid())
            return Protocol::makeError("Профиль заказчика не найден.");
        if (customerId <= 0)
            customerId = m_currentCustomer.customerId();
        else if (m_currentCustomer.customerId() != customerId)
            return Protocol::makeError("Вы можете создавать заказы только для себя.");
    }
    DatabaseManager &db = DatabaseManager::instance();
    QString dateStr = params["purchase_date"].toString();
    if (dateStr.isEmpty())
        dateStr = QDate::currentDate().toString("yyyy-MM-dd");
    int id = db.createOrder(customerId, dateStr);
    if (id < 0) return Protocol::makeError(db.lastError());
    return Protocol::makeSuccess(QJsonObject{{"order_id", id}});
}

QByteArray ClientHandler::handleUpdateOrder(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    DatabaseManager &db = DatabaseManager::instance();
    if (!db.updateOrder(params["order_id"].toInt(), params["customer_id"].toInt(),
                         params["purchase_date"].toString()))
        return Protocol::makeError(db.lastError());
    return Protocol::makeSuccess();
}

QByteArray ClientHandler::handleDeleteOrder(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    if (!DatabaseManager::instance().deleteOrder(params["order_id"].toInt()))
        return Protocol::makeError(DatabaseManager::instance().lastError());
    return Protocol::makeSuccess();
}

// ====================== ORDER ITEMS ======================

QByteArray ClientHandler::handleGetOrderItems(const QJsonObject &params) {
    auto items = DatabaseManager::instance().getOrderItemsByOrder(params["order_id"].toInt());
    QJsonArray arr;
    for (const auto &i : items) arr.append(i.toJson());
    return Protocol::makeSuccess(arr);
}

QByteArray ClientHandler::handleCreateOrderItem(const QJsonObject &params) {
    DatabaseManager &db = DatabaseManager::instance();
    int id = db.createOrderItem(params["order_id"].toInt(), params["product_id"].toInt(),
                                 params["delivery_method_id"].toInt(),
                                 params["actual_delivery_cost"].toDouble(),
                                 params["quantity"].toInt());
    if (id < 0) return Protocol::makeError(db.lastError());
    return Protocol::makeSuccess(QJsonObject{{"order_item_id", id}});
}

QByteArray ClientHandler::handleDeleteOrderItem(const QJsonObject &params) {
    if (!DatabaseManager::instance().deleteOrderItem(params["order_item_id"].toInt()))
        return Protocol::makeError(DatabaseManager::instance().lastError());
    return Protocol::makeSuccess();
}

// ====================== DELIVERY METHODS ======================

QByteArray ClientHandler::handleGetDeliveryMethods(const QJsonObject &) {
    auto methods = DatabaseManager::instance().getAllDeliveryMethods();
    QJsonArray arr;
    for (const auto &m : methods) arr.append(m.toJson());
    return Protocol::makeSuccess(arr);
}

QByteArray ClientHandler::handleCreateDeliveryMethod(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    DatabaseManager &db = DatabaseManager::instance();
    int id = db.createDeliveryMethod(params["method_name"].toString(), params["speed"].toString());
    if (id < 0) return Protocol::makeError(db.lastError());
    return Protocol::makeSuccess(QJsonObject{{"delivery_method_id", id}});
}

QByteArray ClientHandler::handleUpdateDeliveryMethod(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    if (!DatabaseManager::instance().updateDeliveryMethod(params["delivery_method_id"].toInt(),
                                                           params["method_name"].toString(),
                                                           params["speed"].toString()))
        return Protocol::makeError(DatabaseManager::instance().lastError());
    return Protocol::makeSuccess();
}

QByteArray ClientHandler::handleDeleteDeliveryMethod(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    if (!DatabaseManager::instance().deleteDeliveryMethod(params["delivery_method_id"].toInt()))
        return Protocol::makeError(DatabaseManager::instance().lastError());
    return Protocol::makeSuccess();
}

// ====================== DELIVERY TARIFFS ======================

QByteArray ClientHandler::handleGetDeliveryTariffs(const QJsonObject &params) {
    QVector<DeliveryTariff> tariffs;
    if (params.contains("product_id"))
        tariffs = DatabaseManager::instance().getDeliveryTariffsByProduct(params["product_id"].toInt());
    else
        tariffs = DatabaseManager::instance().getAllDeliveryTariffs();
    QJsonArray arr;
    for (const auto &t : tariffs) arr.append(t.toJson());
    return Protocol::makeSuccess(arr);
}

QByteArray ClientHandler::handleCreateDeliveryTariff(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    DatabaseManager &db = DatabaseManager::instance();
    int id = db.createDeliveryTariff(params["product_id"].toInt(), params["delivery_method_id"].toInt(),
                                      params["base_delivery_price"].toDouble());
    if (id < 0) return Protocol::makeError(db.lastError());
    return Protocol::makeSuccess(QJsonObject{{"delivery_tariff_id", id}});
}

QByteArray ClientHandler::handleUpdateDeliveryTariff(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    if (!DatabaseManager::instance().updateDeliveryTariff(params["delivery_tariff_id"].toInt(),
                                                           params["product_id"].toInt(),
                                                           params["delivery_method_id"].toInt(),
                                                           params["base_delivery_price"].toDouble()))
        return Protocol::makeError(DatabaseManager::instance().lastError());
    return Protocol::makeSuccess();
}

QByteArray ClientHandler::handleDeleteDeliveryTariff(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    if (!DatabaseManager::instance().deleteDeliveryTariff(params["delivery_tariff_id"].toInt()))
        return Protocol::makeError(DatabaseManager::instance().lastError());
    return Protocol::makeSuccess();
}

// ====================== USERS ======================

QByteArray ClientHandler::handleGetUsers(const QJsonObject &) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    auto users = DatabaseManager::instance().getAllUsers();
    QJsonArray arr;
    for (const auto &u : users) arr.append(u.toJson());
    return Protocol::makeSuccess(arr);
}

QByteArray ClientHandler::handleCreateUser(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    QString pwd = params["password"].toString();
    if (!isPasswordValid(pwd))
        return Protocol::makeError("Пароль не соответствует требованиям безопасности.");
    DatabaseManager &db = DatabaseManager::instance();
    int id = db.createUser(params["role_id"].toInt(), params["email"].toString(),
                            DatabaseManager::hashPassword(pwd));
    if (id < 0) return Protocol::makeError(db.lastError());
    return Protocol::makeSuccess(QJsonObject{{"user_id", id}});
}

QByteArray ClientHandler::handleUpdateUser(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    DatabaseManager &db = DatabaseManager::instance();
    User existing = db.getUserById(params["user_id"].toInt());
    if (!existing.isValid()) return Protocol::makeError("Пользователь не найден.");

    QString hash = existing.passwordHash();
    if (params.contains("password") && !params["password"].toString().isEmpty()) {
        QString pwd = params["password"].toString();
        if (!isPasswordValid(pwd))
            return Protocol::makeError("Пароль не соответствует требованиям безопасности.");
        hash = DatabaseManager::hashPassword(pwd);
    }
    if (!db.updateUser(params["user_id"].toInt(), params["role_id"].toInt(),
                        params["email"].toString(), hash))
        return Protocol::makeError(db.lastError());
    return Protocol::makeSuccess();
}

QByteArray ClientHandler::handleDeleteUser(const QJsonObject &params) {
    if (!isAdmin()) return Protocol::makeError("Недостаточно прав.");
    if (!DatabaseManager::instance().deleteUser(params["user_id"].toInt()))
        return Protocol::makeError(DatabaseManager::instance().lastError());
    return Protocol::makeSuccess();
}

// ====================== ROLES ======================

QByteArray ClientHandler::handleGetRoles(const QJsonObject &) {
    auto roles = DatabaseManager::instance().getAllRoles();
    QJsonArray arr;
    for (const auto &r : roles) arr.append(r.toJson());
    return Protocol::makeSuccess(arr);
}
