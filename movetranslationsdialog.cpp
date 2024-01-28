#include "movetranslationsdialog.h"

#include <QGuiApplication>
#include <QScreen>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>


MoveTranslationsDialog::MoveTranslationsDialog(QWidget *parent):
	QDialog(parent)
{
	QStringList headerLabels = {tr("Выражение"), tr("Перевод"), tr("Напр.")};
	currentTableWidget = new QTableWidget(0, 3, this);
	currentTableWidget->setHorizontalHeaderLabels(headerLabels);
	currentTableWidget->resizeColumnToContents(2);
	currentTableWidget->setSortingEnabled(true);
	exclusionTableWidget = new QTableWidget(0, 3, this);
	exclusionTableWidget->setHorizontalHeaderLabels(headerLabels);
	exclusionTableWidget->resizeColumnToContents(2);
	exclusionTableWidget->setSortingEnabled(true);
	moveButton = new QPushButton("<>", this);
	changeDirButton = new QPushButton("Перевернуть", this);
	closeButton = new QPushButton("Закрыть", this);
	restoreAllButton = new QPushButton(tr("<<"), this);
	restoreAllButton->setToolTip(tr("Восстановить все"));

	QVBoxLayout *vboxlayout1 = new QVBoxLayout;
	vboxlayout1->addWidget(new QLabel(tr("Текущий список:"), this));
	vboxlayout1->addWidget(currentTableWidget);
	QVBoxLayout *vboxlayout2 = new QVBoxLayout;
	vboxlayout2->addStretch();
	vboxlayout2->addWidget(moveButton);
	vboxlayout2->addWidget(changeDirButton);
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
	layout->addLayout(hboxlayout1);

	setLayout(layout);
	//QScreen *screen = QApplication::screens.at(0);
	//QScreen* screen = QGuiApplication::screens().begin();
	QRect screenRect = QGuiApplication::primaryScreen()->geometry();
	setMinimumWidth(2*currentTableWidget->width() + changeDirButton->width() + 40);
	setMinimumHeight(currentTableWidget->height() + closeButton->height() + 30);
	int wdt = 640;
	int hgt = 400;
	setGeometry( (screenRect.width()-wdt)/2, (screenRect.height()-hgt)/2, wdt, hgt);

	connect(closeButton, &QPushButton::clicked,
			this, &QDialog::accept);

	connect(currentTableWidget, &QTableWidget::itemSelectionChanged,
			this, &MoveTranslationsDialog::tableItemSelectionChanged);
	connect(exclusionTableWidget, &QTableWidget::itemSelectionChanged,
			this, &MoveTranslationsDialog::tableItemSelectionChanged);

	connect(changeDirButton, &QPushButton::clicked,
			this, &MoveTranslationsDialog::changeDirection);
	connect(moveButton, &QPushButton::clicked,
			this, &MoveTranslationsDialog::moveTranslation);
	connect(restoreAllButton, &QPushButton::clicked,
			this, &MoveTranslationsDialog::restoreAllTranslations);
}
MoveTranslationsDialog::~MoveTranslationsDialog() {
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

void MoveTranslationsDialog::changeDirection()
{
	QTableWidget *widg = nullptr;
	if (! currentTableWidget->selectedItems().isEmpty())
		widg = currentTableWidget;
	else if (! exclusionTableWidget->selectedItems().isEmpty())
		widg = exclusionTableWidget;
	else return;

	int sldRow = widg->currentRow();
	TranslationItem *rowtip = getRowTIFromData(widg, sldRow);
	if (nullptr == rowtip)
		return;
	rowtip->setInvert(rowtip->isInvert()?false:true);
	setTableItemRow(widg, sldRow, rowtip);
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
		bool inv = rowtip->isInvert();
		item = new QTableWidgetItem();
		Qt::ItemFlags flags = (item->flags() & ~Qt::ItemIsEditable);
		item->setFlags(flags);
		item->setText(inv?rowtip->secondExpr():rowtip->firstExpr());
		currentTableWidget->setItem(row1, 0, item);
		item = new QTableWidgetItem();
		item->setFlags(flags);
		item->setText(inv?rowtip->firstExpr():rowtip->secondExpr());
		currentTableWidget->setItem(row1, 1, item);
		item = new QTableWidgetItem();
		item->setFlags(flags);
		item->setTextAlignment(Qt::AlignHCenter);
		item->setText(inv?"<--":"-->");
		currentTableWidget->setItem(row1, 2, item);
		setRowTIData(currentTableWidget, row1, rowtip);
	}
	currentTableWidget->setSortingEnabled(true);
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
}
void MoveTranslationsDialog::removeTranslation(const TranslationItem *tip) {
	Q_ASSERT(nullptr != tip);
	TranslationItem *rowtip = nullptr;
	for (int row=0; row<currentTableWidget->rowCount(); ++row) {
		rowtip = getRowTIFromData(currentTableWidget, row);
		if (tip == rowtip)
		{
			currentTableWidget->removeRow(row);
			return;
		}
	}
	for (int row=0; row<exclusionTableWidget->rowCount(); ++row) {
		rowtip = getRowTIFromData(exclusionTableWidget, row);
		if (tip == rowtip)
		{
			exclusionTableWidget->removeRow(row);
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
			return;
		}
	}
	for (int row=0; row<exclusionTableWidget->rowCount(); ++row) {
		rowtip = getRowTIFromData(exclusionTableWidget, row);
		if (tip == rowtip)
		{
			setTableItemRow(exclusionTableWidget, row, tip);
			return;
		}
	}
}
void MoveTranslationsDialog::setupTables(const QList<TranslationItem*> &l) {
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
}
// Процедура - itemы уже существуют в них только нужно установить текст поля
void MoveTranslationsDialog::setTableItemRow(QTableWidget *w, int row, const TranslationItem *tip) {
	Q_ASSERT(w->rowCount() >= row);
	w->setSortingEnabled(false);
	if (!tip->isInvert()) {
		w->item(row, 0)->setText(tip->firstExpr());
		w->item(row,1)->setText(tip->secondExpr());
		w->item(row, 2)->setText("-->");
	}
	else {
		w->item(row, 0)->setText(tip->secondExpr());
		w->item(row,1)->setText(tip->firstExpr());
		w->item(row, 2)->setText("<--");
	}
	w->setSortingEnabled(true);
}
// с нуля создание и заполнение нового ряда
bool MoveTranslationsDialog::setupTableItemRow(QTableWidget *widg, int row, const TranslationItem *tip)
{
	Q_ASSERT(widg->rowCount() >= row);
	//if (widg->rowCount()<row)
	//	return false;
	widg->insertRow(row);
	widg->setSortingEnabled(false);
	QTableWidgetItem *item = nullptr;
	bool inv = tip->isInvert();
	item = new QTableWidgetItem();
	Qt::ItemFlags flags = (item->flags() & ~Qt::ItemIsEditable);
	item->setFlags(flags);
	item->setText(inv?tip->secondExpr():tip->firstExpr());
	widg->setItem(row, 0, item);
	item = new QTableWidgetItem();
	item->setFlags(flags);
	item->setText(inv?tip->firstExpr():tip->secondExpr());
	widg->setItem(row, 1, item);
	item = new QTableWidgetItem();
	item->setFlags(flags);
	item->setTextAlignment(Qt::AlignHCenter);
	item->setText(inv?"<--":"-->");
	widg->setItem(row, 2, item);

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
