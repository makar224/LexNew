#include "movetranslationsdialog.h"

#include <QGuiApplication>
#include <QScreen>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>


MoveTranslationsDialog::MoveTranslationsDialog(QWidget *parent):
	QDialog(parent)
{
	QStringList headerLabels = {tr("Выражение"), tr("Перевод"), tr("Направление")};
	//QStringList() << tr("Выражение") << tr("Перевод") << tr("Направление"));
	currentTableWidget = new QTableWidget(0, 3, this);
	currentTableWidget->setHorizontalHeaderLabels(headerLabels);
	currentTableWidget->setSortingEnabled(true);
	exclusionTableWidget = new QTableWidget(0, 3, this);
	exclusionTableWidget->setHorizontalHeaderLabels(headerLabels);
	exclusionTableWidget->setSortingEnabled(true);
	//new QLabel(tr("Текущий список:"), this);
	//exclusionTableLabel = new QLabel(tr("Текущий список:"), this);
	moveButton = new QPushButton("<>", this);
	changeDirButton = new QPushButton("Перевернуть", this);
	closeButton = new QPushButton("Закрыть", this);

	/*QGridLayout *layout = new QGridLayout;
	layout->addWidget(new QLabel(tr("Текущий список:"), this), 0, 0, Qt::AlignLeft);
	layout->addWidget(new QLabel(tr("Список исключения:"), this), 0, 2, Qt::AlignLeft);
	layout->addWidget(currentTableWidget, 1, 0, 2, 1);
	layout->addWidget(exclusionTableWidget, 1, 2, 2, 1);
	QVBoxLayout *buttonsLayout = new QVBoxLayout;
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(moveButton);
	buttonsLayout->addWidget(changeDirButton);
	buttonsLayout->addStretch();
	//layout->addWidget(moveButton, 0, 1);
	//layout->addWidget(changeDirButton, 1, 1);
	layout->addLayout(buttonsLayout, 0, 1, 3, 1);
	layout->addWidget(closeButton, 3, 2, Qt::AlignRight);*/

	QVBoxLayout *vboxlayout1 = new QVBoxLayout;
	vboxlayout1->addWidget(new QLabel(tr("Текущий список:"), this));
	vboxlayout1->addWidget(currentTableWidget);
	QVBoxLayout *vboxlayout2 = new QVBoxLayout;
	vboxlayout2->addStretch();
	vboxlayout2->addWidget(moveButton);
	vboxlayout2->addWidget(changeDirButton);
	QVBoxLayout *vboxlayout3 = new QVBoxLayout;
	vboxlayout2->addStretch();
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
	//setGeometry(200, 200, 640, 400);
	int wdt = 640;
	int hgt = 400;
	setGeometry( (screenRect.width()-wdt)/2, (screenRect.height()-hgt)/2, wdt, hgt);

	connect(closeButton, &QPushButton::clicked,
			this, &QDialog::accept);

	connect(currentTableWidget, &QTableWidget::itemSelectionChanged,
			this, &MoveTranslationsDialog::tableItemSelectionChanged);
	connect(exclusionTableWidget, &QTableWidget::itemSelectionChanged,
			this, &MoveTranslationsDialog::tableItemSelectionChanged);

	//connect(currentTableWidget, &QTableWidget::currentCellChanged,
	//		this, &MoveTranslationsDialog::currentTableCurrentCellChanged);

	connect(changeDirButton, &QPushButton::clicked,
			this, &MoveTranslationsDialog::changeDirection);
	connect(moveButton, &QPushButton::clicked,
			this, &MoveTranslationsDialog::moveTranslation);
}
MoveTranslationsDialog::~MoveTranslationsDialog() {
	/*delete currentTableWidget;
	delete exclusionTableWidget;
	delete moveButton;
	delete changeDirButton;
	delete closeButton;*/
}
/*void done(int result) {
	if (result == QDialog::Accepted) {
}*/
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

