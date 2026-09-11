#include "DatabaseManager.h"

DatabaseManager::DatabaseManager() {}
DatabaseManager::~DatabaseManager() { disconnect(); }

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

bool DatabaseManager::connectToDatabase(const QString &host, int port,
                                         const QString &dbName,
                                         const QString &user, const QString &password) {
    if (m_db.isOpen()) return true;
    m_db = QSqlDatabase::addDatabase("QPSQL");
    m_db.setHostName(host);
    m_db.setPort(port);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(user);
    m_db.setPassword(password);
    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        qCritical() << "DB connection failed:" << m_lastError;
        return false;
    }
    qDebug() << "Connected to database:" << dbName;
    return true;
}

bool DatabaseManager::connectToSqlite(const QString &dbFilePath) {
    if (m_db.isOpen()) return true;
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbFilePath);
    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        qCritical() << "SQLite DB connection failed:" << m_lastError;
        return false;
    }
    // Enable SQLite foreign keys
    QSqlQuery pragma(m_db);
    pragma.exec("PRAGMA foreign_keys = ON;");
    qDebug() << "Connected to SQLite database:" << dbFilePath;
    return true;
}

bool DatabaseManager::isConnected() const { return m_db.isOpen(); }
void DatabaseManager::disconnect() { if (m_db.isOpen()) m_db.close(); }

bool DatabaseManager::initializeSchema(const QString &sqlFilePath) {
    QFile file(sqlFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = "Cannot open SQL file: " + sqlFilePath;
        return false;
    }
    QTextStream in(&file);
    QString sql = in.readAll();
    file.close();
    // Strip line comments (-- ...) first
    QString cleanSql;
    const QStringList lines = sql.split('\n');
    for (const QString &line : lines) {
        QString t = line.trimmed();
        if (!t.startsWith("--")) {
            cleanSql.append(line).append('\n');
        }
    }

    QStringList statements = cleanSql.split(';', Qt::SkipEmptyParts);
    for (const QString &stmt : statements) {
        QString trimmed = stmt.trimmed();
        if (trimmed.isEmpty()) continue;
        QSqlQuery query(m_db);
        if (!query.exec(trimmed)) {
            QString err = query.lastError().text();
            if (!err.contains("already exists", Qt::CaseInsensitive))
                qWarning() << "SQL Error:" << err;
        }
    }
    qDebug() << "Database schema initialized.";
    return true;
}

