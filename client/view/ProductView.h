#ifndef PRODUCTVIEW_H
#define PRODUCTVIEW_H
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
class ProductView : public QWidget {
    Q_OBJECT
public:
    explicit ProductView(bool isAdmin, QWidget *parent = nullptr);
    void refreshData();
private slots:
    void onAdd();
    void onEdit();
    void onDelete();
private:
    QTableWidget *m_table;
    QPushButton *m_addBtn, *m_editBtn, *m_deleteBtn;
    bool m_isAdmin;
};
#endif
