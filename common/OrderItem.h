#ifndef ORDERITEM_H
#define ORDERITEM_H

#include <QString>
#include <QJsonObject>
#ifdef QT_SQL_LIB
#include <QSqlRecord>
#endif

class OrderItem
{
public:
    OrderItem() : m_orderItemId(0), m_orderId(0), m_productId(0),
                  m_deliveryMethodId(0), m_actualDeliveryCost(0.0), m_quantity(0) {}
    OrderItem(int itemId, int orderId, int productId, int deliveryMethodId,
              double actualDeliveryCost, int quantity)
        : m_orderItemId(itemId), m_orderId(orderId), m_productId(productId),
          m_deliveryMethodId(deliveryMethodId), m_actualDeliveryCost(actualDeliveryCost),
          m_quantity(quantity) {}

#ifdef QT_SQL_LIB
    static OrderItem fromRecord(const QSqlRecord &record) {
        OrderItem oi;
        oi.m_orderItemId = record.value("order_item_id").toInt();
        oi.m_orderId = record.value("order_id").toInt();
        oi.m_productId = record.value("product_id").toInt();
        oi.m_deliveryMethodId = record.value("delivery_method_id").toInt();
        oi.m_actualDeliveryCost = record.value("actual_delivery_cost").toDouble();
        oi.m_quantity = record.value("quantity").toInt();
        return oi;
    }
#endif

    static OrderItem fromJson(const QJsonObject &obj) {
        OrderItem oi;
        oi.m_orderItemId = obj["order_item_id"].toInt();
        oi.m_orderId = obj["order_id"].toInt();
        oi.m_productId = obj["product_id"].toInt();
        oi.m_deliveryMethodId = obj["delivery_method_id"].toInt();
        oi.m_actualDeliveryCost = obj["actual_delivery_cost"].toDouble();
        oi.m_quantity = obj["quantity"].toInt();
        return oi;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["order_item_id"] = m_orderItemId;
        obj["order_id"] = m_orderId;
        obj["product_id"] = m_productId;
        obj["delivery_method_id"] = m_deliveryMethodId;
        obj["actual_delivery_cost"] = m_actualDeliveryCost;
        obj["quantity"] = m_quantity;
        return obj;
    }

    bool isValid() const { return m_orderItemId > 0; }

    int orderItemId() const { return m_orderItemId; }
    int orderId() const { return m_orderId; }
    int productId() const { return m_productId; }
    int deliveryMethodId() const { return m_deliveryMethodId; }
    double actualDeliveryCost() const { return m_actualDeliveryCost; }
    int quantity() const { return m_quantity; }

    void setOrderItemId(int id) { m_orderItemId = id; }
    void setOrderId(int id) { m_orderId = id; }
    void setProductId(int id) { m_productId = id; }
    void setDeliveryMethodId(int id) { m_deliveryMethodId = id; }
    void setActualDeliveryCost(double cost) { m_actualDeliveryCost = cost; }
    void setQuantity(int qty) { m_quantity = qty; }

private:
    int m_orderItemId;
    int m_orderId;
    int m_productId;
    int m_deliveryMethodId;
    double m_actualDeliveryCost;
    int m_quantity;
};

#endif // ORDERITEM_H
