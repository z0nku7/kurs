#include "MainWindow.h"
#include "NetworkClient.h"
#include "ProductView.h"
#include "OrderView.h"
#include "CustomerView.h"
#include "DeliveryView.h"
#include "dialogs/ProfileDialog.h"
#include "Product.h"
#include "Order.h"
#include "OrderItem.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QMap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_isAdmin(NetworkClient::instance().isAdmin())
{
    setupUi();
    setupMenu();
    setupStatusBar();
}

void MainWindow::setupUi()
{
    resize(1100, 700);
    setMinimumSize(900, 600);

    NetworkClient &net = NetworkClient::instance();
    QString roleTitle = m_isAdmin ? "Администратор" : "Заказчик";
    QString email = net.currentUser()["email"].toString();
    setWindowTitle(QString("Система управления заказами — %1 (%2)").arg(email, roleTitle));

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setDocumentMode(true);

    // 1. Products Tab
    m_productView = new ProductView(m_isAdmin, this);
    m_tabWidget->addTab(m_productView, "Товары");

    // 2. Orders Tab
    int customerId = m_isAdmin ? -1 : net.currentCustomerId();
    m_orderView = new OrderView(m_isAdmin, customerId, this);
    m_tabWidget->addTab(m_orderView, "Заказы");

    // 3. Delivery Tab
    m_deliveryView = new DeliveryView(m_isAdmin, this);
    m_tabWidget->addTab(m_deliveryView, "Доставка и тарифы");

    // 4. Customers Tab (Admin only)
    if (m_isAdmin) {
        m_customerView = new CustomerView(m_isAdmin, this);
        m_tabWidget->addTab(m_customerView, "Заказчики");
    } else {
        m_customerView = nullptr;
    }

    // 5. Statistics Tab
    m_statsWidget = createStatsWidget();
    m_tabWidget->addTab(m_statsWidget, "Финансовая статистика");

    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    setCentralWidget(m_tabWidget);
}

