#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QDebug>

class Server : public QTcpServer
{
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);
    bool startServer(quint16 port);

protected:
    void incomingConnection(qintptr socketDescriptor) override;
};

#endif // SERVER_H
