#ifndef USER_H
#define USER_H

#include <QString>
#include <QJsonObject>
#ifdef QT_SQL_LIB
#include <QSqlRecord>
#endif

class User
{
public:
    User() : m_userId(0), m_roleId(0) {}
    User(int userId, int roleId, const QString &email, const QString &passwordHash)
        : m_userId(userId), m_roleId(roleId), m_email(email), m_passwordHash(passwordHash) {}

#ifdef QT_SQL_LIB
    static User fromRecord(const QSqlRecord &record) {
        User u;
        u.m_userId = record.value("user_id").toInt();
        u.m_roleId = record.value("role_id").toInt();
        u.m_email = record.value("email").toString();
        u.m_passwordHash = record.value("password_hash").toString();
        return u;
    }
#endif

    static User fromJson(const QJsonObject &obj) {
        User u;
        u.m_userId = obj["user_id"].toInt();
        u.m_roleId = obj["role_id"].toInt();
        u.m_email = obj["email"].toString();
        u.m_passwordHash = obj["password_hash"].toString();
        return u;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["user_id"] = m_userId;
        obj["role_id"] = m_roleId;
        obj["email"] = m_email;
        // password_hash intentionally omitted for client
        return obj;
    }

    bool isValid() const { return m_userId > 0; }

    int userId() const { return m_userId; }
    int roleId() const { return m_roleId; }
    QString email() const { return m_email; }
    QString passwordHash() const { return m_passwordHash; }

    void setUserId(int id) { m_userId = id; }
    void setRoleId(int id) { m_roleId = id; }
    void setEmail(const QString &email) { m_email = email; }
    void setPasswordHash(const QString &hash) { m_passwordHash = hash; }

private:
    int m_userId;
    int m_roleId;
    QString m_email;
    QString m_passwordHash;
};

#endif // USER_H
