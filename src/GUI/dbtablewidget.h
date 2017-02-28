#ifndef DBTABLEWIDGET_H
#define DBTABLEWIDGET_H

#include <QTableWidget>
#include <QPushButton>
#include "DB/dbtable.h"

class DBTableWidgetColumn;

class DBTableWidget: public QTableWidget
{

Q_OBJECT

public:
    DBTableWidget();

    DBTableWidgetColumn* column(QString name);
    DBTableWidgetColumn* column(int id);
    const DBDataStructure& structure() const {return _structure;}
    
public slots:
    void addRow(SDBElement e=0);
    void readFromTable(DBBaseTable* table=0);
    void reset();
    void apply();
    
    void readAllDefinitions();
    void readDefinition(int id);
    
protected slots:
    void itemHadChanged(QTableWidgetItem* item);
    void onSelectionChanged();
    void resetUI();
    
protected:
    DBBaseTable* _table=0;
    QList<DBTableWidgetColumn*> _columnDefinitions;
    DBDataStructure _structure;

    void initUI();
};

class DBTableWidgetColumn: public QObject{

Q_OBJECT

public:
    DBTableWidgetColumn(DBTableWidget* parentWidget, int id);

    enum Type{
        STRING,
        DOUBLE,
        INTEGER,
        BOOLEAN,
        EXTERN_KEY
    };

    int             id()    const {return _id;}
    int             width() const {return _width;}
    const QString&  title() const {return _title;}
    const QString&  placeHolder() const {return _placeHolder;}
    const Type&     type()  const {return _type;}
    bool    isNullable()    const {return _nullable;}
    bool    isNumber()      const {return _type==INTEGER || _type==DOUBLE;}
    int     maxCharacter()  const {return _maxCharacter;}
    DBBaseTable* externalTable() const {return _externalTable;}

    void setTitle(const QString& title);
    void setPlaceHolder(const QString& placeHolder);
    void setWidth(int width);

    bool isHidden() const {return _hidden;}
    void setHidden(bool hide);
    
    const QString& suffix() const {return _suffix;}
    void setSuffix(const QString& suffix);

protected:
    DBTableWidget* _parentWidget;
    int _id;
    bool _hidden=false;
    int _width = -1;
    QString _title;
    QString _placeHolder;
    QString _suffix;
    
    Type _type;
    bool _nullable;
    int _maxCharacter = -1;
    DBBaseTable* _externalTable = 0;

    void reportHeaderChange();
    void reportColumnChange();
};


class DBTableWidgetDelegate: public QItemDelegate{
public:
    DBTableWidgetDelegate(DBTableWidget *parent);
    
protected:
    DBTableWidget* _parent;
    
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
};


class DBTableEditor: public QWidget{
Q_OBJECT
    
    DBTableWidget* _tableWidget;
    QPushButton* _bAdd, *_bRemove;

public: 
    DBTableEditor();
    DBTableWidget* tableW() const {return _tableWidget;}
public slots:
    void readFromTable(DBBaseTable* table=0);
    void apply();
    void reset();

};

#endif // DBTABLEWIDGET_H
