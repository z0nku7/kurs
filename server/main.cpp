#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include "Server.h"
#include "DatabaseManager.h"
#include "Protocol.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "========================================";
    qDebug() << "           OrderServer v1.0             ";
    qDebug() << "========================================";

    quint16 tcpPort = Protocol::DEFAULT_PORT;
    bool forceSqlite = false;
    QString sqlitePath = "orders.db";

    QString pgHost = "localhost";
    int pgPort = 5432;
    QString pgDbName = "orders_db";
    QString pgUser = "postgres";
    QString pgPassword = "postgres";

    for (int i = 1; i < argc; ++i) {
        QString arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            tcpPort = QString(argv[++i]).toUShort();
        } else if (arg == "--sqlite") {
            forceSqlite = true;
            if (i + 1 < argc && !argv[i + 1][0] == '-') {
                sqlitePath = argv[++i];
            }
        } else if (arg == "--db-host" && i + 1 < argc) {
            pgHost = argv[++i];
        } else if (arg == "--db-port" && i + 1 < argc) {
            pgPort = QString(argv[++i]).toInt();
        } else if (arg == "--db-name" && i + 1 < argc) {
            pgDbName = argv[++i];
        } else if (arg == "--db-user" && i + 1 < argc) {
            pgUser = argv[++i];
        } else if (arg == "--db-pass" && i + 1 < argc) {
            pgPassword = argv[++i];
        }
    }

    DatabaseManager &db = DatabaseManager::instance();
    bool isSqlite = forceSqlite;

    if (!forceSqlite) {
        qDebug() << "Attempting connection to PostgreSQL database" << pgDbName << "at" << pgHost << ":" << pgPort << "...";
        if (!db.connectToDatabase(pgHost, pgPort, pgDbName, pgUser, pgPassword)) {
            qWarning() << "PostgreSQL connection failed:" << db.lastError();
            qWarning() << "Falling back to embedded SQLite database (" << sqlitePath << ")...";
            isSqlite = true;
        }
    }

    if (isSqlite) {
        if (!db.connectToSqlite(sqlitePath)) {
            qCritical() << "Failed to connect to SQLite database:" << db.lastError();
            return 1;
        }
    }

    // Resolve SQL init script
    QString appDir = QCoreApplication::applicationDirPath();
    QString sqlFileName = isSqlite ? "init_sqlite.sql" : "init.sql";
    QStringList searchPaths = {
        appDir + "/../../sql/" + sqlFileName,
        appDir + "/../sql/" + sqlFileName,
        "sql/" + sqlFileName,
        appDir + "/" + sqlFileName
    };

    QString foundSqlPath;
    for (const QString &p : searchPaths) {
        if (QFile::exists(p)) {
            foundSqlPath = QFileInfo(p).absoluteFilePath();
            break;
        }
    }

    if (!foundSqlPath.isEmpty()) {
        qDebug() << "Initializing database schema from:" << foundSqlPath;
        db.initializeSchema(foundSqlPath);
    } else {
        qWarning() << "SQL init script" << sqlFileName << "not found, skipping schema initialization.";
    }

    // Start TCP server
    Server server;
    if (!server.startServer(tcpPort)) {
        qCritical() << "Failed to start TCP server on port" << tcpPort;
        return 1;
    }

    qDebug() << "OrderServer successfully running on port" << tcpPort;
    qDebug() << "Awaiting client connections...";

    return app.exec();
}
