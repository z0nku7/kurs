#ifndef ROLE_H
#define ROLE_H

#include <QString>
#include <QJsonObject>
#ifdef QT_SQL_LIB
#include <QSqlRecord>
#endif

class Role
{
public:
    Role() : m_roleId(0) {}
    Role(int roleId, const QString &roleName)
        : m_roleId(roleId), m_roleName(roleName) {}

#ifdef QT_SQL_LIB
    static Role fromRecord(const QSqlRecord &record) {
        Role r;
        r.m_roleId = record.value("role_id").toInt();
        r.m_roleName = record.value("role_name").toString();
        return r;
    }
#endif

    static Role fromJson(const QJsonObject &obj) {
        Role r;
        r.m_roleId = obj["role_id"].toInt();
        r.m_roleName = obj["role_name"].toString();
        return r;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["role_id"] = m_roleId;
        obj["role_name"] = m_roleName;
        return obj;
    }

    bool isValid() const { return m_roleId > 0; }

    int roleId() const { return m_roleId; }
    QString roleName() const { return m_roleName; }
    void setRoleId(int id) { m_roleId = id; }
    void setRoleName(const QString &name) { m_roleName = name; }

private:
    int m_roleId;
    QString m_roleName;
};

#endif // ROLE_H
