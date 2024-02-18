#include <QDebug>
#include <iostream>
#include <iomanip>
#include <string>
using std::string;
using std::ios;
#include "translationmodel.h"

TranslationItem::TranslationItem():invert(false), excluded(false) {}

TranslationItem::TranslationItem(const QString &expr1, const QString &expr2, bool excl, bool inv):
exprs(QPair<QString, QString>(expr1, expr2)),
invert(inv),
excluded(excl),
nSuccessCounter(0)
{}

/*istream& operator>>(istream& is, TranslationItem& ti) {
	string expr;
	is >> expr; ti.exprs.first = QString::fromStdString(expr);
	is >> expr; ti.exprs.second = QString::fromStdString(expr);
	is >> ti.excluded;
	is >> ti.invert;
	return is;
}
ostream& operator<<(ostream& os, const TranslationItem& ti) {
	os << ti.exprs.first.toStdString()
		<< ti.exprs.second.toStdString()
		<< ti.excluded
		<< ti.invert;
	return os;
}*/

// ...Обязательно в такой последовательности (счетчик идет после флагов) должны записываться и извлекаться данные.
// Иначе eof возвращает false после чтения последнего bool и цикл чтения не заканчивается и происходит fail по след чтении...
istream& operator>>(istream& is, TranslationItem& ti) {
	char exprA[128] = {0};
	is.getline(exprA, 128); ti.exprs.first = QString::fromUtf8(exprA);
	//if(!is)
	//	qDebug("is failed s1");
	is.getline(exprA, 128); ti.exprs.second = QString::fromUtf8(exprA);
	is >> std::boolalpha >> ti.excluded;
	is >> std::boolalpha >> ti.invert;
	/*bool b = false;
	is >> std::boolalpha >> b;
	//b = true;
	is >> std::boolalpha >> b;
	if (!is)
		qDebug("is failed, bad=%d, fail=%d", is.bad(), is.fail());*/

	is >> ti.nSuccessCounter;

	return is;
}
ostream& operator<<(ostream& os, const TranslationItem& ti) {
	//os << std::setw(128) << std::setiosflags(ios::left) << ti.exprs.first.toStdString()
	os << ti.exprs.first.toStdString() << "\n"
		<< ti.exprs.second.toStdString() << "\n"
		<< std::boolalpha<<ti.excluded
		<< std::boolalpha<<ti.invert
		<< ti.nSuccessCounter;
	return os;
}
