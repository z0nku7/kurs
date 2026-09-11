#ifndef ORDERVIEW_H
#define ORDERVIEW_H
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDateEdit>
class OrderView : public QWidget {
    Q_OBJECT
public:
    explicit OrderView(bool isAdmin, int customerId = -1, QWidget *parent = nullptr);
    void refreshData();
private slots:
    void onAddOrder();
    void onDeleteOrder();
    void onOrderSelected();
    void onAddItem();
    void onDeleteItem();
private:
    void refreshOrderItems(int orderId);
    QTableWidget *m_orderTable, *m_itemTable;
    QPushButton *m_addOrderBtn, *m_deleteOrderBtn, *m_addItemBtn, *m_deleteItemBtn;
    bool m_isAdmin;
    int m_customerId;
};
#endif
