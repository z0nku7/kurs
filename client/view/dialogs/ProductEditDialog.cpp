#include "ProductEditDialog.h"
ProductEditDialog::ProductEditDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Товар"); setMinimumWidth(400);
    auto *l = new QFormLayout(this);
    m_nameEdit = new QLineEdit; l->addRow("Название:", m_nameEdit);
    m_priceEdit = new QDoubleSpinBox; m_priceEdit->setRange(0, 999999999.99);
    m_priceEdit->setDecimals(2); m_priceEdit->setSuffix(" ₽"); l->addRow("Цена:", m_priceEdit);
    m_refInfoEdit = new QTextEdit; m_refInfoEdit->setMaximumHeight(100);
    l->addRow("Справочная\nинформация:", m_refInfoEdit);
    m_deliveryCheck = new QCheckBox("Доступна"); m_deliveryCheck->setChecked(true);
    l->addRow("Доставка:", m_deliveryCheck);
    auto *b = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(b, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(b, &QDialogButtonBox::rejected, this, &QDialog::reject);
    l->addRow(b);
}
void ProductEditDialog::setProduct(const Product &p) {
    m_nameEdit->setText(p.productName()); m_priceEdit->setValue(p.price());
    m_refInfoEdit->setText(p.referenceInfo()); m_deliveryCheck->setChecked(p.isDeliveryAvailable());
}
QString ProductEditDialog::productName() const { return m_nameEdit->text().trimmed(); }
double ProductEditDialog::price() const { return m_priceEdit->value(); }
QString ProductEditDialog::referenceInfo() const { return m_refInfoEdit->toPlainText().trimmed(); }
bool ProductEditDialog::isDeliveryAvailable() const { return m_deliveryCheck->isChecked(); }
