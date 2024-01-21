#include "translationdialog.h"
#include "time.h"
#include <QGuiApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QShowEvent>

TranslationDialog::TranslationDialog(QWidget *parent, QVector<TranslationItem*>* tivp) :
	//QDialog(parent),
	QWidget(parent),
	mTrItemsVPtr(tivp),
	mRequestTrItem(nullptr),
	nSessionMinutes(5),
	nAlternatives(5),
	nSuccessesForExclusion(3),
	nTries(4),
	nTriesCounter(0)
{
	srand(time(NULL)%RAND_MAX); // устанавливаем "отправную точку" генерирования последовательности случайных чисел

	requestLabel = new QLabel(this);
	alternativesBox = new QComboBox(this);
	alternativesBox->setEditable(false);
	//alternativesBox->setDuplicatesEnabled(false); // запрещение добавлять из edit одинаковые строки (-пользователем-не программно)
	closeButton = new QPushButton(this);
	connect(closeButton, &QPushButton::clicked,
			this, &QWidget::close);
	okButton = new QPushButton(this);
	okButton->setText(tr("Ok"));
	okButton->setMaximumWidth(40);
	closeButton->setText(tr("Закрыть"));
	closeButton->setMaximumWidth(80);
	/*QVBoxLayout *vlayout1 = new QVBoxLayout;
	vlayout1->addWidget(requestLabel);
	vlayout1->addWidget(alternativesBox);
	QVBoxLayout *vlayout2 = new QVBoxLayout;
	vlayout2->addWidget(okButton);
	vlayout2->addWidget(closeButton);
	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->addLayout(vlayout1);
	hlayout->addLayout(vlayout2);
	setLayout(hlayout);*/
	QVBoxLayout *vlayout = new QVBoxLayout;
	vlayout->setSpacing(10);
	vlayout->addWidget(requestLabel);
	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->addWidget(alternativesBox);
	hlayout->addWidget(okButton);
	vlayout->addLayout(hlayout);
	vlayout->addStretch();
	vlayout->addWidget(closeButton, 0, Qt::AlignRight);
	setLayout(vlayout);
	QRect screenRect = QGuiApplication::primaryScreen()->geometry();
	int wdt = 300;
	int hgt = sizeHint().height();
	setGeometry( (screenRect.width()-wdt)/2, (screenRect.height()-hgt)/2, wdt, hgt);

	//connect(alternativesBox, &QComboBox::activated,
		//	this, &TranslationDialog::alternativeChoosen);
	connect(alternativesBox, SIGNAL(activated(int)),
			this, SLOT(alternativeChoosen(int)));
	connect(okButton, &QPushButton::clicked,
			this, &TranslationDialog::nextTranslationRequest);
}

