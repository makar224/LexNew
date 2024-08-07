#include "movetranslationsdialog.h"

#include <QGuiApplication>
#include <QScreen>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>


MoveTranslationsDialog::MoveTranslationsDialog(QWidget *parent):
	QDialog(parent)
{
	QStringList headerLabels = {tr("Выражение"), tr("Перевод")};
	currentTableWidget = new QTableWidget(0, 2, this);
	currentTableWidget->setHorizontalHeaderLabels(headerLabels);
	//currentTableWidget->resizeColumnToContents(2);
	currentTableWidget->setSortingEnabled(true);
	exclusionTableWidget = new QTableWidget(0, 2, this);
	exclusionTableWidget->setHorizontalHeaderLabels(headerLabels);
	//exclusionTableWidget->resizeColumnToContents(2);
	exclusionTableWidget->setSortingEnabled(true);
	moveButton = new QPushButton("<>", this);
	moveButton->setToolTip(tr("Включить/исключить"));
	closeButton = new QPushButton(tr("Закрыть"), this);
	restoreAllButton = new QPushButton(tr("<<"), this);
	restoreAllButton->setToolTip(tr("Восстановить все"));

	QVBoxLayout *vboxlayout1 = new QVBoxLayout;
	vboxlayout1->addWidget(new QLabel(tr("Текущий список:"), this));
	vboxlayout1->addWidget(currentTableWidget);
	QVBoxLayout *vboxlayout2 = new QVBoxLayout;
	vboxlayout2->addStretch();
	vboxlayout2->addWidget(moveButton);
	vboxlayout2->addStretch();
	vboxlayout2->addWidget(restoreAllButton);
	QVBoxLayout *vboxlayout3 = new QVBoxLayout;
	vboxlayout3->addWidget(new QLabel(tr("Список исключения:"), this));
	vboxlayout3->addWidget(exclusionTableWidget);

	QHBoxLayout *hboxlayout = new QHBoxLayout;
	hboxlayout->addLayout(vboxlayout1);
	hboxlayout->addLayout(vboxlayout2);
	hboxlayout->addLayout(vboxlayout3);
	QHBoxLayout *hboxlayout1 = new QHBoxLayout;
	hboxlayout1->addStretch();
	hboxlayout1->addWidget(closeButton);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addLayout(hboxlayout);
	layout->addSpacing(10);
	layout->addLayout(hboxlayout1);

	setLayout(layout);
	//QScreen *screen = QApplication::screens.at(0);
	//QScreen* screen = QGuiApplication::screens().begin();
	QRect screenRect = QGuiApplication::primaryScreen()->geometry();
	setMinimumWidth(2*currentTableWidget->width() + moveButton->width() + 40);
	setMinimumHeight(currentTableWidget->height() + closeButton->height() + 30);
	int wdt = 680;
	int hgt = 450;
	setGeometry( (screenRect.width()-wdt)/2, (screenRect.height()-hgt)/2, wdt, hgt);

	connect(closeButton, &QPushButton::clicked,
			this, &QDialog::accept);

	connect(currentTableWidget, &QTableWidget::itemSelectionChanged,
			this, &MoveTranslationsDialog::tableItemSelectionChanged);
	connect(exclusionTableWidget, &QTableWidget::itemSelectionChanged,
			this, &MoveTranslationsDialog::tableItemSelectionChanged);

	connect(moveButton, &QPushButton::clicked,
			this, &MoveTranslationsDialog::moveTranslation);
	connect(restoreAllButton, &QPushButton::clicked,
			this, &MoveTranslationsDialog::restoreAllTranslations);
}
MoveTranslationsDialog::~MoveTranslationsDialog() {
}
void MoveTranslationsDialog::resizeEvent(QResizeEvent *e) {
	int wdt = currentTableWidget->width();
	int cwdt = (wdt-currentTableWidget->columnWidth(2))/2 - currentTableWidget->verticalHeader()->width()/2	-2;
	currentTableWidget->setColumnWidth(0, cwdt);
	currentTableWidget->setColumnWidth(1, cwdt);
	exclusionTableWidget->setColumnWidth(0, cwdt);
	exclusionTableWidget->setColumnWidth(1, cwdt);
	QDialog::resizeEvent(e);
}
void MoveTranslationsDialog::tableItemSelectionChanged()
{
	static bool recursion = false;
	if (recursion)
		return;
	QTableWidget *tableWidget = qobject_cast<QTableWidget*>(QObject::sender());
	if (nullptr==tableWidget)
		return;
	QTableWidget *contrTableWidget = nullptr;
	if (tableWidget == currentTableWidget)
		contrTableWidget = exclusionTableWidget;
	else if (tableWidget == exclusionTableWidget)
		contrTableWidget = currentTableWidget;
	else return;
	QList<QTableWidgetItem*> sldItems = contrTableWidget->selectedItems();
	if (!sldItems.isEmpty()) {// переключение
		int sldRow = (*sldItems.first()).row();
		recursion = true;
		contrTableWidget->setRangeSelected(
					QTableWidgetSelectionRange(sldRow, 0, sldRow,
											   contrTableWidget->columnCount()-1), false);
		sldRow = tableWidget->currentRow();
		tableWidget->setRangeSelected(
					QTableWidgetSelectionRange(sldRow, 0, sldRow, tableWidget->columnCount()-1), true);
		recursion = false;
		return;
	}

	int curRow = tableWidget->currentRow();
	QList<int> uniqueRows;
	sldItems = tableWidget->selectedItems();
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
	//tableWidget->selectRow(curRow);
	recursion = false;
}