QString DatabaseManager::hashPassword(const QString &password) {
    return QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QString DatabaseManager::lastError() const { return m_lastError; }

// === Roles ===
QVector<Role> DatabaseManager::getAllRoles() {
    QVector<Role> r;
    QSqlQuery q(m_db); q.prepare("SELECT role_id, role_name FROM roles ORDER BY role_id");
    if (q.exec()) while (q.next()) r.append(Role::fromRecord(q.record()));
    return r;
}
Role DatabaseManager::getRoleById(int id) {
    QSqlQuery q(m_db); q.prepare("SELECT role_id, role_name FROM roles WHERE role_id = :id");
    q.bindValue(":id", id);
    if (q.exec() && q.next()) return Role::fromRecord(q.record());
    return {};
}
Role DatabaseManager::getRoleByName(const QString &name) {
    QSqlQuery q(m_db); q.prepare("SELECT role_id, role_name FROM roles WHERE role_name = :n");
    q.bindValue(":n", name);
    if (q.exec() && q.next()) return Role::fromRecord(q.record());
    return {};
}

// === Users ===
QVector<User> DatabaseManager::getAllUsers() {
    QVector<User> r;
    QSqlQuery q(m_db); q.prepare("SELECT user_id, role_id, email, password_hash FROM users ORDER BY user_id");
    if (q.exec()) while (q.next()) r.append(User::fromRecord(q.record()));
    return r;
}
User DatabaseManager::getUserById(int id) {
    QSqlQuery q(m_db); q.prepare("SELECT user_id, role_id, email, password_hash FROM users WHERE user_id = :id");
    q.bindValue(":id", id);
    if (q.exec() && q.next()) return User::fromRecord(q.record());
    return {};
}
User DatabaseManager::getUserByEmail(const QString &email) {
    QSqlQuery q(m_db); q.prepare("SELECT user_id, role_id, email, password_hash FROM users WHERE email = :e");
    q.bindValue(":e", email);
    if (q.exec() && q.next()) return User::fromRecord(q.record());
    return {};
}
int DatabaseManager::createUser(int roleId, const QString &email, const QString &passwordHash) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO users (role_id, email, password_hash) VALUES (:r, :e, :p) RETURNING user_id");
    q.bindValue(":r", roleId); q.bindValue(":e", email); q.bindValue(":p", passwordHash);
    if (q.exec() && q.next()) return q.value(0).toInt();
    m_lastError = q.lastError().text(); return -1;
}
bool DatabaseManager::updateUser(int userId, int roleId, const QString &email, const QString &passwordHash) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE users SET role_id=:r, email=:e, password_hash=:p WHERE user_id=:id");
    q.bindValue(":r", roleId); q.bindValue(":e", email); q.bindValue(":p", passwordHash); q.bindValue(":id", userId);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return q.numRowsAffected() > 0;
}
bool DatabaseManager::deleteUser(int id) {
    QSqlQuery q(m_db); q.prepare("DELETE FROM users WHERE user_id = :id");
    q.bindValue(":id", id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return q.numRowsAffected() > 0;
}

// === Customers ===
QVector<Customer> DatabaseManager::getAllCustomers() {
    QVector<Customer> r;
    QSqlQuery q(m_db); q.prepare("SELECT customer_id, user_id, organization_name, contact_person, address, phone FROM customers ORDER BY customer_id");
    if (q.exec()) while (q.next()) r.append(Customer::fromRecord(q.record()));
    return r;
}
Customer DatabaseManager::getCustomerById(int id) {
    QSqlQuery q(m_db); q.prepare("SELECT customer_id, user_id, organization_name, contact_person, address, phone FROM customers WHERE customer_id = :id");
    q.bindValue(":id", id);
    if (q.exec() && q.next()) return Customer::fromRecord(q.record());
    return {};
}
Customer DatabaseManager::getCustomerByUserId(int userId) {
    QSqlQuery q(m_db); q.prepare("SELECT customer_id, user_id, organization_name, contact_person, address, phone FROM customers WHERE user_id = :uid");
    q.bindValue(":uid", userId);
    if (q.exec() && q.next()) return Customer::fromRecord(q.record());
    return {};
}
int DatabaseManager::createCustomer(int userId, const QString &orgName, const QString &contactPerson,
                                     const QString &address, const QString &phone) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO customers (user_id, organization_name, contact_person, address, phone) VALUES (:u, :o, :c, :a, :p) RETURNING customer_id");
    q.bindValue(":u", userId); q.bindValue(":o", orgName); q.bindValue(":c", contactPerson);
    q.bindValue(":a", address); q.bindValue(":p", phone);
    if (q.exec() && q.next()) return q.value(0).toInt();
    m_lastError = q.lastError().text(); return -1;
}
bool DatabaseManager::updateCustomer(int id, const QString &orgName, const QString &contactPerson,
                                      const QString &address, const QString &phone) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE customers SET organization_name=:o, contact_person=:c, address=:a, phone=:p WHERE customer_id=:id");
    q.bindValue(":o", orgName); q.bindValue(":c", contactPerson); q.bindValue(":a", address);
    q.bindValue(":p", phone); q.bindValue(":id", id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return q.numRowsAffected() > 0;
}
bool DatabaseManager::deleteCustomer(int id) {
    QSqlQuery q(m_db); q.prepare("DELETE FROM customers WHERE customer_id = :id");
    q.bindValue(":id", id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return q.numRowsAffected() > 0;
}

// === Products ===
QVector<Product> DatabaseManager::getAllProducts() {
    QVector<Product> r;
    QSqlQuery q(m_db); q.prepare("SELECT product_id, product_name, price, reference_info, is_delivery_available FROM products ORDER BY product_id");
    if (q.exec()) while (q.next()) r.append(Product::fromRecord(q.record()));
    return r;
}
Product DatabaseManager::getProductById(int id) {
    QSqlQuery q(m_db); q.prepare("SELECT product_id, product_name, price, reference_info, is_delivery_available FROM products WHERE product_id = :id");
    q.bindValue(":id", id);
    if (q.exec() && q.next()) return Product::fromRecord(q.record());
    return {};
}
int DatabaseManager::createProduct(const QString &name, double price, const QString &refInfo, bool delivery) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO products (product_name, price, reference_info, is_delivery_available) VALUES (:n, :p, :r, :d) RETURNING product_id");
    q.bindValue(":n", name); q.bindValue(":p", price); q.bindValue(":r", refInfo); q.bindValue(":d", delivery);
    if (q.exec() && q.next()) return q.value(0).toInt();
    m_lastError = q.lastError().text(); return -1;
}
bool DatabaseManager::updateProduct(int id, const QString &name, double price, const QString &refInfo, bool delivery) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE products SET product_name=:n, price=:p, reference_info=:r, is_delivery_available=:d WHERE product_id=:id");
    q.bindValue(":n", name); q.bindValue(":p", price); q.bindValue(":r", refInfo); q.bindValue(":d", delivery); q.bindValue(":id", id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return q.numRowsAffected() > 0;
}
bool DatabaseManager::deleteProduct(int id) {
    QSqlQuery q(m_db); q.prepare("DELETE FROM products WHERE product_id = :id");
    q.bindValue(":id", id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return q.numRowsAffected() > 0;
}

// === Delivery Methods ===
QVector<DeliveryMethod> DatabaseManager::getAllDeliveryMethods() {
    QVector<DeliveryMethod> r;
    QSqlQuery q(m_db); q.prepare("SELECT delivery_method_id, method_name, speed FROM delivery_methods ORDER BY delivery_method_id");
    if (q.exec()) while (q.next()) r.append(DeliveryMethod::fromRecord(q.record()));
    return r;
}
DeliveryMethod DatabaseManager::getDeliveryMethodById(int id) {
    QSqlQuery q(m_db); q.prepare("SELECT delivery_method_id, method_name, speed FROM delivery_methods WHERE delivery_method_id = :id");
    q.bindValue(":id", id);
    if (q.exec() && q.next()) return DeliveryMethod::fromRecord(q.record());
    return {};
}
int DatabaseManager::createDeliveryMethod(const QString &name, const QString &speed) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO delivery_methods (method_name, speed) VALUES (:n, :s) RETURNING delivery_method_id");
    q.bindValue(":n", name); q.bindValue(":s", speed);
    if (q.exec() && q.next()) return q.value(0).toInt();
    m_lastError = q.lastError().text(); return -1;
}
bool DatabaseManager::updateDeliveryMethod(int id, const QString &name, const QString &speed) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE delivery_methods SET method_name=:n, speed=:s WHERE delivery_method_id=:id");
    q.bindValue(":n", name); q.bindValue(":s", speed); q.bindValue(":id", id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return q.numRowsAffected() > 0;
}
bool DatabaseManager::deleteDeliveryMethod(int id) {
    QSqlQuery q(m_db); q.prepare("DELETE FROM delivery_methods WHERE delivery_method_id = :id");
    q.bindValue(":id", id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return q.numRowsAffected() > 0;
}

// === Delivery Tariffs ===
QVector<DeliveryTariff> DatabaseManager::getAllDeliveryTariffs() {
    QVector<DeliveryTariff> r;
    QSqlQuery q(m_db); q.prepare("SELECT delivery_tariff_id, product_id, delivery_method_id, base_delivery_price FROM delivery_tariffs ORDER BY delivery_tariff_id");
    if (q.exec()) while (q.next()) r.append(DeliveryTariff::fromRecord(q.record()));
    return r;
}
QVector<DeliveryTariff> DatabaseManager::getDeliveryTariffsByProduct(int productId) {
    QVector<DeliveryTariff> r;
    QSqlQuery q(m_db); q.prepare("SELECT delivery_tariff_id, product_id, delivery_method_id, base_delivery_price FROM delivery_tariffs WHERE product_id = :pid ORDER BY delivery_tariff_id");
    q.bindValue(":pid", productId);
    if (q.exec()) while (q.next()) r.append(DeliveryTariff::fromRecord(q.record()));
    return r;
}
DeliveryTariff DatabaseManager::getDeliveryTariffById(int id) {
    QSqlQuery q(m_db); q.prepare("SELECT delivery_tariff_id, product_id, delivery_method_id, base_delivery_price FROM delivery_tariffs WHERE delivery_tariff_id = :id");
    q.bindValue(":id", id);
    if (q.exec() && q.next()) return DeliveryTariff::fromRecord(q.record());
    return {};
}
int DatabaseManager::createDeliveryTariff(int productId, int methodId, double basePrice) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO delivery_tariffs (product_id, delivery_method_id, base_delivery_price) VALUES (:p, :m, :b) RETURNING delivery_tariff_id");
    q.bindValue(":p", productId); q.bindValue(":m", methodId); q.bindValue(":b", basePrice);
    if (q.exec() && q.next()) return q.value(0).toInt();
    m_lastError = q.lastError().text(); return -1;
}
bool DatabaseManager::updateDeliveryTariff(int id, int productId, int methodId, double basePrice) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE delivery_tariffs SET product_id=:p, delivery_method_id=:m, base_delivery_price=:b WHERE delivery_tariff_id=:id");
    q.bindValue(":p", productId); q.bindValue(":m", methodId); q.bindValue(":b", basePrice); q.bindValue(":id", id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return q.numRowsAffected() > 0;
}
bool DatabaseManager::deleteDeliveryTariff(int id) {
    QSqlQuery q(m_db); q.prepare("DELETE FROM delivery_tariffs WHERE delivery_tariff_id = :id");
    q.bindValue(":id", id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return q.numRowsAffected() > 0;
}

// === Orders ===
QVector<Order> DatabaseManager::getAllOrders() {
    QVector<Order> r;
    QSqlQuery q(m_db); q.prepare("SELECT order_id, customer_id, purchase_date FROM orders ORDER BY order_id DESC");
    if (q.exec()) while (q.next()) r.append(Order::fromRecord(q.record()));
    return r;
}
QVector<Order> DatabaseManager::getOrdersByCustomer(int customerId) {
    QVector<Order> r;
    QSqlQuery q(m_db); q.prepare("SELECT order_id, customer_id, purchase_date FROM orders WHERE customer_id = :cid ORDER BY order_id DESC");
    q.bindValue(":cid", customerId);
    if (q.exec()) while (q.next()) r.append(Order::fromRecord(q.record()));
    return r;
}
Order DatabaseManager::getOrderById(int id) {
    QSqlQuery q(m_db); q.prepare("SELECT order_id, customer_id, purchase_date FROM orders WHERE order_id = :id");
    q.bindValue(":id", id);
    if (q.exec() && q.next()) return Order::fromRecord(q.record());
    return {};
}
int DatabaseManager::createOrder(int customerId, const QString &purchaseDate) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO orders (customer_id, purchase_date) VALUES (:c, :d) RETURNING order_id");
    q.bindValue(":c", customerId); q.bindValue(":d", purchaseDate);
    if (q.exec() && q.next()) return q.value(0).toInt();
    m_lastError = q.lastError().text(); return -1;
}
bool DatabaseManager::updateOrder(int id, int customerId, const QString &purchaseDate) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE orders SET customer_id=:c, purchase_date=:d WHERE order_id=:id");
    q.bindValue(":c", customerId); q.bindValue(":d", purchaseDate); q.bindValue(":id", id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return q.numRowsAffected() > 0;
}
bool DatabaseManager::deleteOrder(int id) {
    QSqlQuery q(m_db); q.prepare("DELETE FROM orders WHERE order_id = :id");
    q.bindValue(":id", id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return q.numRowsAffected() > 0;
}

// === Order Items ===
QVector<OrderItem> DatabaseManager::getOrderItemsByOrder(int orderId) {
    QVector<OrderItem> r;
    QSqlQuery q(m_db); q.prepare("SELECT order_item_id, order_id, product_id, delivery_method_id, actual_delivery_cost, quantity FROM order_items WHERE order_id = :oid ORDER BY order_item_id");
    q.bindValue(":oid", orderId);
    if (q.exec()) while (q.next()) r.append(OrderItem::fromRecord(q.record()));
    return r;
}
OrderItem DatabaseManager::getOrderItemById(int id) {
    QSqlQuery q(m_db); q.prepare("SELECT order_item_id, order_id, product_id, delivery_method_id, actual_delivery_cost, quantity FROM order_items WHERE order_item_id = :id");
    q.bindValue(":id", id);
    if (q.exec() && q.next()) return OrderItem::fromRecord(q.record());
    return {};
}
int DatabaseManager::createOrderItem(int orderId, int productId, int deliveryMethodId,
                                      double cost, int quantity) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO order_items (order_id, product_id, delivery_method_id, actual_delivery_cost, quantity) VALUES (:o, :p, :m, :c, :q) RETURNING order_item_id");
    q.bindValue(":o", orderId); q.bindValue(":p", productId); q.bindValue(":m", deliveryMethodId);
    q.bindValue(":c", cost); q.bindValue(":q", quantity);
    if (q.exec() && q.next()) return q.value(0).toInt();
    m_lastError = q.lastError().text(); return -1;
}
bool DatabaseManager::updateOrderItem(int id, int orderId, int productId, int deliveryMethodId,
                                       double cost, int quantity) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE order_items SET order_id=:o, product_id=:p, delivery_method_id=:m, actual_delivery_cost=:c, quantity=:q WHERE order_item_id=:id");
    q.bindValue(":o", orderId); q.bindValue(":p", productId); q.bindValue(":m", deliveryMethodId);
    q.bindValue(":c", cost); q.bindValue(":q", quantity); q.bindValue(":id", id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return q.numRowsAffected() > 0;
}
bool DatabaseManager::deleteOrderItem(int id) {
    QSqlQuery q(m_db); q.prepare("DELETE FROM order_items WHERE order_item_id = :id");
    q.bindValue(":id", id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return q.numRowsAffected() > 0;
}
