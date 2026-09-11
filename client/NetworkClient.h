#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "Protocol.h"

class NetworkClient : public QObject
{
    Q_OBJECT

public:
    static NetworkClient& instance();
    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator=(const NetworkClient&) = delete;

    bool connectToServer(const QString &host, quint16 port);
    void disconnect();
    bool isConnected() const;

    // Synchronous request-response
    QJsonObject sendRequest(const QString &action, const QJsonObject &params = {});

    // Session info (set after login)
    bool isLoggedIn() const { return m_loggedIn; }
    bool isAdmin() const { return m_isAdmin; }
    QJsonObject currentUser() const { return m_currentUser; }
    QJsonObject currentCustomer() const { return m_currentCustomer; }
    int currentCustomerId() const { return m_currentCustomer["customer_id"].toInt(); }

    void setSession(const QJsonObject &loginData);
    void clearSession();

private:
    NetworkClient();
    ~NetworkClient();

    QTcpSocket *m_socket;
    bool m_loggedIn = false;
    bool m_isAdmin = false;
    QJsonObject m_currentUser;
    QJsonObject m_currentCustomer;
};

#endif // NETWORKCLIENT_H
