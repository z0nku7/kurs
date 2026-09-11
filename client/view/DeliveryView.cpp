#include "DeliveryView.h"
#include "NetworkClient.h"
#include "DeliveryMethod.h"
#include "DeliveryTariff.h"
#include "Product.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDialog>

DeliveryView::DeliveryView(bool isAdmin, QWidget *parent) : QWidget(parent), m_isAdmin(isAdmin) {
    auto *layout = new QVBoxLayout(this);
    auto *tabs = new QTabWidget;

    // Methods tab
    auto *mw = new QWidget; auto *ml = new QVBoxLayout(mw);
    auto *mt = new QHBoxLayout;
    m_addMethodBtn = new QPushButton("Добавить"); m_editMethodBtn = new QPushButton("Редактировать");
    m_deleteMethodBtn = new QPushButton("Удалить");
    m_addMethodBtn->setVisible(isAdmin); m_editMethodBtn->setVisible(isAdmin); m_deleteMethodBtn->setVisible(isAdmin);
    connect(m_addMethodBtn, &QPushButton::clicked, this, &DeliveryView::onAddMethod);
    connect(m_editMethodBtn, &QPushButton::clicked, this, &DeliveryView::onEditMethod);
    connect(m_deleteMethodBtn, &QPushButton::clicked, this, &DeliveryView::onDeleteMethod);
    mt->addWidget(m_addMethodBtn); mt->addWidget(m_editMethodBtn); mt->addWidget(m_deleteMethodBtn);
    mt->addStretch(); ml->addLayout(mt);
    m_methodTable = new QTableWidget; m_methodTable->setColumnCount(3);
    m_methodTable->setHorizontalHeaderLabels({"ID", "Название", "Скорость"});
    m_methodTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_methodTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_methodTable->horizontalHeader()->setStretchLastSection(true);
    m_methodTable->setColumnWidth(0, 50); m_methodTable->setColumnWidth(1, 250);
    ml->addWidget(m_methodTable); tabs->addTab(mw, "Способы доставки");

    // Tariffs tab
    auto *tw = new QWidget; auto *tl = new QVBoxLayout(tw);
    auto *tt = new QHBoxLayout;
    m_addTariffBtn = new QPushButton("Добавить"); m_editTariffBtn = new QPushButton("Редактировать");
    m_deleteTariffBtn = new QPushButton("Удалить");
    m_addTariffBtn->setVisible(isAdmin); m_editTariffBtn->setVisible(isAdmin); m_deleteTariffBtn->setVisible(isAdmin);
    connect(m_addTariffBtn, &QPushButton::clicked, this, &DeliveryView::onAddTariff);
    connect(m_editTariffBtn, &QPushButton::clicked, this, &DeliveryView::onEditTariff);
    connect(m_deleteTariffBtn, &QPushButton::clicked, this, &DeliveryView::onDeleteTariff);
    tt->addWidget(m_addTariffBtn); tt->addWidget(m_editTariffBtn); tt->addWidget(m_deleteTariffBtn);
    tt->addStretch(); tl->addLayout(tt);
    m_tariffTable = new QTableWidget; m_tariffTable->setColumnCount(4);
    m_tariffTable->setHorizontalHeaderLabels({"ID", "Товар", "Способ доставки", "Базовая цена"});
    m_tariffTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tariffTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tariffTable->horizontalHeader()->setStretchLastSection(true);
    m_tariffTable->setColumnWidth(0, 50); m_tariffTable->setColumnWidth(1, 200); m_tariffTable->setColumnWidth(2, 200);
    tl->addWidget(m_tariffTable); tabs->addTab(tw, "Тарифы доставки");

    layout->addWidget(tabs);
    refreshData();
}

void DeliveryView::refreshData() { refreshMethods(); refreshTariffs(); }

void DeliveryView::refreshMethods() {
    auto resp = NetworkClient::instance().sendRequest("get_delivery_methods");
    if (!resp["success"].toBool()) return;
    QJsonArray arr = resp["data"].toArray();
    m_methodTable->setRowCount(arr.size());
    for (int i = 0; i < arr.size(); ++i) {
        DeliveryMethod dm = DeliveryMethod::fromJson(arr[i].toObject());
        m_methodTable->setItem(i, 0, new QTableWidgetItem(QString::number(dm.deliveryMethodId())));
        m_methodTable->setItem(i, 1, new QTableWidgetItem(dm.methodName()));
        m_methodTable->setItem(i, 2, new QTableWidgetItem(dm.speed()));
    }
}

