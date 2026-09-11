#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QVariant>
#include <QVector>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QCryptographicHash>

#include "Role.h"
#include "User.h"
#include "Customer.h"
#include "Product.h"
#include "DeliveryMethod.h"
#include "DeliveryTariff.h"
#include "Order.h"
#include "OrderItem.h"

class DatabaseManager
{
public:
    static DatabaseManager& instance();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool connectToDatabase(const QString &host, int port, const QString &dbName,
                           const QString &user, const QString &password);
    bool connectToSqlite(const QString &dbFilePath = "orders.db");
    bool isConnected() const;
    void disconnect();
    bool initializeSchema(const QString &sqlFilePath);

    // Roles
    QVector<Role> getAllRoles();
    Role getRoleById(int roleId);
    Role getRoleByName(const QString &name);

    // Users
    QVector<User> getAllUsers();
    User getUserById(int userId);
    User getUserByEmail(const QString &email);
    int createUser(int roleId, const QString &email, const QString &passwordHash);
    bool updateUser(int userId, int roleId, const QString &email, const QString &passwordHash);
    bool deleteUser(int userId);

    // Customers
    QVector<Customer> getAllCustomers();
    Customer getCustomerById(int customerId);
    Customer getCustomerByUserId(int userId);
    int createCustomer(int userId, const QString &orgName, const QString &contactPerson,
                       const QString &address, const QString &phone);
    bool updateCustomer(int customerId, const QString &orgName, const QString &contactPerson,
                        const QString &address, const QString &phone);
    bool deleteCustomer(int customerId);

    // Products
    QVector<Product> getAllProducts();
    Product getProductById(int productId);
    int createProduct(const QString &name, double price, const QString &refInfo, bool delivery);
    bool updateProduct(int id, const QString &name, double price, const QString &refInfo, bool delivery);
    bool deleteProduct(int id);

    // Delivery Methods
    QVector<DeliveryMethod> getAllDeliveryMethods();
    DeliveryMethod getDeliveryMethodById(int id);
    int createDeliveryMethod(const QString &name, const QString &speed);
    bool updateDeliveryMethod(int id, const QString &name, const QString &speed);
    bool deleteDeliveryMethod(int id);

    // Delivery Tariffs
    QVector<DeliveryTariff> getAllDeliveryTariffs();
    QVector<DeliveryTariff> getDeliveryTariffsByProduct(int productId);
    DeliveryTariff getDeliveryTariffById(int id);
    int createDeliveryTariff(int productId, int methodId, double basePrice);
    bool updateDeliveryTariff(int id, int productId, int methodId, double basePrice);
    bool deleteDeliveryTariff(int id);

    // Orders
    QVector<Order> getAllOrders();
    QVector<Order> getOrdersByCustomer(int customerId);
    Order getOrderById(int id);
    int createOrder(int customerId, const QString &purchaseDate);
    bool updateOrder(int id, int customerId, const QString &purchaseDate);
    bool deleteOrder(int id);

    // Order Items
    QVector<OrderItem> getOrderItemsByOrder(int orderId);
    OrderItem getOrderItemById(int id);
    int createOrderItem(int orderId, int productId, int deliveryMethodId,
                        double cost, int quantity);
    bool updateOrderItem(int id, int orderId, int productId, int deliveryMethodId,
                         double cost, int quantity);
    bool deleteOrderItem(int id);

    static QString hashPassword(const QString &password);
    QString lastError() const;

private:
    DatabaseManager();
    ~DatabaseManager();
    QSqlDatabase m_db;
    QString m_lastError;
};

#endif // DATABASEMANAGER_H
