#ifndef TRANSLATIONDIALOG_H
#define TRANSLATIONDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include "translationmodel.h"

class TranslationDialog : public QWidget
{
	Q_OBJECT
public:
	explicit TranslationDialog(QWidget *parent = nullptr, QList<TranslationItem*>* tilp=nullptr);
	~TranslationDialog() {}
	void setTranslations(QList<TranslationItem*>* tipL) {
		mTrItemsLPtr = tipL;
	}
	int sessionInterval() {return nSessionInterval;}
	int successesForExclusion() {return nSuccessesForExclusion;}
	int alternativesNumber() {return nAlternatives;}
	int triesNumber() {return nTries;}
	QComboBox* alternativesBox() const {return mAlternativesBox;}

	void setSessionInterval(int num) { nSessionInterval = num;}
	void setSuccessesForExclusion(int num);
	void setAlternativesNumber(int num) { nAlternatives = num; }
	void setTriesNumber(int num) { nTries = num; }

	bool prepareTranslationRequest();

protected slots:
	void alternativeChoosen(int index);
	void requestAfterWrongAlternativeChoosen();
signals:
	void excludeTranslation(TranslationItem *tip);
protected:
	//void closeEvent(QCloseEvent *) override;

private:
	QLabel *requestLabel;
	QComboBox *mAlternativesBox;
	QPushButton *closeButton;
	QPushButton *okButton;
	QList<TranslationItem*>* mTrItemsLPtr;

	int mnComboboxCorrectAlternativeIndex; // индекс правильного ответа в combobox
	TranslationItem *mRequestTrItem;

	int nSessionInterval;
	int nAlternatives;
	int nSuccessesForExclusion;
	int nTries;
	int nTriesCounter;
};

#endif // TRANSLATIONDIALOG_H