void DeliveryView::refreshTariffs() {
    auto resp = NetworkClient::instance().sendRequest("get_delivery_tariffs");
    auto prodResp = NetworkClient::instance().sendRequest("get_products");
    auto methResp = NetworkClient::instance().sendRequest("get_delivery_methods");
    QMap<int, QString> productNames, methodNames;
    if (prodResp["success"].toBool()) for (const auto &v : prodResp["data"].toArray()) { auto o = v.toObject(); productNames[o["product_id"].toInt()] = o["product_name"].toString(); }
    if (methResp["success"].toBool()) for (const auto &v : methResp["data"].toArray()) { auto o = v.toObject(); methodNames[o["delivery_method_id"].toInt()] = o["method_name"].toString(); }

    if (!resp["success"].toBool()) return;
    QJsonArray arr = resp["data"].toArray();
    m_tariffTable->setRowCount(arr.size());
    for (int i = 0; i < arr.size(); ++i) {
        DeliveryTariff dt = DeliveryTariff::fromJson(arr[i].toObject());
        m_tariffTable->setItem(i, 0, new QTableWidgetItem(QString::number(dt.deliveryTariffId())));
        m_tariffTable->setItem(i, 1, new QTableWidgetItem(productNames.value(dt.productId())));
        m_tariffTable->setItem(i, 2, new QTableWidgetItem(methodNames.value(dt.deliveryMethodId())));
        m_tariffTable->setItem(i, 3, new QTableWidgetItem(QString::number(dt.baseDeliveryPrice(), 'f', 2)));
    }
}

