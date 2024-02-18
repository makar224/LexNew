#ifndef DICTIONARYEDITDIALOG_H
#define DICTIONARYEDITDIALOG_H

#include "translationmodel.h"
#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>


class DictionaryEditDialog : public QDialog
{
	Q_OBJECT
public:
	explicit DictionaryEditDialog(QWidget *parent = nullptr);
	void clearTable();
	void setupTable(const QList<TranslationItem*> &l);
protected:
	bool eventFilter(QObject *, QEvent *) override;
protected slots:
	void tableItemSelectionChanged();
	void addTranslation();
	void removeTranslation();
	void editTranslation(int row, int col);
signals:
	void addTranslationSig(const TranslationItem *);
	void removeTranslationSig(const TranslationItem *item);
	void editTranslationSig(const TranslationItem *);
private:
	bool setupTableItemRow(int row, const TranslationItem *tip);
	TranslationItem *getRowTIFromData(int row) const;

	QTableWidget *tableWidget;
	QPushButton *addButton;
	QPushButton *removeButton;
	QLineEdit *expr1Edit;
	QLineEdit *expr2Edit;
	QPushButton *closeButton;
};

#endif // DICTIONARYEDITDIALOG_H
