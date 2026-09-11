#ifndef CUSTOMERVIEW_H
#define CUSTOMERVIEW_H
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
class CustomerView : public QWidget {
    Q_OBJECT
public:
    explicit CustomerView(bool isAdmin, QWidget *parent = nullptr);
    void refreshData();
private slots:
    void onDelete();
private:
    QTableWidget *m_table;
    QPushButton *m_deleteBtn;
    bool m_isAdmin;
};
#endif
