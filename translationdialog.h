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
	int sessionInterval() {return nSessionMinutes;}
	int successesForExclusion() {return nSuccessesForExclusion;}
	int alternativesNumber() {return nAlternatives;}
	int triesNumber() {return nTries;}

	void setSessionMinutes(int num) { nSessionMinutes = num;}
	void setSuccessesForExclusion(int num);
	void setAlternativesNumber(int num) { nAlternatives = num; }
	void setTriesNumber(int num) { nTries = num; }

	bool checkReady();

public slots:
	//void open() override;
protected slots:
	void alternativeChoosen(int index);
private slots:
	bool prepareTranslationRequest();
signals:
	void excludeTranslation(TranslationItem *tip);
protected:
	void showEvent(QShowEvent *event) override;
	void closeEvent(QCloseEvent *) override;
private:
	void nextTranslationRequest();

	QLabel *requestLabel;
	QComboBox *alternativesBox;
	QPushButton *closeButton;
	QPushButton *okButton;
	QVector<TranslationItem*>* mTrItemsVPtr;

	int mComboboxCorrectAlternativeIndex;
	TranslationItem *mRequestTrItem;

	int nSessionMinutes;
	int nAlternatives;
	int nSuccessesForExclusion;
	int nTries;
	int nTriesCounter;
};

#endif // TRANSLATIONDIALOG_H
