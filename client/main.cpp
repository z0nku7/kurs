#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QStyleFactory>

#include "NetworkClient.h"
#include "LoginDialog.h"
#include "MainWindow.h"
#include "Protocol.h"

// Connection dialog in case default host:port cannot be reached
bool ensureConnection(const QString &defaultHost, int defaultPort)
{
    NetworkClient &net = NetworkClient::instance();
    if (net.connectToServer(defaultHost, defaultPort)) {
        return true;
    }

    // If direct connection fails, ask user for host and port
    QDialog dlg;
    dlg.setWindowTitle("Подключение к серверу");
    dlg.setMinimumWidth(350);

    auto *layout = new QVBoxLayout(&dlg);
    auto *info = new QLabel("Не удалось подключиться к серверу по умолчанию.\nУкажите параметры подключения:", &dlg);
    layout->addWidget(info);

    auto *form = new QFormLayout;
    auto *hostEdit = new QLineEdit(defaultHost, &dlg);
    auto *portSpin = new QSpinBox(&dlg);
    portSpin->setRange(1, 65535);
    portSpin->setValue(defaultPort);

    form->addRow("Хост (IP):", hostEdit);
    form->addRow("Порт:", portSpin);
    layout->addLayout(form);

    auto *btnLayout = new QHBoxLayout;
    auto *connectBtn = new QPushButton("Подключиться", &dlg);
    auto *cancelBtn = new QPushButton("Выход", &dlg);
    btnLayout->addWidget(connectBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    QObject::connect(connectBtn, &QPushButton::clicked, [&]() {
        QString host = hostEdit->text().trimmed();
        int port = portSpin->value();
        if (net.connectToServer(host, port)) {
            dlg.accept();
        } else {
            QMessageBox::critical(&dlg, "Ошибка", "Не удалось установить соединение с " + host + ":" + QString::number(port));
        }
    });

    return dlg.exec() == QDialog::Accepted;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("OrderManagementClient");
    app.setApplicationDisplayName("Система управления заказами");

    // Modern styling
    app.setStyle(QStyleFactory::create("Fusion"));

    QString host = "127.0.0.1";
    quint16 port = Protocol::DEFAULT_PORT;

    for (int i = 1; i < argc; ++i) {
        QString arg = argv[i];
        if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            port = QString(argv[++i]).toUShort();
        }
    }

    if (!ensureConnection(host, port)) {
        return 0;
    }

    // Main event loop with support for re-login on logout
    while (true) {
        LoginDialog loginDlg;
        if (loginDlg.exec() != QDialog::Accepted) {
            break;
        }

        MainWindow mainWindow;
        bool wantRelogin = false;
        QObject::connect(&mainWindow, &MainWindow::logoutRequested, [&]() {
            wantRelogin = true;
        });

        mainWindow.show();
        app.exec();

        if (!wantRelogin) {
            break;
        }

        // Reconnect if socket was disconnected on logout
        if (!NetworkClient::instance().isConnected()) {
            if (!ensureConnection(host, port)) {
                break;
            }
        }
    }

    return 0;
}
