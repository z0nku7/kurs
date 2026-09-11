#ifndef PRODUCTEDITDIALOG_H
#define PRODUCTEDITDIALOG_H
#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QFormLayout>
#include <QDialogButtonBox>
#include "Product.h"
class ProductEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProductEditDialog(QWidget *parent = nullptr);
    void setProduct(const Product &p);
    QString productName() const;
    double price() const;
    QString referenceInfo() const;
    bool isDeliveryAvailable() const;
private:
    QLineEdit *m_nameEdit;
    QDoubleSpinBox *m_priceEdit;
    QTextEdit *m_refInfoEdit;
    QCheckBox *m_deliveryCheck;
};
#endif
