#ifndef TRANSLATIONMODEL_H
#define TRANSLATIONMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QPair>

class TranslationItem {
public:
	//TranslationItem():invert(false), excluded(false) {}
	TranslationItem(const QString &expr1, const QString &expr2, bool excl=false, bool inv=false):
	exprs(QPair<QString, QString>(expr1, expr2)),
	invert(inv),
	excluded(excl),
	_successCounter(0)
	{}
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

Q_DECLARE_METATYPE(TranslationItem*)

class TranslationModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TranslationModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	//bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	//bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	//bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

private:
	QVector<TranslationItem*> items;
};

#endif // TRANSLATIONMODEL_H