/*void MoveTranslationsDialog::currentTableCurrentCellChanged(int curRow,int,int prevRow,int) {
	if (nullptr == currentTableWidget)
		return;

	if (curRow != prevRow) { // не перемещение по колонкам в одном ряду

		QList<int> uniqueRows;
		QList<QTableWidgetItem*> sldItems = currentTableWidget->selectedItems();
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
		for (int i :uniqueRows)
			currentTableWidget->setRangeSelected(
						QTableWidgetSelectionRange(i, 0, i,
												   currentTableWidget->columnCount()-1), false);
	}
	currentTableWidget->setRangeSelected(
				QTableWidgetSelectionRange(curRow, 0, curRow,
										   currentTableWidget->columnCount()-1), true);
}*/
void MoveTranslationsDialog::changeDirection()
{
	//QList<QTableWidgetItem*> sldItems = contrTableWidget->selectedItems();
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
	//contrWidg->insertRow(contrSldRow);
	setupTableItemRow(contrWidg, contrSldRow, rowtip);
	//contrWidg->setCurrentCell(contrSldRow, 0); // - важно установить, чтобы в след обработчике выделения итема (~после widg->removeRow) выделился current-ряд, где добавился ряд, иначе выделяется ряд в том table widget из которого удалился ряд

	//contrWidg->setRangeSelected(
	//			QTableWidgetSelectionRange(contrSldRow, 0, contrSldRow,
	//									   contrWidg->columnCount()-1), true);
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
			tip->resetSuccessCounter();
			return;
		}
	}
}
void MoveTranslationsDialog::addTranslation(const TranslationItem *tip)
{
	Q_ASSERT(nullptr != tip);
	Q_ASSERT(!tip->isExcluded());
	int row = currentTableWidget->rowCount();
	//currentTableWidget->insertRow(row);
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
void MoveTranslationsDialog::setupTables(const QVector<TranslationItem*> &v) {
	QTableWidget *tableWidget = nullptr;
	int row = 0;
	for(const TranslationItem *tip: v) {
		if (! tip->isExcluded())
			tableWidget = currentTableWidget;
		else
			tableWidget = exclusionTableWidget;
		row = tableWidget->rowCount();
		//tableWidget->insertRow(row);
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
	item->setText(inv?"<--":"-->");
	widg->setItem(row, 2, item);

	/*if (tip->isInvert()) {
		widg->setItem(row, 0, new QTableWidgetItem( tip->secondExpr() ));
		widg->setItem(row, 1, new QTableWidgetItem( tip->firstExpr() ));
		widg->setItem(row, 2, new QTableWidgetItem("<--"));
	}
	else {
		widg->setItem(row, 0, new QTableWidgetItem( tip->firstExpr() ));
		widg->setItem(row, 1, new QTableWidgetItem( tip->secondExpr() ));
		widg->setItem(row, 2, new QTableWidgetItem("-->"));
	}*/
	setRowTIData(widg, row, tip);
	widg->setSortingEnabled(true);
	return true;
}
TranslationItem *MoveTranslationsDialog::getRowTIFromData(const QTableWidget *widg, int row) const {
	QVariant rowVarData = widg->item(row, 0)->data(Qt::UserRole);
	TranslationItem *rowtip = nullptr;
	//if (rowVarData.canConvert<TranslationItem*>())
	//	rowtip = rowVarData.value<TranslationItem*>();
	//TranslationItem *rowtip = static_cast<TranslationItem*>(rowVarData.data());
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

/*void MoveTranslationsDialog::updateTables(const TranslationItem *tip) {
	if (nullptr == tip)
		return;

	if (! tip->isExcluded() ) {
		for (int i=0; i<currentTableWidget->rowCount(); i++) {
			QVariant rowVarData = currentTableWidget->item(i, 0)->data(Qt::UserRole);
			const TranslationItem *rowtip = static_cast<const TranslationItem*>(rowVarData.data());
			// --> проверить правильно ли преобразован
			if (0 == rowtip)
				break;

			if (rowtip == tip) {
				// Обновить весь ряд или удалить ряд
			}
			else {
				// Добавить ряд
			}
		}
	}
	else {
		// ищем в excludedTableWidget


	}
}*/
