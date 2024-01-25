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
	explicit TranslationDialog(QWidget *parent = nullptr, QVector<TranslationItem*>* tivp=nullptr);
	~TranslationDialog() {}
	void setTranslations(QVector<TranslationItem*>* vtip) {
		mTrItemsVPtr = vtip;
	}
	int sessionInterval() {return nSessionInterval;}
	int successesForExclusion() {return nSuccessesForExclusion;}
	int alternativesNumber() {return nAlternatives;}
	int triesNumber() {return nTries;}

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
	QComboBox *alternativesBox;
	QPushButton *closeButton;
	QPushButton *okButton;
	QVector<TranslationItem*>* mTrItemsVPtr;

	int mnComboboxCorrectAlternativeIndex; // индекс правильного ответа в combobox
	TranslationItem *mRequestTrItem;

	int nSessionInterval;
	int nAlternatives;
	int nSuccessesForExclusion;
	int nTries;
	int nTriesCounter;
};

#endif // TRANSLATIONDIALOG_H
