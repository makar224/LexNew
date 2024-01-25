#include "translationmodel.h"

TranslationItem::TranslationItem(const QString &expr1, const QString &expr2, bool excl, bool inv):
exprs(QPair<QString, QString>(expr1, expr2)),
invert(inv),
excluded(excl),
nSuccessCounter(0)
{}
