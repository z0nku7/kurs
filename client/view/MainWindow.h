#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>

class ProductView;
class OrderView;
class CustomerView;
class DeliveryView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

signals:
    void logoutRequested();

private slots:
    void onLogout();
    void onEditProfile();
    void onAbout();
    void onTabChanged(int index);
    void refreshStatistics();

private:
    void setupUi();
    void setupMenu();
    void setupStatusBar();
    QWidget* createStatsWidget();

    QTabWidget *m_tabWidget;
    ProductView *m_productView;
    OrderView *m_orderView;
    CustomerView *m_customerView;
    DeliveryView *m_deliveryView;
    QWidget *m_statsWidget;

    // Stats labels
    QLabel *m_totalOrdersLabel;
    QLabel *m_totalRevenueLabel;
    QLabel *m_totalDeliveryCostLabel;
    QLabel *m_averageOrderValueLabel;
    QLabel *m_totalItemsSoldLabel;
    QTableWidget *m_topProductsTable;

    QLabel *m_statusLabel;
    bool m_isAdmin;
};

#endif // MAINWINDOW_H
