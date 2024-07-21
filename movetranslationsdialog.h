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
	void clearTables();
	void setupTables(const QList<TranslationItem*> &l);
public slots:
	void addTranslation(const TranslationItem *tip);
	void removeTranslation(const TranslationItem *tip);
	void editTranslation(const TranslationItem *tip);
	void excludeTranslation(TranslationItem *tip);
protected slots:
	void tableItemSelectionChanged();
	void moveTranslation();
	void restoreAllTranslations();
protected:
	void resizeEvent(QResizeEvent *) override;
private:
	void setTableItemRow(QTableWidget *widg, int row, const TranslationItem *tip);
	bool setupTableItemRow(QTableWidget *widg, int row, const TranslationItem *tip);
	TranslationItem *getRowTIFromData(const QTableWidget *widg, int row) const;
	bool setRowTIData(QTableWidget *widg, int row, const TranslationItem *item);

	QTableWidget *currentTableWidget;
	QTableWidget *exclusionTableWidget;
	QPushButton *moveButton;
	QPushButton *restoreAllButton;
	QPushButton *closeButton;
};


#endif // MOVETRANSLATIONSDIALOG_H
