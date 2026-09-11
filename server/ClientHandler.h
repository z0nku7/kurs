#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonArray>

#include "User.h"
#include "Role.h"
#include "Customer.h"

class ClientHandler : public QObject
{
    Q_OBJECT

public:
    explicit ClientHandler(qintptr socketDescriptor, QObject *parent = nullptr);

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    void processRequest(const QByteArray &data);
    void sendResponse(const QByteArray &response);

    // Action handlers
    QByteArray handleLogin(const QJsonObject &params);
    QByteArray handleRegister(const QJsonObject &params);
    QByteArray handleUpdateProfile(const QJsonObject &params);

    QByteArray handleGetProducts(const QJsonObject &params);
    QByteArray handleCreateProduct(const QJsonObject &params);
    QByteArray handleUpdateProduct(const QJsonObject &params);
    QByteArray handleDeleteProduct(const QJsonObject &params);

    QByteArray handleGetCustomers(const QJsonObject &params);
    QByteArray handleCreateCustomer(const QJsonObject &params);
    QByteArray handleUpdateCustomer(const QJsonObject &params);
    QByteArray handleDeleteCustomer(const QJsonObject &params);

    QByteArray handleGetOrders(const QJsonObject &params);
    QByteArray handleGetMyOrders(const QJsonObject &params);
    QByteArray handleCreateOrder(const QJsonObject &params);
    QByteArray handleUpdateOrder(const QJsonObject &params);
    QByteArray handleDeleteOrder(const QJsonObject &params);

    QByteArray handleGetOrderItems(const QJsonObject &params);
    QByteArray handleCreateOrderItem(const QJsonObject &params);
    QByteArray handleDeleteOrderItem(const QJsonObject &params);

    QByteArray handleGetDeliveryMethods(const QJsonObject &params);
    QByteArray handleCreateDeliveryMethod(const QJsonObject &params);
    QByteArray handleUpdateDeliveryMethod(const QJsonObject &params);
    QByteArray handleDeleteDeliveryMethod(const QJsonObject &params);

    QByteArray handleGetDeliveryTariffs(const QJsonObject &params);
    QByteArray handleCreateDeliveryTariff(const QJsonObject &params);
    QByteArray handleUpdateDeliveryTariff(const QJsonObject &params);
    QByteArray handleDeleteDeliveryTariff(const QJsonObject &params);

    QByteArray handleGetUsers(const QJsonObject &params);
    QByteArray handleCreateUser(const QJsonObject &params);
    QByteArray handleUpdateUser(const QJsonObject &params);
    QByteArray handleDeleteUser(const QJsonObject &params);

    QByteArray handleGetRoles(const QJsonObject &params);

    // Auth helpers
    bool isAuthenticated() const;
    bool isAdmin() const;
    static bool isPasswordValid(const QString &password);

    QTcpSocket *m_socket;
    QByteArray m_buffer;

    // Session state
    bool m_authenticated = false;
    User m_currentUser;
    Role m_currentRole;
    Customer m_currentCustomer;
};

#endif // CLIENTHANDLER_H
