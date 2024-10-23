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
	int successesForExclusion() {return m_nSuccessesForExclusion;}
	int alternativesNumber() {return m_nAlternatives;}
	int triesNumber() {return m_nTries;}
	QComboBox* alternativesBox() const {return mAlternativesBox;}

	void setSessionInterval(int num) { nSessionInterval = num;}
	void setSuccessesForExclusion(int num);
	void setAlternativesNumber(int num) { m_nAlternatives = num; }
	void setTriesNumber(int num) { m_nTries = num; }

	bool prepareTranslationRequest();
protected:
	void resizeEvent(QResizeEvent *) override;
	//void closeEvent(QCloseEvent *) override;
	void keyPressEvent(QKeyEvent *) override;
protected slots:
	void alternativeChoosen(int index);
	void requestAfterWrongAlternativeChoosen();
signals:
	void excludeTranslation(TranslationItem *tip);

private:
	QLabel *requestLabel;
	QComboBox *mAlternativesBox;
	QPushButton *closeButton;
	QPushButton *okButton;
	QList<TranslationItem*>* mTrItemsLPtr;

	int m_nComboboxCorrectAlternativeIndex; // индекс правильного ответа в combobox
	TranslationItem *mRequestTrItem;

	int nSessionInterval;
	int m_nAlternatives;
	int m_nSuccessesForExclusion;
	int m_nTries;
	int m_nTriesCounter;
};

#endif // TRANSLATIONDIALOG_H
