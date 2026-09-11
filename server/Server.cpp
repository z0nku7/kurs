#include "Server.h"
#include "ClientHandler.h"

Server::Server(QObject *parent) : QTcpServer(parent) {}

bool Server::startServer(quint16 port)
{
    if (!listen(QHostAddress::Any, port)) {
        qCritical() << "Server failed to start on port" << port << ":" << errorString();
        return false;
    }
    qDebug() << "Server listening on port" << port;
    return true;
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "New client connection:" << socketDescriptor;
    auto *handler = new ClientHandler(socketDescriptor, this);
    Q_UNUSED(handler);
}
