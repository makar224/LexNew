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
	//void done(int result);
	void setupTables(const QVector<TranslationItem*> &v);
public slots:
	void addTranslation(const TranslationItem *tip);
	void removeTranslation(const TranslationItem *tip);
	void editTranslation(const TranslationItem *tip);
	//void updateTables(const TranslationItem *tip);
	void excludeTranslation(TranslationItem *tip);
protected slots:
	//void currentTableCurrentCellChanged(int,int,int,int);
	//void currentTableItemSelectionChanged();

	void tableItemSelectionChanged();
	void changeDirection();
	void moveTranslation();

private:
	void setTableItemRow(QTableWidget *widg, int row, const TranslationItem *tip);
	bool setupTableItemRow(QTableWidget *widg, int row, const TranslationItem *tip);
	TranslationItem *getRowTIFromData(const QTableWidget *widg, int row) const;
	bool setRowTIData(QTableWidget *widg, int row, const TranslationItem *item);

	QTableWidget *currentTableWidget;
	QTableWidget *exclusionTableWidget;
	//QLabel *currentTableLabel;
	//QLabel *exclusionTableLabel;
	QPushButton *moveButton;
	QPushButton *changeDirButton;
	//QPushButton *restoreAllButton;
	QPushButton *closeButton;
};


#endif // MOVETRANSLATIONSDIALOG_H
