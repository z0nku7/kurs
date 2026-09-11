#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <QString>
#include <QJsonObject>
#ifdef QT_SQL_LIB
#include <QSqlRecord>
#endif

class Customer
{
public:
    Customer() : m_customerId(0), m_userId(0) {}
    Customer(int customerId, int userId, const QString &orgName,
             const QString &contactPerson, const QString &address, const QString &phone)
        : m_customerId(customerId), m_userId(userId), m_organizationName(orgName),
          m_contactPerson(contactPerson), m_address(address), m_phone(phone) {}

#ifdef QT_SQL_LIB
    static Customer fromRecord(const QSqlRecord &record) {
        Customer c;
        c.m_customerId = record.value("customer_id").toInt();
        c.m_userId = record.value("user_id").toInt();
        c.m_organizationName = record.value("organization_name").toString();
        c.m_contactPerson = record.value("contact_person").toString();
        c.m_address = record.value("address").toString();
        c.m_phone = record.value("phone").toString();
        return c;
    }
#endif

    static Customer fromJson(const QJsonObject &obj) {
        Customer c;
        c.m_customerId = obj["customer_id"].toInt();
        c.m_userId = obj["user_id"].toInt();
        c.m_organizationName = obj["organization_name"].toString();
        c.m_contactPerson = obj["contact_person"].toString();
        c.m_address = obj["address"].toString();
        c.m_phone = obj["phone"].toString();
        return c;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["customer_id"] = m_customerId;
        obj["user_id"] = m_userId;
        obj["organization_name"] = m_organizationName;
        obj["contact_person"] = m_contactPerson;
        obj["address"] = m_address;
        obj["phone"] = m_phone;
        return obj;
    }

    bool isValid() const { return m_customerId > 0; }

    int customerId() const { return m_customerId; }
    int userId() const { return m_userId; }
    QString organizationName() const { return m_organizationName; }
    QString contactPerson() const { return m_contactPerson; }
    QString address() const { return m_address; }
    QString phone() const { return m_phone; }

    void setCustomerId(int id) { m_customerId = id; }
    void setUserId(int id) { m_userId = id; }
    void setOrganizationName(const QString &name) { m_organizationName = name; }
    void setContactPerson(const QString &person) { m_contactPerson = person; }
    void setAddress(const QString &addr) { m_address = addr; }
    void setPhone(const QString &phone) { m_phone = phone; }

private:
    int m_customerId;
    int m_userId;
    QString m_organizationName;
    QString m_contactPerson;
    QString m_address;
    QString m_phone;
};

#endif // CUSTOMER_H
