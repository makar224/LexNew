#ifndef MOVETRANSLATIONSDIALOG_H
#define MOVETRANSLATIONSDIALOG_H

#include "translationmodel.h"
#include <QDialog>
#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QMutex>

class MoveTranslationsDialog: public QDialog
{
	Q_OBJECT

public:
	explicit MoveTranslationsDialog(QWidget *parent=nullptr);
	~MoveTranslationsDialog();
	void setupTables(const QVector<TranslationItem*> &v);
public slots:
	void addTranslation(const TranslationItem *tip);
	void removeTranslation(const TranslationItem *tip);
	void editTranslation(const TranslationItem *tip);
	void excludeTranslation(TranslationItem *tip);
protected slots:
	void tableItemSelectionChanged();
	void changeDirection();
	void moveTranslation();
	void restoreAllTranslations();

private:
	void setTableItemRow(QTableWidget *widg, int row, const TranslationItem *tip);
	bool setupTableItemRow(QTableWidget *widg, int row, const TranslationItem *tip);
	TranslationItem *getRowTIFromData(const QTableWidget *widg, int row) const;
	bool setRowTIData(QTableWidget *widg, int row, const TranslationItem *item);

	QTableWidget *currentTableWidget;
	QTableWidget *exclusionTableWidget;
	QPushButton *moveButton;
	QPushButton *changeDirButton;
	QPushButton *restoreAllButton;
	QPushButton *closeButton;
};


#endif // MOVETRANSLATIONSDIALOG_H
