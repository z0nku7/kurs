#ifndef DELIVERYVIEW_H
#define DELIVERYVIEW_H
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QHeaderView>
#include <QMessageBox>
class DeliveryView : public QWidget {
    Q_OBJECT
public:
    explicit DeliveryView(bool isAdmin, QWidget *parent = nullptr);
    void refreshData();
private slots:
    void onAddMethod(); void onEditMethod(); void onDeleteMethod();
    void onAddTariff(); void onEditTariff(); void onDeleteTariff();
private:
    void refreshMethods(); void refreshTariffs();
    QTableWidget *m_methodTable, *m_tariffTable;
    QPushButton *m_addMethodBtn, *m_editMethodBtn, *m_deleteMethodBtn;
    QPushButton *m_addTariffBtn, *m_editTariffBtn, *m_deleteTariffBtn;
    bool m_isAdmin;
};
#endif