void MainWindow::setupMenu()
{
    auto *fileMenu = menuBar()->addMenu("Файл");

    auto *logoutAct = fileMenu->addAction("Сменить пользователя...");
    logoutAct->setShortcut(QKeySequence("Ctrl+L"));
    connect(logoutAct, &QAction::triggered, this, &MainWindow::onLogout);

    fileMenu->addSeparator();

    auto *exitAct = fileMenu->addAction("Выход");
    exitAct->setShortcut(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    auto *profileMenu = menuBar()->addMenu("Профиль");
    auto *editProfileAct = profileMenu->addAction("Редактировать профиль...");
    editProfileAct->setShortcut(QKeySequence("Ctrl+P"));
    connect(editProfileAct, &QAction::triggered, this, &MainWindow::onEditProfile);

    auto *helpMenu = menuBar()->addMenu("Справка");
    auto *aboutAct = helpMenu->addAction("О программе");
    connect(aboutAct, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupStatusBar()
{
    NetworkClient &net = NetworkClient::instance();
    QString userStr = net.currentUser()["email"].toString();
    QString roleStr = m_isAdmin ? "Администратор" : "Заказчик";

    if (!m_isAdmin && !net.currentCustomer().isEmpty()) {
        QString org = net.currentCustomer()["organization_name"].toString();
        if (!org.isEmpty()) {
            userStr = QString("%1 (%2)").arg(org, userStr);
        }
    }

    m_statusLabel = new QLabel(QString("Подключен к серверу | %1: %2").arg(roleStr, userStr), this);
    statusBar()->addWidget(m_statusLabel);
}

QWidget* MainWindow::createStatsWidget()
{
    auto *widget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(widget);

    auto *topToolbar = new QHBoxLayout;
    auto *refreshBtn = new QPushButton("Обновить отчет", widget);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshStatistics);
    topToolbar->addWidget(refreshBtn);
    topToolbar->addStretch();
    mainLayout->addLayout(topToolbar);

    // Metrics summary group
    auto *metricsGroup = new QGroupBox("Финансовые показатели", widget);
    auto *form = new QFormLayout(metricsGroup);

    QFont metricFont;
    metricFont.setBold(true);
    metricFont.setPointSize(11);

    m_totalOrdersLabel = new QLabel("0", widget);
    m_totalOrdersLabel->setFont(metricFont);
    form->addRow("Всего оформлено заказов:", m_totalOrdersLabel);

    m_totalItemsSoldLabel = new QLabel("0 шт.", widget);
    m_totalItemsSoldLabel->setFont(metricFont);
    form->addRow("Всего продано товарных единиц:", m_totalItemsSoldLabel);

    m_totalRevenueLabel = new QLabel("0.00 руб.", widget);
    m_totalRevenueLabel->setFont(metricFont);
    form->addRow("Общая стоимость товаров:", m_totalRevenueLabel);

    m_totalDeliveryCostLabel = new QLabel("0.00 руб.", widget);
    m_totalDeliveryCostLabel->setFont(metricFont);
    form->addRow("Общие затраты на доставку:", m_totalDeliveryCostLabel);

    m_averageOrderValueLabel = new QLabel("0.00 руб.", widget);
    m_averageOrderValueLabel->setFont(metricFont);
    form->addRow("Средний чек заказа (с доставкой):", m_averageOrderValueLabel);

    mainLayout->addWidget(metricsGroup);

    // Products sales breakdown
    auto *tableGroup = new QGroupBox("Продажи по товарам", widget);
    auto *tableLayout = new QVBoxLayout(tableGroup);

    m_topProductsTable = new QTableWidget(widget);
    m_topProductsTable->setColumnCount(4);
    m_topProductsTable->setHorizontalHeaderLabels({"Товар", "Цена за ед.", "Продано (шт.)", "Выручка (руб.)"});
    m_topProductsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_topProductsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_topProductsTable->horizontalHeader()->setStretchLastSection(true);
    m_topProductsTable->setColumnWidth(0, 260);
    m_topProductsTable->setColumnWidth(1, 120);
    m_topProductsTable->setColumnWidth(2, 120);
    tableLayout->addWidget(m_topProductsTable);

    mainLayout->addWidget(tableGroup);

    return widget;
}

void MainWindow::onTabChanged(int index)
{
    QWidget *current = m_tabWidget->widget(index);
    if (current == m_statsWidget) {
        refreshStatistics();
    } else if (current == m_productView) {
        m_productView->refreshData();
    } else if (current == m_orderView) {
        m_orderView->refreshData();
    } else if (current == m_deliveryView) {
        m_deliveryView->refreshData();
    } else if (m_customerView && current == m_customerView) {
        m_customerView->refreshData();
    }
}

void MainWindow::refreshStatistics()
{
    NetworkClient &net = NetworkClient::instance();

    // 1. Fetch products map (id -> Product)
    auto prodResp = net.sendRequest("get_products");
    QMap<int, Product> productMap;
    if (prodResp["success"].toBool()) {
        QJsonArray arr = prodResp["data"].toArray();
        for (const auto &val : arr) {
            Product p = Product::fromJson(val.toObject());
            productMap.insert(p.productId(), p);
        }
    }

    // 2. Fetch orders
    QString orderAction = m_isAdmin ? "get_orders" : "get_my_orders";
    auto ordersResp = net.sendRequest(orderAction);
    if (!ordersResp["success"].toBool()) return;

    QJsonArray ordersArr = ordersResp["data"].toArray();
    int orderCount = ordersArr.size();
    int totalItems = 0;
    double totalGoodsRevenue = 0.0;
    double totalDelivery = 0.0;

    struct ProductStat {
        QString name;
        double price = 0.0;
        int qty = 0;
        double revenue = 0.0;
    };
    QMap<int, ProductStat> statsMap;

    for (const auto &oVal : ordersArr) {
        int orderId = oVal.toObject()["order_id"].toInt();
        auto itemsResp = net.sendRequest("get_order_items", {{"order_id", orderId}});
        if (!itemsResp["success"].toBool()) continue;

        QJsonArray itemsArr = itemsResp["data"].toArray();
        for (const auto &iVal : itemsArr) {
            OrderItem item = OrderItem::fromJson(iVal.toObject());
            int pid = item.productId();
            int qty = item.quantity();
            double delCost = item.actualDeliveryCost();

            totalItems += qty;
            totalDelivery += delCost;

            double pPrice = productMap.contains(pid) ? productMap[pid].price() : 0.0;
            QString pName = productMap.contains(pid) ? productMap[pid].productName() : QString("Товар #%1").arg(pid);
            double lineTotal = pPrice * qty;
            totalGoodsRevenue += lineTotal;

            ProductStat &ps = statsMap[pid];
            ps.name = pName;
            ps.price = pPrice;
            ps.qty += qty;
            ps.revenue += lineTotal;
        }
    }

    double grandTotal = totalGoodsRevenue + totalDelivery;
    double avgOrder = (orderCount > 0) ? (grandTotal / orderCount) : 0.0;

    m_totalOrdersLabel->setText(QString::number(orderCount));
    m_totalItemsSoldLabel->setText(QString("%1 шт.").arg(totalItems));
    m_totalRevenueLabel->setText(QString("%1 руб.").arg(totalGoodsRevenue, 0, 'f', 2));
    m_totalDeliveryCostLabel->setText(QString("%1 руб.").arg(totalDelivery, 0, 'f', 2));
    m_averageOrderValueLabel->setText(QString("%1 руб.").arg(avgOrder, 0, 'f', 2));

    // Populate top products table
    m_topProductsTable->setRowCount(statsMap.size());
    int row = 0;
    for (auto it = statsMap.begin(); it != statsMap.end(); ++it, ++row) {
        const ProductStat &ps = it.value();
        m_topProductsTable->setItem(row, 0, new QTableWidgetItem(ps.name));
        m_topProductsTable->setItem(row, 1, new QTableWidgetItem(QString("%1 руб.").arg(ps.price, 0, 'f', 2)));
        m_topProductsTable->setItem(row, 2, new QTableWidgetItem(QString::number(ps.qty)));
        m_topProductsTable->setItem(row, 3, new QTableWidgetItem(QString("%1 руб.").arg(ps.revenue, 0, 'f', 2)));
    }
}

void MainWindow::onEditProfile()
{
    ProfileDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        setupStatusBar();
    }
}

void MainWindow::onLogout()
{
    if (QMessageBox::question(this, "Выход", "Завершить текущий сеанс?") == QMessageBox::Yes) {
        NetworkClient::instance().disconnect();
        emit logoutRequested();
        close();
    }
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "О программе",
        "<h3>Информационная система ведения заказов</h3>"
        "<p>Клиент-серверное приложение для оптовой торговли и отслеживания финансовых результатов.</p>"
        "<p><b>Архитектура:</b> TCP Client-Server (Qt6 C++), JSON Protocol, PostgreSQL DB.</p>"
        "<p><b>Разделение прав:</b> Администратор и Заказчик (Пользователь).</p>");
}
