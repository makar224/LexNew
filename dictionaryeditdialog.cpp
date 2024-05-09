#include "dictionaryeditdialog.h"
#include <QKeyEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>


DictionaryEditDialog::DictionaryEditDialog(QWidget *parent) :
	QDialog(parent)
{
	QStringList headerLabels = {tr("Выражение"), tr("Перевод")};
	tableWidget = new QTableWidget(0, 2, this);
	tableWidget->setHorizontalHeaderLabels(headerLabels);
	tableWidget->setSortingEnabled(true);
	addButton = new QPushButton("+", this);
	removeButton = new QPushButton("-", this);
	saveButton = new QPushButton(tr("Сохранить"), this);
	closeButton = new QPushButton(tr("Закрыть"), this);
	expr1Edit = new QLineEdit(this);
	expr2Edit = new QLineEdit(this);

	QHBoxLayout *hboxlayout1 = new QHBoxLayout;
	hboxlayout1->addStretch();
	hboxlayout1->addWidget(addButton);
	hboxlayout1->addWidget(removeButton);
	QHBoxLayout *hboxlayout2 = new QHBoxLayout;
	hboxlayout2->addStretch();
	hboxlayout2->addWidget(expr1Edit);
	hboxlayout2->addWidget(expr2Edit);
	QHBoxLayout *hboxlayout3 = new QHBoxLayout;
	hboxlayout3->addWidget(saveButton);
	hboxlayout3->addStretch();
	hboxlayout3->addWidget(closeButton);
	QVBoxLayout *layout = new QVBoxLayout;
	layout->addLayout(hboxlayout1);
	layout->addLayout(hboxlayout2);
	layout->addWidget(tableWidget);
	layout->addSpacing(10);
	layout->addLayout(hboxlayout3);
	setLayout(layout);

	QRect screenRect = QGuiApplication::primaryScreen()->geometry();
	setMinimumWidth(tableWidget->width() + 20);
	//setMinimumHeight(tableWidget->height() + addButton->height() + closeButton->height() + 50);
	setMinimumHeight(tableWidget->height() + closeButton->height() + 40);
	int wdt = 350;
	int hgt = 450;
	setGeometry( (screenRect.width()-wdt)/2, (screenRect.height()-hgt)/2, wdt, hgt);

	tableWidget->installEventFilter(this);
	connect(tableWidget, &QTableWidget::itemSelectionChanged,
			this, &DictionaryEditDialog::tableItemSelectionChanged);
	connect(tableWidget, &QTableWidget::cellChanged,
			this, &DictionaryEditDialog::editTranslation);
	connect(closeButton, &QPushButton::clicked,
			this, &QDialog::accept);
	connect(addButton, &QPushButton::clicked,
			this, &DictionaryEditDialog::addTranslation);
	//connect(expr1Edit, &QLineEdit::returnPressed,
	//		this, SLOT(addTranslation));
	connect(removeButton, &QPushButton::clicked,
			this, &DictionaryEditDialog::removeTranslation);
}
void DictionaryEditDialog::resizeEvent(QResizeEvent *e) {
	int linewdt = tableWidget->width()/2-tableWidget->verticalHeader()->width()/2-2;
	tableWidget->setColumnWidth(0, linewdt);
	tableWidget->setColumnWidth(1, linewdt);
	expr1Edit->setMinimumWidth(linewdt-1);
	expr2Edit->setMinimumWidth(linewdt-1);
	QDialog::resizeEvent(e);
}
bool DictionaryEditDialog::eventFilter(QObject *obj, QEvent *event) {
	if (obj == tableWidget)
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
			if (keyEvent->key()==Qt::Key_Delete
					|| keyEvent->key()==Qt::Key_Backspace)
			{
				//qDebug("Ate key press %d", keyEvent->key());
				removeTranslation();
				return true;
			}
		}
	// pass the event on to the parent class
	return QDialog::eventFilter(obj, event);
}
void DictionaryEditDialog::tableItemSelectionChanged()
{
	static bool recursion = false;
	if (recursion)
		return;

	int curRow = tableWidget->currentRow();
	QList<int> uniqueRows;
	QList<QTableWidgetItem*> sldItems = tableWidget->selectedItems();
	// оставляем только уникальные значения для рядов item-ов в uniqueRows
	bool match = false;
	for (QTableWidgetItem *item: sldItems) {
		if (item->row() == curRow)
			continue;
		for (int i: uniqueRows)
			if (item->row()==i)
				match = true;
		if (!match)
			uniqueRows.append(item->row());
		match = false;
	}
	recursion = true;
	for (int i :uniqueRows)
		tableWidget->setRangeSelected(
					QTableWidgetSelectionRange(i, 0, i,
											   tableWidget->columnCount()-1), false);

	tableWidget->setRangeSelected(
				QTableWidgetSelectionRange(curRow, 0, curRow, tableWidget->columnCount()-1), true);
	recursion = false;
}
// ! можно вставлять одинковые по обоим полям строки
void DictionaryEditDialog::addTranslation() {
	// должны быть expr1Edit и expr2Edit заполнены, и хотя бы одно из них модифицировано или нет рядов в таблице
	if ((!expr1Edit->isModified() && !expr2Edit->isModified() && tableWidget->rowCount()>0)
		|| (expr1Edit->text()=="" || expr2Edit->text()==""))
		return;

	TranslationItem *tip = new TranslationItem(expr1Edit->text(), expr2Edit->text());
	expr1Edit->clear();
	expr2Edit->clear();
	expr1Edit->setModified(false);
	expr2Edit->setModified(false);
    expr1Edit->setFocus();
	int row = tableWidget->rowCount();
	tableWidget->insertRow(row);
	setupTableItemRow(row, tip);
	setWindowModified(true);
	emit addTranslationSig(tip);
}
void DictionaryEditDialog::removeTranslation() {
	QList<QTableWidgetItem*> sldItems = tableWidget->selectedItems();
	if (sldItems.isEmpty())
		return;
	int sldRow = (*sldItems.first()).row();
	TranslationItem *tip = getRowTIFromData(sldRow);
	Q_ASSERT(nullptr != tip);
	tableWidget->removeRow(sldRow);
	setWindowModified(true);
	emit removeTranslationSig(tip);
}
void DictionaryEditDialog::editTranslation(int row, int col) {
	Q_ASSERT(0==col || 1==col);
	Q_ASSERT(tableWidget->item(row, col) != nullptr);
	TranslationItem *tip = getRowTIFromData(row);
	//Q_ASSERT(nullptr != tip);
	if (nullptr == tip)
		return; // это не редактирование, а первое заполнение полей в setupTable

	if (0 == col)
		tip->setFirstExpr(tableWidget->item(row, col)->text());
	else if (1 == col)
		tip->setSecondExpr(tableWidget->item(row, col)->text());
	else
		return;
	setWindowModified(true);
	emit editTranslationSig(tip);
}
void DictionaryEditDialog::setupTable(const QList<TranslationItem*> &l) {
	clearTable();
	int row = 0;
	for(const TranslationItem *tip: l) {
		tableWidget->insertRow(row);
		setupTableItemRow(row, tip);
		++row;
	}
	setWindowModified(false);
}
void DictionaryEditDialog::clearTable() {
	for (int row=tableWidget->rowCount()-1; row>=0; --row) {
		tableWidget->removeRow(row);
	}
	setWindowModified(false);
}
bool DictionaryEditDialog::setupTableItemRow(int row, const TranslationItem *tip)
{
	Q_ASSERT(tableWidget->rowCount() >= row);
	if (tableWidget->rowCount()<row)
		return false;

	tableWidget->setRowHeight(row, 25);
	tableWidget->setSortingEnabled(false);
	tableWidget->setItem(row, 0, new QTableWidgetItem( tip->firstExpr() ));
	tableWidget->setItem(row, 1, new QTableWidgetItem( tip->secondExpr() ));
	//setRowTIData(widg, row, tip);
	tableWidget->item(row, 0)->setData(Qt::UserRole, QVariant((unsigned long long)tip));
	tableWidget->setSortingEnabled(true);
	return true;
}

TranslationItem *DictionaryEditDialog::getRowTIFromData(int row) const {
	QVariant rowVarData = tableWidget->item(row, 0)->data(Qt::UserRole);
	TranslationItem *rowtip = (TranslationItem*)rowVarData.toULongLong();
	//Q_ASSERT(nullptr != rowtip); // если условие не сохранилось - это не редактирование, а первое заполнение полей в setupTable
	return rowtip;
}
