#include "translationdialog.h"
#include "time.h"
#include <QGuiApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QShowEvent>

TranslationDialog::TranslationDialog(QWidget *parent, QList<TranslationItem*>* tilp) :
	QWidget(parent),
	mTrItemsLPtr(tilp),
	mRequestTrItem(nullptr),
	nSessionInterval(5),
	nAlternatives(5),
	nSuccessesForExclusion(3),
	nTries(4),
	nTriesCounter(0)
{
	srand(time(NULL)%RAND_MAX); // устанавливаем "отправную точку" генерирования последовательности случайных чисел

	requestLabel = new QLabel(this);
	requestLabel->setAlignment(Qt::AlignCenter);
	requestLabel->setStyleSheet("QLabel {"
								"background-color: yellow;"
								//"font-weight: bold; font-size: 13px;"
								"font-size: 14px;"
								"border-style: solid; border-width: 2px; border-color: black;"
								"}"
								"QToolTip {"
								"background-color: black;"
								"font-size: 14px;"
								//"font-color: white;"
								"color: white;"
								"}"
								);
	//requestLabel->setFrameStyle(QFrame::Box|QFrame::Plain);
	mAlternativesBox = new QComboBox(this);
	mAlternativesBox->setEditable(false);
	mAlternativesBox->setFixedHeight(30);
	closeButton = new QPushButton(this);
	connect(closeButton, &QPushButton::clicked,
			this, &QWidget::close);
	okButton = new QPushButton(this);
	okButton->setText(tr("Ok"));
	okButton->setMaximumWidth(40);
	okButton->setAutoDefault(true);
	closeButton->setText(tr("Закрыть"));
	closeButton->setMaximumWidth(80);
	QVBoxLayout *vlayout = new QVBoxLayout;
	vlayout->setSpacing(0);
	vlayout->addWidget(requestLabel);
	vlayout->addSpacing(10);
	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->addWidget(mAlternativesBox);
	hlayout->addSpacing(10);
	hlayout->addWidget(okButton);
	vlayout->addLayout(hlayout);
	vlayout->addSpacing(closeButton->height());
	vlayout->addWidget(closeButton, 0, Qt::AlignRight);
	setLayout(vlayout);
	QRect screenRect = QGuiApplication::primaryScreen()->geometry();
	int wdt = 320;
	int hgt = sizeHint().height();
	setGeometry( (screenRect.width()-wdt)/2, (screenRect.height()-hgt)/2, wdt, hgt);
	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
	setWindowTitle(tr("Переводы"));

	//connect(alternativesBox, &QComboBox::activated,
		//	this, &TranslationDialog::alternativeChoosen);
	connect(mAlternativesBox, SIGNAL(activated(int)),
			this, SLOT(alternativeChoosen(int)));
	connect(okButton, &QPushButton::clicked,
			this, &TranslationDialog::requestAfterWrongAlternativeChoosen);
}
void TranslationDialog::resizeEvent(QResizeEvent *e) {
	requestLabel->setFixedSize(mAlternativesBox->width(), 40);
	QWidget::resizeEvent(e);
}
void TranslationDialog::keyPressEvent(QKeyEvent *event) {
	int key = event->key();
	if (Qt::Key_Return == key) {
		if (mAlternativesBox->isEnabled()) {
			mAlternativesBox->setCurrentIndex(0);
			mAlternativesBox->showPopup();
			return;
		}
	}
	else if (Qt::Key_Escape == key) {
		close();
	}
	QWidget::keyPressEvent(event);
}
void TranslationDialog::requestAfterWrongAlternativeChoosen() {
	if (nTriesCounter >= nTries) {
		nTriesCounter = 0;
		close();
		return;
	}
	prepareTranslationRequest();
}
void TranslationDialog::alternativeChoosen(int index) {
	++ nTriesCounter;
	if (index == mnComboboxCorrectAlternativeIndex)
	{ // правильный выбор перевода в combo box
		Q_ASSERT(nullptr != mRequestTrItem);
		mRequestTrItem->incrSuccessCounter();
		if (mRequestTrItem->successCounter() >= nSuccessesForExclusion) {
			emit excludeTranslation(mRequestTrItem);
		}
		if (nTriesCounter >= nTries)
		{
			nTriesCounter = 0;
			close();
			return;
		}
		if (! prepareTranslationRequest())
		{
			hide();
			return;
		}
	}
	else {
		mRequestTrItem->resetSuccessCounter();
		mAlternativesBox->setCurrentIndex(mnComboboxCorrectAlternativeIndex);
		//alternativesBox->setCurrentIndex(-1);
		mAlternativesBox->setEnabled(false);
		okButton->setEnabled(true);
		okButton->setFocus();
	}
}
void TranslationDialog::setSuccessesForExclusion(int num) {
	// Те TrItem-ы, в которых счетчики превышают заданный уровень - исключаются
	//for (TranslationItem *tip: *mTrItemsLPtr) {
	//	if (tip->successCounter() >= num)
	//		emit excludeTranslation(tip);
	//} // закомментировано - здесь - не исключаются - будут исключаться сразу после попадания в запрос перевода
	nSuccessesForExclusion = num;
}
// Подготавливаем диалог к выдаче запроса перевода
bool TranslationDialog::prepareTranslationRequest()
{
	Q_ASSERT(nullptr != mTrItemsLPtr);
	if (nullptr == mTrItemsLPtr) {
		QMessageBox::warning(nullptr, tr("TranslationDialog"), tr("Отсутствуют данные переводов."));
		return false;
	}

	// отбираем неисключенные переводы в отдельный массив
	QVector<TranslationItem *> workTrItemsv;
	for (const TranslationItem * tip: *mTrItemsLPtr) {
		if (!tip->isExcluded())
			workTrItemsv.append(const_cast<TranslationItem*>(tip));
	}
	if (workTrItemsv.count() < 2)
	{
		QMessageBox::warning(nullptr, tr("TranslationDialog"), tr("Недостаточно переводов для формирования диалога."));
		return false;
	}
	// формируем запрашиваемый перевод
	int index = 0;
	do { // исключаем повторение одного и того же запроса
		index = rand() % workTrItemsv.count();
	} while (workTrItemsv[index] == mRequestTrItem);
	mRequestTrItem = workTrItemsv[index];

	workTrItemsv.remove(index); // удаляем, чтобы он больше не выбирался во время выбора альтернатив
	bool trdir = (bool)(rand() % 2); // задаем направление переводов

	QString correctAnswer = trdir?mRequestTrItem->firstExpr():mRequestTrItem->secondExpr();
	QString requestText = trdir?mRequestTrItem->secondExpr():mRequestTrItem->firstExpr();
	requestLabel->setText(requestText);
	requestLabel->setToolTip(requestText);

	// Заполняем combo box альтернативами прав перевода выражения
	if (mAlternativesBox->count() >0)
		mAlternativesBox->clear();
	QString altText="";
	// формируем список возможных ответов без правильного ответа
	QStringList answerAlternatives;
	for (const TranslationItem *tip: workTrItemsv) {
		altText = trdir?tip->firstExpr():tip->secondExpr();
		if (altText != correctAnswer)
			answerAlternatives << altText;
	}
	answerAlternatives.removeDuplicates();
	if (answerAlternatives.count() < 1) {
		QMessageBox::warning(nullptr, tr("TranslationDialog"), tr("Недостаточно альтернатив перевода."));
		return false;
	}


    mnComboboxCorrectAlternativeIndex = rand() % std::min(nAlternatives,(int)answerAlternatives.count()+1);
	int ind = 0;
	while ( (ind <nAlternatives && !answerAlternatives.isEmpty())
		   || (ind==mnComboboxCorrectAlternativeIndex && answerAlternatives.isEmpty()) ) {
		if (ind == mnComboboxCorrectAlternativeIndex) {
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

		mAlternativesBox->insertItem(ind, altText);
		++ind;
	}
	mAlternativesBox->setEnabled(true);
	mAlternativesBox->setCurrentIndex(-1);
	//mAlternativesBox->setFocus();
	okButton->setEnabled(false);

	return true;
}
