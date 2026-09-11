#include "NetworkClient.h"
#include <QDebug>

NetworkClient::NetworkClient() : m_socket(new QTcpSocket(this)) {}
NetworkClient::~NetworkClient() { disconnect(); }

NetworkClient& NetworkClient::instance() {
    static NetworkClient inst;
    return inst;
}

bool NetworkClient::connectToServer(const QString &host, quint16 port) {
    if (m_socket->state() == QAbstractSocket::ConnectedState)
        return true;

    m_socket->connectToHost(host, port);
    if (!m_socket->waitForConnected(5000)) {
        qWarning() << "Connection failed:" << m_socket->errorString();
        return false;
    }
    qDebug() << "Connected to server" << host << ":" << port;
    return true;
}

void NetworkClient::disconnect() {
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
    clearSession();
}

bool NetworkClient::isConnected() const {
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

QJsonObject NetworkClient::sendRequest(const QString &action, const QJsonObject &params) {
    if (!isConnected()) {
        return Protocol::parse(Protocol::makeError("Нет соединения с сервером."));
    }

    QByteArray request = Protocol::makeRequest(action, params);
    m_socket->write(request);
    m_socket->flush();

    // Wait for response (synchronous)
    QByteArray buffer;
    while (!buffer.contains('\n')) {
        if (!m_socket->waitForReadyRead(10000)) {
            return Protocol::parse(Protocol::makeError("Таймаут ожидания ответа от сервера."));
        }
        buffer.append(m_socket->readAll());
    }

    int idx = buffer.indexOf('\n');
    QByteArray responseLine = buffer.left(idx);
    return Protocol::parse(responseLine);
}

void NetworkClient::setSession(const QJsonObject &loginData) {
    m_loggedIn = true;
    m_currentUser = loginData["user"].toObject();
    m_isAdmin = loginData["role"].toObject()["role_name"].toString() == "admin";
    if (loginData.contains("customer"))
        m_currentCustomer = loginData["customer"].toObject();
}

void NetworkClient::clearSession() {
    m_loggedIn = false;
    m_isAdmin = false;
    m_currentUser = {};
    m_currentCustomer = {};
}
