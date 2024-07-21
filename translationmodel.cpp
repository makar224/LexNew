#include <iostream>
#include <iomanip>
#include <string>
using std::string;
using std::ios;
#include "translationmodel.h"

TranslationItem::TranslationItem():excluded(false) {}

TranslationItem::TranslationItem(const QString &expr1, const QString &expr2, bool excl):
exprs(QPair<QString, QString>(expr1, expr2)),
excluded(excl),
nSuccessCounter(0)
{}

// ...Обязательно в такой последовательности (счетчик идет после флагов) должны записываться и извлекаться данные.
// Иначе eof возвращает false после чтения последнего bool и цикл чтения не заканчивается и происходит fail по след чтении...
istream& operator>>(istream& is, TranslationItem& ti) {
	char exprA[128] = {0};
	is.getline(exprA, 128); ti.exprs.first = QString::fromUtf8(exprA);
	is.getline(exprA, 128); ti.exprs.second = QString::fromUtf8(exprA);
	is >> std::boolalpha >> ti.excluded;
	is >> ti.nSuccessCounter;

	return is;
}
ostream& operator<<(ostream& os, const TranslationItem& ti) {
	os << ti.exprs.first.toStdString() << "\n"
		<< ti.exprs.second.toStdString() << "\n"
		<< std::boolalpha<<ti.excluded
		<< ti.nSuccessCounter;
	return os;
}
