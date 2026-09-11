#ifndef DELIVERYTARIFF_H
#define DELIVERYTARIFF_H

#include <QString>
#include <QJsonObject>
#ifdef QT_SQL_LIB
#include <QSqlRecord>
#endif

class DeliveryTariff
{
public:
    DeliveryTariff() : m_deliveryTariffId(0), m_productId(0),
                        m_deliveryMethodId(0), m_baseDeliveryPrice(0.0) {}
    DeliveryTariff(int tariffId, int productId, int methodId, double basePrice)
        : m_deliveryTariffId(tariffId), m_productId(productId),
          m_deliveryMethodId(methodId), m_baseDeliveryPrice(basePrice) {}

#ifdef QT_SQL_LIB
    static DeliveryTariff fromRecord(const QSqlRecord &record) {
        DeliveryTariff dt;
        dt.m_deliveryTariffId = record.value("delivery_tariff_id").toInt();
        dt.m_productId = record.value("product_id").toInt();
        dt.m_deliveryMethodId = record.value("delivery_method_id").toInt();
        dt.m_baseDeliveryPrice = record.value("base_delivery_price").toDouble();
        return dt;
    }
#endif

    static DeliveryTariff fromJson(const QJsonObject &obj) {
        DeliveryTariff dt;
        dt.m_deliveryTariffId = obj["delivery_tariff_id"].toInt();
        dt.m_productId = obj["product_id"].toInt();
        dt.m_deliveryMethodId = obj["delivery_method_id"].toInt();
        dt.m_baseDeliveryPrice = obj["base_delivery_price"].toDouble();
        return dt;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["delivery_tariff_id"] = m_deliveryTariffId;
        obj["product_id"] = m_productId;
        obj["delivery_method_id"] = m_deliveryMethodId;
        obj["base_delivery_price"] = m_baseDeliveryPrice;
        return obj;
    }

    bool isValid() const { return m_deliveryTariffId > 0; }

    int deliveryTariffId() const { return m_deliveryTariffId; }
    int productId() const { return m_productId; }
    int deliveryMethodId() const { return m_deliveryMethodId; }
    double baseDeliveryPrice() const { return m_baseDeliveryPrice; }

    void setDeliveryTariffId(int id) { m_deliveryTariffId = id; }
    void setProductId(int id) { m_productId = id; }
    void setDeliveryMethodId(int id) { m_deliveryMethodId = id; }
    void setBaseDeliveryPrice(double price) { m_baseDeliveryPrice = price; }

private:
    int m_deliveryTariffId;
    int m_productId;
    int m_deliveryMethodId;
    double m_baseDeliveryPrice;
};

#endif // DELIVERYTARIFF_H
