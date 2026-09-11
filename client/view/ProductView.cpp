#include "ProductView.h"
#include "ProductEditDialog.h"
#include "NetworkClient.h"
#include "Product.h"

ProductView::ProductView(bool isAdmin, QWidget *parent) : QWidget(parent), m_isAdmin(isAdmin) {
    auto *layout = new QVBoxLayout(this);
    auto *toolbar = new QHBoxLayout;
    m_addBtn = new QPushButton("Добавить"); m_editBtn = new QPushButton("Редактировать");
    m_deleteBtn = new QPushButton("Удалить");
    m_addBtn->setVisible(m_isAdmin); m_editBtn->setVisible(m_isAdmin); m_deleteBtn->setVisible(m_isAdmin);
    connect(m_addBtn, &QPushButton::clicked, this, &ProductView::onAdd);
    connect(m_editBtn, &QPushButton::clicked, this, &ProductView::onEdit);
    connect(m_deleteBtn, &QPushButton::clicked, this, &ProductView::onDelete);
    toolbar->addWidget(m_addBtn); toolbar->addWidget(m_editBtn); toolbar->addWidget(m_deleteBtn);
    toolbar->addStretch();
    auto *refreshBtn = new QPushButton("Обновить");
    connect(refreshBtn, &QPushButton::clicked, this, &ProductView::refreshData);
    toolbar->addWidget(refreshBtn);
    layout->addLayout(toolbar);

    m_table = new QTableWidget;
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({"ID", "Название", "Цена", "Справочная информация", "Доставка"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setColumnWidth(0, 50); m_table->setColumnWidth(1, 200);
    m_table->setColumnWidth(2, 100); m_table->setColumnWidth(3, 250);
    layout->addWidget(m_table);
    refreshData();
}

void ProductView::refreshData() {
    auto resp = NetworkClient::instance().sendRequest("get_products");
    if (!resp["success"].toBool()) return;
    QJsonArray arr = resp["data"].toArray();
    m_table->setRowCount(arr.size());
    for (int i = 0; i < arr.size(); ++i) {
        Product p = Product::fromJson(arr[i].toObject());
        m_table->setItem(i, 0, new QTableWidgetItem(QString::number(p.productId())));
        m_table->setItem(i, 1, new QTableWidgetItem(p.productName()));
        m_table->setItem(i, 2, new QTableWidgetItem(QString::number(p.price(), 'f', 2)));
        m_table->setItem(i, 3, new QTableWidgetItem(p.referenceInfo()));
        m_table->setItem(i, 4, new QTableWidgetItem(p.isDeliveryAvailable() ? "Да" : "Нет"));
    }
}

void ProductView::onAdd() {
    ProductEditDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        QJsonObject params;
        params["product_name"] = dlg.productName();
        params["price"] = dlg.price();
        params["reference_info"] = dlg.referenceInfo();
        params["is_delivery_available"] = dlg.isDeliveryAvailable();
        auto resp = NetworkClient::instance().sendRequest("create_product", params);
        if (!resp["success"].toBool()) QMessageBox::warning(this, "Ошибка", resp["error"].toString());
        refreshData();
    }
}

void ProductView::onEdit() {
    int row = m_table->currentRow();
    if (row < 0) { QMessageBox::warning(this, "Ошибка", "Выберите товар."); return; }
    Product p;
    p.setProductId(m_table->item(row, 0)->text().toInt());
    p.setProductName(m_table->item(row, 1)->text());
    p.setPrice(m_table->item(row, 2)->text().toDouble());
    p.setReferenceInfo(m_table->item(row, 3)->text());
    p.setIsDeliveryAvailable(m_table->item(row, 4)->text() == "Да");

    ProductEditDialog dlg(this);
    dlg.setProduct(p);
    if (dlg.exec() == QDialog::Accepted) {
        QJsonObject params;
        params["product_id"] = p.productId();
        params["product_name"] = dlg.productName();
        params["price"] = dlg.price();
        params["reference_info"] = dlg.referenceInfo();
        params["is_delivery_available"] = dlg.isDeliveryAvailable();
        auto resp = NetworkClient::instance().sendRequest("update_product", params);
        if (!resp["success"].toBool()) QMessageBox::warning(this, "Ошибка", resp["error"].toString());
        refreshData();
    }
}

void ProductView::onDelete() {
    int row = m_table->currentRow();
    if (row < 0) { QMessageBox::warning(this, "Ошибка", "Выберите товар."); return; }
    int id = m_table->item(row, 0)->text().toInt();
    if (QMessageBox::question(this, "Подтверждение",
            QString("Удалить товар \"%1\"?").arg(m_table->item(row, 1)->text())) == QMessageBox::Yes) {
        auto resp = NetworkClient::instance().sendRequest("delete_product", {{"product_id", id}});
        if (!resp["success"].toBool()) QMessageBox::warning(this, "Ошибка", resp["error"].toString());
        refreshData();
    }
}
