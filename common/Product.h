#ifndef PRODUCT_H
#define PRODUCT_H

#include <QString>
#include <QJsonObject>
#ifdef QT_SQL_LIB
#include <QSqlRecord>
#endif

class Product
{
public:
    Product() : m_productId(0), m_price(0.0), m_isDeliveryAvailable(true) {}
    Product(int productId, const QString &name, double price,
            const QString &refInfo, bool isDeliveryAvailable)
        : m_productId(productId), m_productName(name), m_price(price),
          m_referenceInfo(refInfo), m_isDeliveryAvailable(isDeliveryAvailable) {}

#ifdef QT_SQL_LIB
    static Product fromRecord(const QSqlRecord &record) {
        Product p;
        p.m_productId = record.value("product_id").toInt();
        p.m_productName = record.value("product_name").toString();
        p.m_price = record.value("price").toDouble();
        p.m_referenceInfo = record.value("reference_info").toString();
        p.m_isDeliveryAvailable = record.value("is_delivery_available").toBool();
        return p;
    }
#endif

    static Product fromJson(const QJsonObject &obj) {
        Product p;
        p.m_productId = obj["product_id"].toInt();
        p.m_productName = obj["product_name"].toString();
        p.m_price = obj["price"].toDouble();
        p.m_referenceInfo = obj["reference_info"].toString();
        p.m_isDeliveryAvailable = obj["is_delivery_available"].toBool();
        return p;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["product_id"] = m_productId;
        obj["product_name"] = m_productName;
        obj["price"] = m_price;
        obj["reference_info"] = m_referenceInfo;
        obj["is_delivery_available"] = m_isDeliveryAvailable;
        return obj;
    }

    bool isValid() const { return m_productId > 0; }

    int productId() const { return m_productId; }
    QString productName() const { return m_productName; }
    double price() const { return m_price; }
    QString referenceInfo() const { return m_referenceInfo; }
    bool isDeliveryAvailable() const { return m_isDeliveryAvailable; }

    void setProductId(int id) { m_productId = id; }
    void setProductName(const QString &name) { m_productName = name; }
    void setPrice(double price) { m_price = price; }
    void setReferenceInfo(const QString &info) { m_referenceInfo = info; }
    void setIsDeliveryAvailable(bool available) { m_isDeliveryAvailable = available; }

private:
    int m_productId;
    QString m_productName;
    double m_price;
    QString m_referenceInfo;
    bool m_isDeliveryAvailable;
};

#endif // PRODUCT_H
