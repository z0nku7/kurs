#include "OrderView.h"
#include "NetworkClient.h"
#include "Order.h"
#include "OrderItem.h"
#include "Product.h"
#include "DeliveryMethod.h"
#include "Customer.h"

OrderView::OrderView(bool isAdmin, int customerId, QWidget *parent)
    : QWidget(parent), m_isAdmin(isAdmin), m_customerId(customerId) {
    auto *layout = new QVBoxLayout(this);

    auto *toolbar = new QHBoxLayout;
    auto *label = new QLabel("<b>Заказы</b>");
    toolbar->addWidget(label); toolbar->addStretch();
    m_addOrderBtn = new QPushButton("Новый заказ");
    m_deleteOrderBtn = new QPushButton("Удалить заказ");
    m_deleteOrderBtn->setVisible(m_isAdmin);
    connect(m_addOrderBtn, &QPushButton::clicked, this, &OrderView::onAddOrder);
    connect(m_deleteOrderBtn, &QPushButton::clicked, this, &OrderView::onDeleteOrder);
    toolbar->addWidget(m_addOrderBtn); toolbar->addWidget(m_deleteOrderBtn);
    auto *refreshBtn = new QPushButton("Обновить");
    connect(refreshBtn, &QPushButton::clicked, this, &OrderView::refreshData);
    toolbar->addWidget(refreshBtn);
    layout->addLayout(toolbar);

    auto *splitter = new QSplitter(Qt::Vertical);

    m_orderTable = new QTableWidget;
    m_orderTable->setColumnCount(3);
    m_orderTable->setHorizontalHeaderLabels({"ID заказа", "ID заказчика", "Дата покупки"});
    m_orderTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_orderTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_orderTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_orderTable->horizontalHeader()->setStretchLastSection(true);
    connect(m_orderTable, &QTableWidget::currentCellChanged, this, &OrderView::onOrderSelected);
    splitter->addWidget(m_orderTable);

    auto *itemsWidget = new QWidget;
    auto *il = new QVBoxLayout(itemsWidget);
    auto *it = new QHBoxLayout;
    it->addWidget(new QLabel("<b>Позиции заказа</b>")); it->addStretch();
    m_addItemBtn = new QPushButton("Добавить позицию");
    m_deleteItemBtn = new QPushButton("Удалить позицию");
    connect(m_addItemBtn, &QPushButton::clicked, this, &OrderView::onAddItem);
    connect(m_deleteItemBtn, &QPushButton::clicked, this, &OrderView::onDeleteItem);
    it->addWidget(m_addItemBtn); it->addWidget(m_deleteItemBtn);
    il->addLayout(it);

    m_itemTable = new QTableWidget;
    m_itemTable->setColumnCount(6);
    m_itemTable->setHorizontalHeaderLabels({"ID", "Товар", "Кол-во", "Способ доставки", "Стоимость доставки", "Итого"});
    m_itemTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_itemTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_itemTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_itemTable->horizontalHeader()->setStretchLastSection(true);
    il->addWidget(m_itemTable);
    splitter->addWidget(itemsWidget);
    layout->addWidget(splitter);
    refreshData();
}

void OrderView::refreshData() {
    NetworkClient &nc = NetworkClient::instance();
    QString action = m_isAdmin ? "get_orders" : "get_my_orders";
    auto resp = nc.sendRequest(action);
    if (!resp["success"].toBool()) return;
    QJsonArray arr = resp["data"].toArray();
    m_orderTable->setRowCount(arr.size());
    for (int i = 0; i < arr.size(); ++i) {
        Order o = Order::fromJson(arr[i].toObject());
        m_orderTable->setItem(i, 0, new QTableWidgetItem(QString::number(o.orderId())));
        m_orderTable->setItem(i, 1, new QTableWidgetItem(QString::number(o.customerId())));
        m_orderTable->setItem(i, 2, new QTableWidgetItem(o.purchaseDate().toString("dd.MM.yyyy")));
    }
    m_itemTable->setRowCount(0);
}

