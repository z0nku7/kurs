#include "CustomerView.h"
#include "NetworkClient.h"
#include "Customer.h"

CustomerView::CustomerView(bool isAdmin, QWidget *parent) : QWidget(parent), m_isAdmin(isAdmin) {
    auto *layout = new QVBoxLayout(this);
    auto *toolbar = new QHBoxLayout;
    m_deleteBtn = new QPushButton("Удалить");
    m_deleteBtn->setVisible(m_isAdmin);
    connect(m_deleteBtn, &QPushButton::clicked, this, &CustomerView::onDelete);
    toolbar->addWidget(m_deleteBtn); toolbar->addStretch();
    auto *refreshBtn = new QPushButton("Обновить");
    connect(refreshBtn, &QPushButton::clicked, this, &CustomerView::refreshData);
    toolbar->addWidget(refreshBtn);
    layout->addLayout(toolbar);

    m_table = new QTableWidget;
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"ID", "ID пользователя", "Организация", "Контактное лицо", "Адрес", "Телефон"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setColumnWidth(0, 50); m_table->setColumnWidth(1, 100);
    m_table->setColumnWidth(2, 200); m_table->setColumnWidth(3, 150); m_table->setColumnWidth(4, 200);
    layout->addWidget(m_table);
    refreshData();
}

void CustomerView::refreshData() {
    auto resp = NetworkClient::instance().sendRequest("get_customers");
    if (!resp["success"].toBool()) return;
    QJsonArray arr = resp["data"].toArray();
    m_table->setRowCount(arr.size());
    for (int i = 0; i < arr.size(); ++i) {
        Customer c = Customer::fromJson(arr[i].toObject());
        m_table->setItem(i, 0, new QTableWidgetItem(QString::number(c.customerId())));
        m_table->setItem(i, 1, new QTableWidgetItem(QString::number(c.userId())));
        m_table->setItem(i, 2, new QTableWidgetItem(c.organizationName()));
        m_table->setItem(i, 3, new QTableWidgetItem(c.contactPerson()));
        m_table->setItem(i, 4, new QTableWidgetItem(c.address()));
        m_table->setItem(i, 5, new QTableWidgetItem(c.phone()));
    }
}

void CustomerView::onDelete() {
    int row = m_table->currentRow();
    if (row < 0) { QMessageBox::warning(this, "Ошибка", "Выберите заказчика."); return; }
    int id = m_table->item(row, 0)->text().toInt();
    if (QMessageBox::question(this, "Подтверждение",
            QString("Удалить заказчика \"%1\"?").arg(m_table->item(row, 2)->text())) == QMessageBox::Yes) {
        auto resp = NetworkClient::instance().sendRequest("delete_customer", {{"customer_id", id}});
        if (!resp["success"].toBool()) QMessageBox::warning(this, "Ошибка", resp["error"].toString());
        refreshData();
    }
}