void DeliveryView::onAddMethod() {
    QDialog dlg(this); dlg.setWindowTitle("Способ доставки"); dlg.setMinimumWidth(350);
    auto *l = new QFormLayout(&dlg);
    auto *nameEdit = new QLineEdit; l->addRow("Название:", nameEdit);
    auto *speedEdit = new QLineEdit; l->addRow("Скорость:", speedEdit);
    auto *b = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(b, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(b, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    l->addRow(b);
    if (dlg.exec() == QDialog::Accepted) {
        auto resp = NetworkClient::instance().sendRequest("create_delivery_method",
            {{"method_name", nameEdit->text().trimmed()}, {"speed", speedEdit->text().trimmed()}});
        if (!resp["success"].toBool()) QMessageBox::warning(this, "Ошибка", resp["error"].toString());
        refreshMethods();
    }
}

void DeliveryView::onEditMethod() {
    int row = m_methodTable->currentRow();
    if (row < 0) { QMessageBox::warning(this, "Ошибка", "Выберите способ доставки."); return; }
    int id = m_methodTable->item(row, 0)->text().toInt();
    QDialog dlg(this); dlg.setWindowTitle("Способ доставки"); dlg.setMinimumWidth(350);
    auto *l = new QFormLayout(&dlg);
    auto *nameEdit = new QLineEdit(m_methodTable->item(row, 1)->text()); l->addRow("Название:", nameEdit);
    auto *speedEdit = new QLineEdit(m_methodTable->item(row, 2)->text()); l->addRow("Скорость:", speedEdit);
    auto *b = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(b, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(b, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    l->addRow(b);
    if (dlg.exec() == QDialog::Accepted) {
        auto resp = NetworkClient::instance().sendRequest("update_delivery_method",
            {{"delivery_method_id", id}, {"method_name", nameEdit->text().trimmed()}, {"speed", speedEdit->text().trimmed()}});
        if (!resp["success"].toBool()) QMessageBox::warning(this, "Ошибка", resp["error"].toString());
        refreshMethods();
    }
}

void DeliveryView::onDeleteMethod() {
    int row = m_methodTable->currentRow();
    if (row < 0) return;
    int id = m_methodTable->item(row, 0)->text().toInt();
    if (QMessageBox::question(this, "Подтверждение", "Удалить способ доставки?") == QMessageBox::Yes) {
        auto resp = NetworkClient::instance().sendRequest("delete_delivery_method", {{"delivery_method_id", id}});
        if (!resp["success"].toBool()) QMessageBox::warning(this, "Ошибка", resp["error"].toString());
        refreshMethods();
    }
}

void DeliveryView::onAddTariff() {
    QDialog dlg(this); dlg.setWindowTitle("Тариф доставки"); dlg.setMinimumWidth(400);
    auto *l = new QFormLayout(&dlg);
    auto *productCombo = new QComboBox; auto *methodCombo = new QComboBox;
    auto prodResp = NetworkClient::instance().sendRequest("get_products");
    if (prodResp["success"].toBool()) for (const auto &v : prodResp["data"].toArray()) { auto o = v.toObject(); productCombo->addItem(o["product_name"].toString(), o["product_id"].toInt()); }
    auto methResp = NetworkClient::instance().sendRequest("get_delivery_methods");
    if (methResp["success"].toBool()) for (const auto &v : methResp["data"].toArray()) { auto o = v.toObject(); methodCombo->addItem(o["method_name"].toString(), o["delivery_method_id"].toInt()); }
    l->addRow("Товар:", productCombo); l->addRow("Способ доставки:", methodCombo);
    auto *priceEdit = new QDoubleSpinBox; priceEdit->setRange(0, 999999999.99); priceEdit->setDecimals(2); priceEdit->setSuffix(" ₽");
    l->addRow("Базовая цена:", priceEdit);
    auto *b = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(b, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(b, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    l->addRow(b);
    if (dlg.exec() == QDialog::Accepted) {
        auto resp = NetworkClient::instance().sendRequest("create_delivery_tariff",
            {{"product_id", productCombo->currentData().toInt()}, {"delivery_method_id", methodCombo->currentData().toInt()},
             {"base_delivery_price", priceEdit->value()}});
        if (!resp["success"].toBool()) QMessageBox::warning(this, "Ошибка", resp["error"].toString());
        refreshTariffs();
    }
}

void DeliveryView::onEditTariff() {
    int row = m_tariffTable->currentRow();
    if (row < 0) { QMessageBox::warning(this, "Ошибка", "Выберите тариф."); return; }
    int id = m_tariffTable->item(row, 0)->text().toInt();
    QDialog dlg(this); dlg.setWindowTitle("Тариф доставки"); dlg.setMinimumWidth(400);
    auto *l = new QFormLayout(&dlg);
    auto *productCombo = new QComboBox; auto *methodCombo = new QComboBox;
    auto prodResp = NetworkClient::instance().sendRequest("get_products");
    if (prodResp["success"].toBool()) for (const auto &v : prodResp["data"].toArray()) { auto o = v.toObject(); productCombo->addItem(o["product_name"].toString(), o["product_id"].toInt()); }
    auto methResp = NetworkClient::instance().sendRequest("get_delivery_methods");
    if (methResp["success"].toBool()) for (const auto &v : methResp["data"].toArray()) { auto o = v.toObject(); methodCombo->addItem(o["method_name"].toString(), o["delivery_method_id"].toInt()); }
    l->addRow("Товар:", productCombo); l->addRow("Способ доставки:", methodCombo);
    auto *priceEdit = new QDoubleSpinBox; priceEdit->setRange(0, 999999999.99); priceEdit->setDecimals(2); priceEdit->setSuffix(" ₽");
    priceEdit->setValue(m_tariffTable->item(row, 3)->text().toDouble());
    l->addRow("Базовая цена:", priceEdit);
    auto *b = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(b, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(b, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    l->addRow(b);
    if (dlg.exec() == QDialog::Accepted) {
        auto resp = NetworkClient::instance().sendRequest("update_delivery_tariff",
            {{"delivery_tariff_id", id}, {"product_id", productCombo->currentData().toInt()},
             {"delivery_method_id", methodCombo->currentData().toInt()}, {"base_delivery_price", priceEdit->value()}});
        if (!resp["success"].toBool()) QMessageBox::warning(this, "Ошибка", resp["error"].toString());
        refreshTariffs();
    }
}

void DeliveryView::onDeleteTariff() {
    int row = m_tariffTable->currentRow();
    if (row < 0) return;
    int id = m_tariffTable->item(row, 0)->text().toInt();
    if (QMessageBox::question(this, "Подтверждение", "Удалить тариф?") == QMessageBox::Yes) {
        auto resp = NetworkClient::instance().sendRequest("delete_delivery_tariff", {{"delivery_tariff_id", id}});
        if (!resp["success"].toBool()) QMessageBox::warning(this, "Ошибка", resp["error"].toString());
        refreshTariffs();
    }
}