void OrderView::onOrderSelected() {
    int row = m_orderTable->currentRow();
    if (row < 0) { m_itemTable->setRowCount(0); return; }
    refreshOrderItems(m_orderTable->item(row, 0)->text().toInt());
}

void OrderView::refreshOrderItems(int orderId) {
    NetworkClient &nc = NetworkClient::instance();
    auto resp = nc.sendRequest("get_order_items", {{"order_id", orderId}});
    if (!resp["success"].toBool()) return;

    // Fetch products and methods for display
    auto prodResp = nc.sendRequest("get_products");
    auto methResp = nc.sendRequest("get_delivery_methods");
    QMap<int, Product> products;
    QMap<int, DeliveryMethod> methods;
    if (prodResp["success"].toBool())
        for (const auto &v : prodResp["data"].toArray()) {
            Product p = Product::fromJson(v.toObject()); products[p.productId()] = p;
        }
    if (methResp["success"].toBool())
        for (const auto &v : methResp["data"].toArray()) {
            DeliveryMethod dm = DeliveryMethod::fromJson(v.toObject()); methods[dm.deliveryMethodId()] = dm;
        }

    QJsonArray arr = resp["data"].toArray();
    m_itemTable->setRowCount(arr.size());
    for (int i = 0; i < arr.size(); ++i) {
        OrderItem item = OrderItem::fromJson(arr[i].toObject());
        Product p = products.value(item.productId());
        DeliveryMethod dm = methods.value(item.deliveryMethodId());
        double total = p.price() * item.quantity() + item.actualDeliveryCost();

        m_itemTable->setItem(i, 0, new QTableWidgetItem(QString::number(item.orderItemId())));
        m_itemTable->setItem(i, 1, new QTableWidgetItem(p.productName()));
        m_itemTable->setItem(i, 2, new QTableWidgetItem(QString::number(item.quantity())));
        m_itemTable->setItem(i, 3, new QTableWidgetItem(dm.methodName()));
        m_itemTable->setItem(i, 4, new QTableWidgetItem(QString::number(item.actualDeliveryCost(), 'f', 2)));
        m_itemTable->setItem(i, 5, new QTableWidgetItem(QString::number(total, 'f', 2)));
    }
}