void MoveTranslationsDialog::moveTranslation()
{
	QTableWidget *widg = nullptr, *contrWidg = nullptr;
	if (! currentTableWidget->selectedItems().isEmpty()) {
		widg = currentTableWidget;
		contrWidg = exclusionTableWidget;
	}
	else if (! exclusionTableWidget->selectedItems().isEmpty()) {
		widg = exclusionTableWidget;
		contrWidg = currentTableWidget;
	}
	else return;
	Q_ASSERT(nullptr != widg);
	int sldRow = widg->currentRow();
	TranslationItem *rowtip = getRowTIFromData(widg, sldRow);
	if (nullptr == rowtip)
		return;
	if (widg == currentTableWidget)
		rowtip->setExcluded(true);
	else {
		rowtip->setExcluded(false);
		rowtip->resetSuccessCounter();
	}
	widg->removeRow(sldRow);
	int contrSldRow = contrWidg->rowCount();
	setupTableItemRow(contrWidg, contrSldRow, rowtip);
	//contrWidg->setCurrentCell(contrSldRow, 0); // - важно установить, чтобы в след обработчике выделения итема (~после widg->removeRow) выделился current-ряд, где добавился ряд, иначе выделяется ряд в том table widget из которого удалился ряд
	//setWindowModified(true);
}
void MoveTranslationsDialog::restoreAllTranslations() {
	currentTableWidget->setSortingEnabled(false);
	TranslationItem *rowtip = nullptr;
	QTableWidgetItem *item = nullptr;
	int row = exclusionTableWidget->rowCount() -1;
	int row1 = 0;
	for (; row>=0; --row) {
		row1 = currentTableWidget->rowCount();
		rowtip = getRowTIFromData(exclusionTableWidget, row);
		Q_ASSERT(nullptr != rowtip);
		exclusionTableWidget->removeRow(row);
		rowtip->setExcluded(false);
		rowtip->resetSuccessCounter();
		currentTableWidget->insertRow(row1);
		item = new QTableWidgetItem();
		Qt::ItemFlags flags = (item->flags() & ~Qt::ItemIsEditable);
		item->setFlags(flags);
		item->setText(rowtip->firstExpr());
		currentTableWidget->setItem(row1, 0, item);
		item = new QTableWidgetItem();
		item->setFlags(flags);
		item->setText(rowtip->secondExpr());
		currentTableWidget->setItem(row1, 1, item);
		setRowTIData(currentTableWidget, row1, rowtip);
	}
	currentTableWidget->setSortingEnabled(true);
	//setWindowModified(true);
}
void MoveTranslationsDialog::excludeTranslation(TranslationItem *tip) {
	TranslationItem *rowtip = nullptr;
	for (int row=0; row<currentTableWidget->rowCount(); ++row) {
		rowtip = getRowTIFromData(currentTableWidget, row);
		if (tip == rowtip)
		{
			currentTableWidget->removeRow(row);
			setupTableItemRow(exclusionTableWidget, exclusionTableWidget->rowCount(), rowtip);
			tip->setExcluded(true);
			//tip->resetSuccessCounter();
			//setWindowModified(true);
			return;
		}
	}
}
void MoveTranslationsDialog::addTranslation(const TranslationItem *tip)
{
	Q_ASSERT(nullptr != tip);
	Q_ASSERT(!tip->isExcluded());
	int row = currentTableWidget->rowCount();
	setupTableItemRow(currentTableWidget, row, tip);
	//setWindowModified(true);
}
void MoveTranslationsDialog::removeTranslation(const TranslationItem *tip) {
	Q_ASSERT(nullptr != tip);
	TranslationItem *rowtip = nullptr;
	for (int row=0; row<currentTableWidget->rowCount(); ++row) {
		rowtip = getRowTIFromData(currentTableWidget, row);
		if (tip == rowtip)
		{
			currentTableWidget->removeRow(row);
			//setWindowModified(true);
			return;
		}
	}
	for (int row=0; row<exclusionTableWidget->rowCount(); ++row) {
		rowtip = getRowTIFromData(exclusionTableWidget, row);
		if (tip == rowtip)
		{
			exclusionTableWidget->removeRow(row);
			//setWindowModified(true);
			return;
		}
	}
}
void MoveTranslationsDialog::editTranslation(const TranslationItem *tip) {
	Q_ASSERT(nullptr != tip);
	TranslationItem *rowtip = nullptr;
	for (int row=0; row<currentTableWidget->rowCount(); ++row) {
		rowtip = getRowTIFromData(currentTableWidget, row);
		if (tip == rowtip)
		{
			setTableItemRow(currentTableWidget, row, tip);
			//setWindowModified(true);
			return;
		}
	}
	for (int row=0; row<exclusionTableWidget->rowCount(); ++row) {
		rowtip = getRowTIFromData(exclusionTableWidget, row);
		if (tip == rowtip)
		{
			setTableItemRow(exclusionTableWidget, row, tip);
			//setWindowModified(true);
			return;
		}
	}
}
void MoveTranslationsDialog::setupTables(const QList<TranslationItem*> &l) {
	clearTables();
	QTableWidget *tableWidget = nullptr;
	int row = 0;
	for(const TranslationItem *tip: l) {
		if (! tip->isExcluded())
			tableWidget = currentTableWidget;
		else
			tableWidget = exclusionTableWidget;
		row = tableWidget->rowCount();
		setupTableItemRow(tableWidget, row, tip);
	}
	setWindowModified(false);
}
void MoveTranslationsDialog::clearTables() {
	int row = currentTableWidget->rowCount()-1;
	for (;row>=0; --row)
		currentTableWidget->removeRow(row);
	for (row=exclusionTableWidget->rowCount()-1; row>=0; --row)
		exclusionTableWidget->removeRow(row);
	setWindowModified(false);
}
// Процедура - itemы уже существуют в них только нужно установить текст поля
void MoveTranslationsDialog::setTableItemRow(QTableWidget *w, int row, const TranslationItem *tip) {
	Q_ASSERT(w->rowCount() >= row);
	w->setSortingEnabled(false);
	w->item(row, 0)->setText(tip->firstExpr());
	w->item(row, 1)->setText(tip->secondExpr());
	w->setSortingEnabled(true);
}
// с нуля создание и заполнение нового ряда
bool MoveTranslationsDialog::setupTableItemRow(QTableWidget *widg, int row, const TranslationItem *tip)
{
	Q_ASSERT(widg->rowCount() >= row);
	//if (widg->rowCount()<row)
	//	return false;
	widg->insertRow(row);
	widg->setRowHeight(row, 25);
	widg->setSortingEnabled(false);
	QTableWidgetItem *item = nullptr;
	item = new QTableWidgetItem();
	Qt::ItemFlags flags = (item->flags() & ~Qt::ItemIsEditable);
	item->setFlags(flags);
	item->setText(tip->firstExpr());
	widg->setItem(row, 0, item);
	item = new QTableWidgetItem();
	item->setFlags(flags);
	item->setText(tip->secondExpr());
	widg->setItem(row, 1, item);

	setRowTIData(widg, row, tip);
	widg->setSortingEnabled(true);
	return true;
}
TranslationItem *MoveTranslationsDialog::getRowTIFromData(const QTableWidget *widg, int row) const {
	QVariant rowVarData = widg->item(row, 0)->data(Qt::UserRole);
	TranslationItem *rowtip = nullptr;
	rowtip = (TranslationItem*)rowVarData.toULongLong();
	Q_ASSERT(nullptr != rowtip);
	return rowtip;
}
bool MoveTranslationsDialog::setRowTIData(QTableWidget *widg, int row, const TranslationItem *tip) {
	if (widg->item(row,0) == nullptr)
		return false;
	widg->item(row, 0)->setData(Qt::UserRole, QVariant((unsigned long long)tip));
	return true;
}
