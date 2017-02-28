#include "dbtablewidget.h"

#include <QHeaderView>
#include <QTableWidgetItem>

#include "customwidget.h"

DBTableWidget::DBTableWidget()
    :QTableWidget()
{
    connect(this, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(itemHadChanged(QTableWidgetItem*)));
    setItemDelegate(new DBTableWidgetDelegate(this));
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(onSelectionChanged()));
}

void DBTableWidget::initUI(){
    resetUI();
    
    _structure = _table->dataStructure();
    for(int i=0; i<_structure.size(true); i++){
        DBTableWidgetColumn* c = new DBTableWidgetColumn(this, i);
        _columnDefinitions.append(c);
    }
    setColumnCount(_columnDefinitions.size());
    _columnDefinitions.at(0)->setHidden(true);
    
    readAllDefinitions();
}

void DBTableWidget::resetUI()
{
    setRowCount(0);
}

DBTableWidgetColumn *DBTableWidget::column(QString name)
{
    for(int i=0; i<_columnDefinitions.size(); i++){
        if(_columnDefinitions.at(i)->title() == name)
            return _columnDefinitions.at(i);
    }
    return 0;
}

DBTableWidgetColumn *DBTableWidget::column(int id)
{
    if(id>0 && id<_columnDefinitions.size())
        return _columnDefinitions.at(id);
    return 0;
}

void DBTableWidget::addRow(SDBElement e)
{
    if(!e)
        e = _table->addNewElement();
    
    int row = rowCount();
    insertRow(row);
    
    for(int i=0; i<_columnDefinitions.size(); i++){
        DBTableWidgetColumn* c = _columnDefinitions.at(i);
        QTableWidgetItem* item = new QTableWidgetItem();
        if(c->type()==DBTableWidgetColumn::BOOLEAN){
            item->setCheckState(e->value(i,true).toBool()?Qt::Checked:Qt::Unchecked);
            item->setFlags(item->flags()|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
            item->setCheckState(e->value(i,true).toBool()?Qt::Checked:Qt::Unchecked);
        }else{
            item->setFlags(item->flags()|Qt::ItemIsEditable|Qt::ItemIsEnabled);
            if(c->type()==DBTableWidgetColumn::EXTERN_KEY){
                item->setText(e->value(i,true).value<SDBElement>()->toString());
                QVariant v; v.setValue(e->value(i,true).value<SDBElement>());
                item->setData(Qt::UserRole, v);
            }else{
                item->setText(e->value(i,true).toString() + c->suffix());
                item->setData(Qt::UserRole, e->value(i,true).toString());
            }
        }
        setItem(row, i, item);
    }
}

void DBTableWidget::readFromTable(DBBaseTable *table)
{
    if(table && table!=_table){
        _table = table;
        initUI();
    }else
        resetUI();

    foreach (SDBElement e, _table->list())
        addRow(e);
}

void DBTableWidget::apply()
{
    if(_table)
        _table->applyCache();
}

void DBTableWidget::reset()
{
    if(_table)
        _table->resetCache();
    resetUI();
}

void DBTableWidget::itemHadChanged(QTableWidgetItem *it)
{
    DBTableWidgetColumn* c = _columnDefinitions.at(it->column());
    SDBElement e = _table->at(item(it->row(),0)->data(Qt::DisplayRole).toInt());
    if(!e)
        return;

    if(c->type()==DBTableWidgetColumn::BOOLEAN)
        e->setValue(it->checkState()!=Qt::Unchecked, it->column()-1);
    else
        e->setValue(it->data(Qt::UserRole), it->column()-1);
}

void DBTableWidget::onSelectionChanged()
{

}

void DBTableWidget::readAllDefinitions()
{
    for(int i=0; i<_columnDefinitions.size(); i++)
        readDefinition(i);
}

void DBTableWidget::readDefinition(int id)
{
    if(id<0 || id>_columnDefinitions.size())
        return;
    DBTableWidgetColumn* c = _columnDefinitions.at(id);
    QHeaderView* h = horizontalHeader();
    
    if(c->isHidden()){
        h->hideSection(id);
        return;
    }else
        h->showSection(id);


    if(!horizontalHeaderItem(id) || horizontalHeaderItem(id)->text() != c->title())
        setHorizontalHeaderItem(id, new QTableWidgetItem(c->title()));
    
    if(c->width()==-1)
        h->setSectionResizeMode(id, QHeaderView::Stretch);
    else{
        h->setSectionResizeMode(id, QHeaderView::Fixed);
        h->resizeSection(id, c->width());
    }
}

/*********************************************
 *             DBTableWidgetColumn           *
 *********************************************/
DBTableWidgetColumn::DBTableWidgetColumn(DBTableWidget *parentWidget, int id)
    :QObject(parentWidget), _parentWidget(parentWidget), _id(id)
{
    const DBDataStructure& s = parentWidget->structure();

    _title = s.fieldsName(true).at(id);
    QStringList definition = s.definitions(true).at(id).split(' ');
    const DBConstraint* c = s.constraintByName(_title);

    QString type = definition.at(1);
    if(type.startsWith("VARCHAR") || type.startsWith("TEXT")){
        _type = STRING;
        _maxCharacter = type.left(type.indexOf(')')).mid(type.indexOf('(')+1).toInt();
    }else if(type == "DOUBLE"){
        _type = DOUBLE;
    }else if(c){
        _type = EXTERN_KEY;
        _externalTable = c->foreignTable();
    }else if(type == "INTEGER"){
        _type = INTEGER;
    }else if(type == "BOOLEAN"){
        _type = BOOLEAN;
    }

    _nullable = definition.last()=="NULL" && definition.at(definition.size()-2)=="NOT";
}

void DBTableWidgetColumn::setTitle(const QString &title)
{
    if(title==_title)
        return;
    _title = title;
    reportHeaderChange();
}

void DBTableWidgetColumn::setPlaceHolder(const QString &placeHolder)
{
    if (_placeHolder == placeHolder)
        return;
    _placeHolder = placeHolder;
    reportColumnChange();
}

void DBTableWidgetColumn::setWidth(int width)
{
    if(width == _width)
        return;
    _width = width;
    reportHeaderChange();
}

void DBTableWidgetColumn::setHidden(bool hide)
{
    if(hide == _hidden)
        return;
    _hidden = hide;
    reportHeaderChange();
}

void DBTableWidgetColumn::setSuffix(const QString &suffix)
{
    if(suffix == _suffix)
        return;
    _suffix = suffix;
    reportColumnChange();
}

void DBTableWidgetColumn::reportHeaderChange()
{
    _parentWidget->readDefinition(_id);
}

void DBTableWidgetColumn::reportColumnChange()
{
    _parentWidget->readFromTable();
}

/*********************************************
 *            DBTableWidgetDelegate          *
 *********************************************/
DBTableWidgetDelegate::DBTableWidgetDelegate(DBTableWidget *parent)
    : QItemDelegate(parent), _parent(parent)
{
}

QWidget *DBTableWidgetDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    DBTableWidgetColumn* c = _parent->column(index.column());
    if(c->isNumber()){
        GSpinBox* w = new GSpinBox(parent);
        if(c->type() == DBTableWidgetColumn::INTEGER)
            w->setDoubleEdit(false);
        w->setSuffix(c->suffix());
        return w;
        
    }else if(c->type()==DBTableWidgetColumn::STRING){
        GLineEdit* w = new GLineEdit(parent);
        w->setMaxLength(c->maxCharacter());
        return w;
        
    }else if(c->type()==DBTableWidgetColumn::EXTERN_KEY){
        QComboBox* w = new QComboBox(parent);
        foreach(SDBElement e, c->externalTable()->list()){
            QVariant elem; elem.setValue(e);
            QVariant descriptor = e->shortDescriptor();
            if(descriptor.canConvert<QIcon>())
                w->addItem(descriptor.value<QIcon>(), "", elem);
            else if(descriptor.type() == QVariant::String)
                w->addItem(descriptor.toString(), elem);
            else
                w->addItem(e->toString(), elem);
        }
        return w;
    }
    
    return QItemDelegate::createEditor(parent, option, index);
}

void DBTableWidgetDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    DBTableWidgetColumn* c = _parent->column(index.column());
    QVariant data = _parent->model()->itemData(index).value(Qt::DisplayRole);
    if(c->isNumber()){
        GSpinBox* w = dynamic_cast<GSpinBox*>(editor);
        if(w)
            w->setValue(data.toDouble());
    }else if(c->type()==DBTableWidgetColumn::STRING){
        GLineEdit* w = dynamic_cast<GLineEdit*>(editor);
        if(w){
            QString s = data.toString();
            if(s.endsWith(c->suffix()))
                s.remove(s.length()-c->suffix().length());
            w->setText(s);
        }
    }else if(c->type()==DBTableWidgetColumn::EXTERN_KEY){
        QComboBox* w = dynamic_cast<QComboBox*>(editor);
        if(w){
            int id = w->findData(data, Qt::DisplayRole);
            if(id > -1)
                w->setCurrentIndex(id);
        }
    }
    
    return QItemDelegate::setEditorData(editor, index);
}

void DBTableWidgetDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    DBTableWidgetColumn* c = _parent->column(index.column());
    QVariant data, userData;
    if(c->isNumber()){
        GSpinBox* w = dynamic_cast<GSpinBox*>(editor);
        if(w){
            data = QString().setNum(w->value())+c->suffix();
            userData = w->value();
        }
    }else if(c->type()==DBTableWidgetColumn::STRING){
        GLineEdit* w = dynamic_cast<GLineEdit*>(editor);
        w->setPlaceholderText(c->placeHolder());
        if(w){
            data = w->text() + c->suffix();
            userData = w->text();
        }
    }else if(c->type()==DBTableWidgetColumn::EXTERN_KEY){
        QComboBox* w = dynamic_cast<QComboBox*>(editor);
        if(w){
           data = w->currentText();
           userData = w->currentData();
        }
    }
    
    if(!data.isNull())
        model->setData(index, data);
    if(!userData.isNull())
        model->setData(index, userData, Qt::UserRole);
    return QItemDelegate::setModelData(editor, model, index);
}


/*********************************************
 *                DBTableEditor              *
 *********************************************/
DBTableEditor::DBTableEditor()
{
    _bAdd = new QPushButton("Add");
    _bRemove = new QPushButton("Remove");
    _tableWidget = new DBTableWidget();

    QHBoxLayout* hl = new QHBoxLayout();
        hl->addStretch();
        hl->addWidget(_bAdd);
        hl->addWidget(_bRemove);

    connect(_bAdd, SIGNAL(released()), _tableWidget, SLOT(addRow()));
    //connect(_bRemove, SIGNAL(released()), _tableWidget, SLOT())

    QVBoxLayout* vl = new QVBoxLayout();
        vl->addWidget(_tableWidget);
        vl->addLayout(hl);

    setLayout(vl);
}

void DBTableEditor::readFromTable(DBBaseTable *table)
{
    if(_tableWidget) _tableWidget->readFromTable(table);
}

void DBTableEditor::apply()
{
    if(_tableWidget)
        _tableWidget->apply();
}

void DBTableEditor::reset()
{
    if(_tableWidget) _tableWidget->reset();
}
