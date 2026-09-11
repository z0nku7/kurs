#ifndef DELIVERYMETHOD_H
#define DELIVERYMETHOD_H

#include <QString>
#include <QJsonObject>
#ifdef QT_SQL_LIB
#include <QSqlRecord>
#endif

class DeliveryMethod
{
public:
    DeliveryMethod() : m_deliveryMethodId(0) {}
    DeliveryMethod(int id, const QString &name, const QString &speed)
        : m_deliveryMethodId(id), m_methodName(name), m_speed(speed) {}

#ifdef QT_SQL_LIB
    static DeliveryMethod fromRecord(const QSqlRecord &record) {
        DeliveryMethod dm;
        dm.m_deliveryMethodId = record.value("delivery_method_id").toInt();
        dm.m_methodName = record.value("method_name").toString();
        dm.m_speed = record.value("speed").toString();
        return dm;
    }
#endif

    static DeliveryMethod fromJson(const QJsonObject &obj) {
        DeliveryMethod dm;
        dm.m_deliveryMethodId = obj["delivery_method_id"].toInt();
        dm.m_methodName = obj["method_name"].toString();
        dm.m_speed = obj["speed"].toString();
        return dm;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["delivery_method_id"] = m_deliveryMethodId;
        obj["method_name"] = m_methodName;
        obj["speed"] = m_speed;
        return obj;
    }

    bool isValid() const { return m_deliveryMethodId > 0; }

    int deliveryMethodId() const { return m_deliveryMethodId; }
    QString methodName() const { return m_methodName; }
    QString speed() const { return m_speed; }

    void setDeliveryMethodId(int id) { m_deliveryMethodId = id; }
    void setMethodName(const QString &name) { m_methodName = name; }
    void setSpeed(const QString &speed) { m_speed = speed; }

private:
    int m_deliveryMethodId;
    QString m_methodName;
    QString m_speed;
};

#endif // DELIVERYMETHOD_H
