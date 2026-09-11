#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <QString>

namespace Protocol {

// Default server settings
constexpr int DEFAULT_PORT = 12345;

// Build a request JSON
inline QByteArray makeRequest(const QString &action, const QJsonObject &params = {})
{
    QJsonObject obj;
    obj["action"] = action;
    obj["params"] = params;
    return QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n";
}

// Build a success response
inline QByteArray makeSuccess(const QJsonValue &data = QJsonValue())
{
    QJsonObject obj;
    obj["success"] = true;
    if (!data.isNull() && !data.isUndefined())
        obj["data"] = data;
    return QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n";
}

// Build an error response
inline QByteArray makeError(const QString &message)
{
    QJsonObject obj;
    obj["success"] = false;
    obj["error"] = message;
    return QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n";
}

// Parse a JSON message from raw bytes
inline QJsonObject parse(const QByteArray &data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return {};
    return doc.object();
}

} // namespace Protocol

#endif // PROTOCOL_H