void OrderView::onAddOrder() {
    QDialog dlg(this);
    dlg.setWindowTitle("Новый заказ"); dlg.setMinimumWidth(350);
    auto *layout = new QFormLayout(&dlg);
    auto *dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true); dateEdit->setDisplayFormat("dd.MM.yyyy");
    layout->addRow("Дата покупки:", dateEdit);

    int custId = m_customerId;
    QComboBox *custCombo = nullptr;
    if (m_isAdmin) {
        custCombo = new QComboBox;
        auto resp = NetworkClient::instance().sendRequest("get_customers");
        if (resp["success"].toBool())
            for (const auto &v : resp["data"].toArray()) {
                Customer c = Customer::fromJson(v.toObject());
                custCombo->addItem(c.organizationName(), c.customerId());
            }
        layout->addRow("Заказчик:", custCombo);
    }

    auto *b = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(b, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(b, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addRow(b);

    if (dlg.exec() == QDialog::Accepted) {
        QJsonObject params;
        params["customer_id"] = m_isAdmin ? custCombo->currentData().toInt() : custId;
        params["purchase_date"] = dateEdit->date().toString("yyyy-MM-dd");
        auto resp = NetworkClient::instance().sendRequest("create_order", params);
        if (!resp["success"].toBool()) QMessageBox::warning(this, "Ошибка", resp["error"].toString());
        refreshData();
    }
}

void OrderView::onDeleteOrder() {
    int row = m_orderTable->currentRow();
    if (row < 0) { QMessageBox::warning(this, "Ошибка", "Выберите заказ."); return; }
    int id = m_orderTable->item(row, 0)->text().toInt();
    if (QMessageBox::question(this, "Подтверждение",
            QString("Удалить заказ #%1?").arg(id)) == QMessageBox::Yes) {
        auto resp = NetworkClient::instance().sendRequest("delete_order", {{"order_id", id}});
        if (!resp["success"].toBool()) QMessageBox::warning(this, "Ошибка", resp["error"].toString());
        refreshData();
    }
}

void OrderView::onAddItem() {
    int row = m_orderTable->currentRow();
    if (row < 0) { QMessageBox::warning(this, "Ошибка", "Сначала выберите заказ."); return; }
    int orderId = m_orderTable->item(row, 0)->text().toInt();

    QDialog dlg(this);
    dlg.setWindowTitle("Добавить позицию"); dlg.setMinimumWidth(400);
    auto *layout = new QFormLayout(&dlg);

    auto *productCombo = new QComboBox;
    auto *methodCombo = new QComboBox;
    NetworkClient &nc = NetworkClient::instance();

    auto prodResp = nc.sendRequest("get_products");
    if (prodResp["success"].toBool())
        for (const auto &v : prodResp["data"].toArray()) {
            Product p = Product::fromJson(v.toObject());
            productCombo->addItem(QString("%1 (%2 ₽)").arg(p.productName()).arg(p.price(), 0, 'f', 2), p.productId());
        }
    layout->addRow("Товар:", productCombo);

    auto methResp = nc.sendRequest("get_delivery_methods");
    if (methResp["success"].toBool())
        for (const auto &v : methResp["data"].toArray()) {
            DeliveryMethod dm = DeliveryMethod::fromJson(v.toObject());
            methodCombo->addItem(QString("%1 (%2)").arg(dm.methodName()).arg(dm.speed()), dm.deliveryMethodId());
        }
    layout->addRow("Способ доставки:", methodCombo);

    auto *qtySpin = new QSpinBox; qtySpin->setRange(1, 999999); qtySpin->setValue(1);
    layout->addRow("Количество:", qtySpin);
    auto *costSpin = new QDoubleSpinBox; costSpin->setRange(0, 999999999.99);
    costSpin->setDecimals(2); costSpin->setSuffix(" ₽");
    layout->addRow("Стоимость доставки:", costSpin);

    // Auto-fill delivery cost from tariffs
    auto updateCost = [&]() {
        int pid = productCombo->currentData().toInt();
        auto tariffResp = nc.sendRequest("get_delivery_tariffs", {{"product_id", pid}});
        if (tariffResp["success"].toBool()) {
            int mid = methodCombo->currentData().toInt();
            for (const auto &v : tariffResp["data"].toArray()) {
                QJsonObject t = v.toObject();
                if (t["delivery_method_id"].toInt() == mid) {
                    costSpin->setValue(t["base_delivery_price"].toDouble());
                    return;
                }
            }
        }
    };
    connect(productCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), &dlg, updateCost);
    connect(methodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), &dlg, updateCost);
    if (productCombo->count() > 0 && methodCombo->count() > 0) updateCost();

    auto *b = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(b, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(b, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addRow(b);

    if (dlg.exec() == QDialog::Accepted) {
        QJsonObject params;
        params["order_id"] = orderId;
        params["product_id"] = productCombo->currentData().toInt();
        params["delivery_method_id"] = methodCombo->currentData().toInt();
        params["actual_delivery_cost"] = costSpin->value();
        params["quantity"] = qtySpin->value();
        auto resp = nc.sendRequest("create_order_item", params);
        if (!resp["success"].toBool()) QMessageBox::warning(this, "Ошибка", resp["error"].toString());
        refreshOrderItems(orderId);
    }
}

void OrderView::onDeleteItem() {
    int row = m_itemTable->currentRow();
    if (row < 0) { QMessageBox::warning(this, "Ошибка", "Выберите позицию."); return; }
    int itemId = m_itemTable->item(row, 0)->text().toInt();
    if (QMessageBox::question(this, "Подтверждение", "Удалить позицию?") == QMessageBox::Yes) {
        auto resp = NetworkClient::instance().sendRequest("delete_order_item", {{"order_item_id", itemId}});
        if (!resp["success"].toBool()) QMessageBox::warning(this, "Ошибка", resp["error"].toString());
        int orderRow = m_orderTable->currentRow();
        if (orderRow >= 0) refreshOrderItems(m_orderTable->item(orderRow, 0)->text().toInt());
    }
}
