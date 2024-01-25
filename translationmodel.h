#ifndef TRANSLATIONMODEL_H
#define TRANSLATIONMODEL_H

#include <QString>
#include <QPair>

class TranslationItem {
public:
	//TranslationItem():invert(false), excluded(false) {}
	TranslationItem(const QString &expr1, const QString &expr2, bool excl=false, bool inv=false);
	int successCounter() {return _successCounter;}
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
	void incrSuccessCounter() {++_successCounter;}
	void resetSuccessCounter() {_successCounter=0;}

private:
	QPair<QString, QString> exprs;
	bool invert;
	bool excluded;
	int _successCounter;
};

//Q_DECLARE_METATYPE(TranslationItem*)

#endif // TRANSLATIONMODEL_H