/*void TranslationDialog::open() {
	if (prepareTranslationRequest())
		QDialog::open();
}*/
void TranslationDialog::showEvent(QShowEvent *e) {
	prepareTranslationRequest();

	QWidget::showEvent(e);
}
void TranslationDialog::closeEvent(QCloseEvent *e) {
	mRequestTrItem = nullptr;
	nTriesCounter = 0;
	mComboboxCorrectAlternativeIndex = -1;
	//QDialog::closeEvent(e);
	QWidget::closeEvent(e);
}
void TranslationDialog::nextTranslationRequest() {
	if (nTriesCounter >= nTries) {
		close();
		return;
	}
	prepareTranslationRequest();
}
void TranslationDialog::alternativeChoosen(int index) {
	++ nTriesCounter;
	if (index == mComboboxCorrectAlternativeIndex)
	{ // правильный выбор перевода в combo box
		Q_ASSERT(nullptr != mRequestTrItem);
		mRequestTrItem->incrSuccessCounter();
		if (mRequestTrItem->successCounter() >= nSuccessesForExclusion) {
			emit excludeTranslation(mRequestTrItem);
		}
		if (nTriesCounter >= nTries)
		{
			close();
			return;
		}
		if (! prepareTranslationRequest())
		{
			close();
			return;
		}
	}
	else {
		alternativesBox->setCurrentIndex(mComboboxCorrectAlternativeIndex);
		//alternativesBox->setCurrentIndex(-1);
		alternativesBox->setEnabled(false);
		okButton->setEnabled(true);
	}
}
void TranslationDialog::setSuccessesForExclusion(int num) {
	for (TranslationItem *tip: *mTrItemsVPtr) {
		if (tip->successCounter() >= num)
			emit excludeTranslation(tip);
	}
	nSuccessesForExclusion = num;
}
bool TranslationDialog::checkReady() {
	Q_ASSERT(nullptr != mTrItemsVPtr);
	if (nullptr == mTrItemsVPtr) {
		QMessageBox::critical(nullptr, tr("TranslationDialog"), tr("Отсутствуют данные переводов."));
		return false;
	}
	// формируем список возможных ответов (с правильным ответом)
	QStringList answerAlternatives; QString altText="";
	for (const TranslationItem *tip: *mTrItemsVPtr) {
		if (!tip->isExcluded()) {
			answerAlternatives << (tip->isInvert()?tip->firstExpr():tip->secondExpr());
		}
	}
	answerAlternatives.removeDuplicates();
	if (answerAlternatives.count()-1 < 1) {// альтернатив без правильной должно быть >= 1
	// недостаточно альтернатив перевода
		QMessageBox::warning(nullptr, tr("TranslationDialog"), tr("Недостаточно альтернатив перевода."));
		return false;
	}

	return true;
}
bool TranslationDialog::prepareTranslationRequest()
{
	Q_ASSERT(nullptr != mTrItemsVPtr);
	if (nullptr == mTrItemsVPtr) {
		//if (verbal)
		//	QMessageBox::warning(nullptr, tr("TranslationDialog"), tr("Отсутствуют данные переводов."));
		return false;
	}

	// Подготавливаем диалог к выдаче запроса переводам
	// отбираем неисключенные переводы в отдельный массив
	QVector<TranslationItem *> workTrItemsv;
	for (const TranslationItem * tip: *mTrItemsVPtr) {
		if (!tip->isExcluded())
			workTrItemsv.append(const_cast<TranslationItem*>(tip));
	}
	if (workTrItemsv.count() < 2)
	{
		//if (verbal)
		//	QMessageBox::warning(nullptr, tr("TranslationDialog"), tr("Недостаточно переводов для формирования диалога."));
		return false;
	}
	// формируем запрашиваемый перевод
	int index = rand() % workTrItemsv.count();
	mRequestTrItem = workTrItemsv[index];
	workTrItemsv.remove(index); // удаляем, чтобы он больше не выбирался во время выбора альтернатив
	QString correctAnswer = mRequestTrItem->isInvert()?mRequestTrItem->firstExpr():mRequestTrItem->secondExpr();
	requestLabel->setText(mRequestTrItem->isInvert()?mRequestTrItem->secondExpr():mRequestTrItem->firstExpr());

	// Заполняем combo box альтернативами прав перевода выражения
	if (alternativesBox->count() >0)
		alternativesBox->clear();
	//alternativesBox->insertItem(0, "lasdfkj"); // в combobox не видно изначально ниодного возможного перевода
	QString altText="";
	// формируем список возможных ответов без правильного ответа
	QStringList answerAlternatives;
	for (const TranslationItem *tip: workTrItemsv) {
		altText = tip->isInvert()?tip->firstExpr():tip->secondExpr();
		if (altText != correctAnswer)
			answerAlternatives << altText;
	}
	answerAlternatives.removeDuplicates();
	if (answerAlternatives.count() < 1) {
		//if (verbal)
		//	QMessageBox::warning(nullptr, tr("TranslationDialog"), tr("Недостаточно альтернатив перевода."));
		return false;
	}
	//
	// индекс правильного ответа в combobox
	//indexOfCorrectInCombobox = 1 + rand() % std::min(nAlternatives,answerAlternatives.count()+1);
	mComboboxCorrectAlternativeIndex = rand() % std::min(nAlternatives,answerAlternatives.count()+1);

	//int ind = 1;
	int ind = 0;
	while ( (ind <nAlternatives && !answerAlternatives.isEmpty())
		   || (ind==mComboboxCorrectAlternativeIndex && answerAlternatives.isEmpty()) ) {
		if (ind == mComboboxCorrectAlternativeIndex) {
			altText = correctAnswer;
		}
		else {
			// случайно выбираем возможный ответ и списка возможных ответов
			int altIndex = rand() % answerAlternatives.count();
			QStringList::const_iterator it = answerAlternatives.begin();
			for (int i=0; i<altIndex; i++)
				++ it;
			altText = *it;
			answerAlternatives.removeAt(altIndex); // удаляем, чтобы он больше не выбирался
		}

		alternativesBox->insertItem(ind, altText);
		++ind;
	}
	alternativesBox->setEnabled(true);
	alternativesBox->setCurrentIndex(-1);
	okButton->setEnabled(false);

	return true;
}
