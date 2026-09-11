#ifndef ORDER_H
#define ORDER_H

#include <QString>
#include <QDate>
#include <QJsonObject>
#ifdef QT_SQL_LIB
#include <QSqlRecord>
#endif

class Order
{
public:
    Order() : m_orderId(0), m_customerId(0) {}
    Order(int orderId, int customerId, const QDate &purchaseDate)
        : m_orderId(orderId), m_customerId(customerId), m_purchaseDate(purchaseDate) {}

#ifdef QT_SQL_LIB
    static Order fromRecord(const QSqlRecord &record) {
        Order o;
        o.m_orderId = record.value("order_id").toInt();
        o.m_customerId = record.value("customer_id").toInt();
        o.m_purchaseDate = record.value("purchase_date").toDate();
        return o;
    }
#endif

    static Order fromJson(const QJsonObject &obj) {
        Order o;
        o.m_orderId = obj["order_id"].toInt();
        o.m_customerId = obj["customer_id"].toInt();
        o.m_purchaseDate = QDate::fromString(obj["purchase_date"].toString(), "yyyy-MM-dd");
        return o;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["order_id"] = m_orderId;
        obj["customer_id"] = m_customerId;
        obj["purchase_date"] = m_purchaseDate.toString("yyyy-MM-dd");
        return obj;
    }

    bool isValid() const { return m_orderId > 0; }

    int orderId() const { return m_orderId; }
    int customerId() const { return m_customerId; }
    QDate purchaseDate() const { return m_purchaseDate; }

    void setOrderId(int id) { m_orderId = id; }
    void setCustomerId(int id) { m_customerId = id; }
    void setPurchaseDate(const QDate &date) { m_purchaseDate = date; }

private:
    int m_orderId;
    int m_customerId;
    QDate m_purchaseDate;
};

#endif // ORDER_H
