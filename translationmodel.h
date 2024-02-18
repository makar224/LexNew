#ifndef TRANSLATIONMODEL_H
#define TRANSLATIONMODEL_H

#include <iostream>
using std::istream;
using std::ostream;
#include <QString>
#include <QPair>

class TranslationItem {
public:
	TranslationItem();
	TranslationItem(const QString &expr1, const QString &expr2, bool excl=false, bool inv=false);
	int successCounter() {return nSuccessCounter;}
	void setInvert(bool b) {invert = b;}
	void setExcluded(bool b) {excluded = b;}
	bool isInvert() const {return invert;}
	bool isExcluded() const {return excluded;}
	void setFirstExpr(const QString &str) {
		exprs.first = str;
	}
	void setSecondExpr(const QString &str) {
		exprs.second = str;
	}
	QString firstExpr() const {return exprs.first;}
	QString secondExpr() const {return exprs.second;}
	void incrSuccessCounter() {++nSuccessCounter;}
	void resetSuccessCounter() {nSuccessCounter=0;}

	friend istream& operator>>(istream&, TranslationItem&);
	friend ostream& operator<<(ostream&, const TranslationItem&);

private:
	QPair<QString, QString> exprs;
	bool invert;
	bool excluded;
	int nSuccessCounter;
};

//Q_DECLARE_METATYPE(TranslationItem*)

#endif // TRANSLATIONMODEL_H
