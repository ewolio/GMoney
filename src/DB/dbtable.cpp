#include "dbtable.h"

#include <QDebug>

DBBaseTable::DBBaseTable(QString name):
    _selectAll(), _selectPrimaryKeyQ(), _insertQ(), _updateQ(), _selectMaxPK(), _cache(this)
{
    _name = name;
}

DBBaseTable::~DBBaseTable()
{
}

bool DBBaseTable::construct(SDBElement sampleElement)
{
    _sample = sampleElement;

    QString query = "CREATE TABLE IF NOT EXISTS "+_name+" (";
    DBDataStructure structure = _sample->dataStructure();
    query+=structure.definitions(true).join(", ");

    foreach (const DBConstraint& c, structure.constraints())
        query+=", "+c.generateQuery();

    query+=")";

    QSqlQuery q;
    if(!q.exec(query)){
        qDebug()<<q.lastError();
        return false;
    }



    //Construction des query
    _selectAll.prepare("SELECT * FROM "+_name);

    _selectPrimaryKeyQ.prepare("SELECT * FROM "+_name+" WHERE id=:id");

    QString insertS = QString("INSERT INTO "+_name+" VALUES (NULL");
    QString updateS = QString("UPDATE "+_name+" SET ");

    foreach(QString s, structure.fieldsName()){
        insertS+=", :"+s;
        updateS+=s+"=:"+s+", ";
    }

    insertS+=")";
    updateS.remove(updateS.length()-2, 2);
    updateS+=" WHERE id=:id";


    _insertQ.prepare(insertS);
    _updateQ.prepare(updateS);

    _selectMaxPK.prepare("SELECT MAX(id) FROM "+_name);

    _lastQuery = &_insertQ;
    return true;
}

SDBElement DBBaseTable::at(int primaryKey)
{
    return _cache.readElementFromCache(primaryKey);
}

SDBElement DBBaseTable::detachedElementAt(int primaryKey, bool notFromCache)
{
    return _cache.readDetachedElement(primaryKey, notFromCache);
}

bool DBBaseTable::add(SDBElement e, bool toCache)
{
    return _cache.addElement(e, toCache);
}


SDBElement DBBaseTable::addNewElement()
{
    DBElement* e = _sample->generateElement();
    if(!add(e))
        return NULL;

    return e;
}

bool DBBaseTable::remove(SDBElement e, bool fromCache)
{
    return _cache.deleteElement(e, fromCache);
}

bool DBBaseTable::remove(int primaryKey, bool fromCache)
{
    SDBElement e = _cache.readElementFromCache(primaryKey);
    return _cache.deleteElement(e,fromCache);
}

bool DBBaseTable::isValid(SDBElement e)
{
    if(e==NULL)
        return false;
    if(!_sample->match(e))
        return false;
    return true;
}

bool DBBaseTable::bindQuery(DBElement *e, QSqlQuery &query) const
{
    QStringList names = _sample->dataStructure().fieldsName();

    QVariantList values;
    e->bindValues(values);
    if(names.size()!=values.size()){
        qDebug()<< "Erreur du bind de query du tableau " + _name<<": mauvaises dimensions de la liste de valeurs...";
        return false;
    }

    for(int i=0; i<names.size(); i++){
        QVariant v = values.at(i);
        if(v.canConvert<SDBElement>()){
            SDBElement ele = v.value<SDBElement>();
            v = 0;
            if(ele)
                v = ele->id();
        }
        query.bindValue(":"+names.at(i), v);
    }

    return true;
}

// ------ CACHE HANDLING -------------------
bool DBBaseTable::applyCache()
{
    return _cache.apply();
}

void DBBaseTable::resetCache()
{
    _cache.reset();
}

bool DBBaseTable::readData(int elementKey, DBElement*& element)
{
    QSqlRecord r = resultAt(elementKey);
    element = detachedElementFromRecord(r);
    if(!element)
        return false;

    return true;
}

bool DBBaseTable::writeData(DBElement *e)
{
    if(!isValid(e))
        return false;

    _updateQ.bindValue(":id", e->id());
    if(!bindQuery(e,_updateQ))
        return false;


    _lastQuery = &_updateQ;
    bool r = _updateQ.exec();
    _updateQ.finish();

    return r;
}

bool DBBaseTable::addData(DBElement* e, int &key)
{
    if(!bindQuery(e,_insertQ))
        return false;

    _lastQuery = &_insertQ;
    bool r = _insertQ.exec();
    _insertQ.finish();

    if(!r)
        return false;
    key = lastID();
    return true;
}

bool DBBaseTable::deleteData(DBElement* /*element*/)
{
    return false;
}

int DBBaseTable::readSize()
{
    QSqlQuery q = QSqlQuery("SELECT COUNT(*) FROM "+_name);
    q.first();
    return q.value(0).toInt();
}

void DBBaseTable::cacheChanged()
{
    emit changed();
}

DBElement *DBBaseTable::newElement()
{
    return _sample->generateElement();
}

// ------ DATABASE HANDLING ----------------
SDBElement DBBaseTable::elementFromRecord(const QSqlRecord &record)
{
    int id = record.value("id").toInt();
    if(_cache.contains(id))
        return _cache.readElementFromCache(id);

    SDBElement e;
    if(!(e=detachedElementFromRecord(record)))
        return 0;
    _cache.emplaceElement(e);

    return e;
}
DBElement* DBBaseTable::detachedElementFromRecord(const QSqlRecord &record) const
{
    if(record.isEmpty() || !record.value("id").toInt())
        return 0;

    DBElement *e = _sample->generateElement();
    e->readResult(record);
    e->setPrimaryKey(record.value("id").toInt());

    return e;
}


QSqlRecord DBBaseTable::resultAt(int primaryKey)
{
    _selectPrimaryKeyQ.bindValue(":id", primaryKey);
    _selectPrimaryKeyQ.exec();
    _lastQuery = &_selectPrimaryKeyQ;
    _selectPrimaryKeyQ.first();

    QSqlRecord r = _selectPrimaryKeyQ.record();
    _selectPrimaryKeyQ.finish();

    return r;
}


int DBBaseTable::lastID()
{
    int r=-1;

    if(_selectMaxPK.exec() && _selectMaxPK.first())
        r = _selectMaxPK.value(0).toInt();
    _selectMaxPK.finish();

    return r;
}


bool DBBaseTable::isValid() const{
    return _sample != NULL;
}

const DBDataStructure &DBBaseTable::dataStructure() const {
    return _sample->dataStructure();
}

bool DBBaseTable::isValidPK(int primaryKey)
{
    _selectPrimaryKeyQ.bindValue(":id", primaryKey);
    _lastQuery = &_selectPrimaryKeyQ;

    bool r=false;

    if(_selectPrimaryKeyQ.exec())
        r=_selectPrimaryKeyQ.first();
    _selectPrimaryKeyQ.finish();

    return r;
}

QSqlQueryModel* DBBaseTable::generateGlobalModel
(QObject *parent)
{
    QSqlQueryModel* m = new QSqlQueryModel(parent);
    m->setQuery(getSelectAllQuery());

    QStringList names = _sample->dataStructure().fieldsName(true);

    for(int i=0; i < names.size(); ++i)
        m->setHeaderData(i, Qt::Horizontal, names.at(i));

    return m;
}

QSqlQueryModel* DBBaseTable::updateGlobalModel(QAbstractItemModel *model)
{
    QSqlQueryModel* m = dynamic_cast<QSqlQueryModel*>(model);
    if(m)
        m->setQuery(getSelectAllQuery());

    return m;
}

QSqlQuery &DBBaseTable::getSelectAllQuery()
{
    _selectAll.exec();
    _lastQuery = &_selectAll;
    return _selectAll;
}





/********************************************************************
 *                              DBElement                           *
 ********************************************************************/

DBElement::DBElement():
    smartcachable<int>()
{
}

DBElement::~DBElement(){
    if(_structure)
        delete _structure;
}

const DBDataStructure& DBElement::dataStructure() {
    if(_structure)
        return *_structure;
    
    if(getTable() && getTable()->sample() && getTable()->sample() != this)
        _structure = getTable()->sample()->_structure;
    else{
        _structure = new DBDataStructure();
        dataStructure(*_structure);
    }
    
    return *_structure;
}

const DBDataStructure DBElement::dataStructure() const {
    if(_structure)
        return *_structure;
    
    if(getTable() && getTable()->sample() && getTable()->sample() != this)
        return getTable()->sample()->dataStructure();
    
    DBDataStructure s;
    dataStructure(s);
    return s;
}

bool DBElement::readResult(const QSqlRecord &result)
{
    for(int i=0; i<dataStructure().fieldsName().size(); i++){
        QString name = dataStructure().fieldsName().at(i);
        QVariant v = result.value(name);
        const DBConstraint* c=dataStructure().constraintByName(name);
        if(c)
            v.setValue(c->foreignTable()->at(v.toInt()));
        if(!readResult( v, i))
            return false;
    }
    return true;
}

bool DBElement::setValue(QVariant v, int id)
{
    if(!readResult(v, id))
        return false;
    reportChange();
    return true;
}

QVariant DBElement::value(int id, bool includeID)
{
    QVariantList values;
    bindValues(values);
    if(includeID){
        if(!id) return DBElement::id();
        else id--;
    }
    return values.at(id);
}

QVariant DBElement::value(QString name)
{
    int id = dataStructure().fieldsName().indexOf(name);
    if(name=="id")
        return DBElement::id();
    if(id==-1)
        return QVariant();

    return value(id);
}

DBBaseTable *DBElement::getTable() const
{
    return dynamic_cast<DBBaseTable*>(cache());
}

bool DBElement::operator ==(DBElement *e) const
{
    return match(e) && key()==e->key() && key()!=-1;
}

bool DBElement::match(DBElement *e) const
{
    DBDataStructure d1, d2;
    e->dataStructure(d1);
    dataStructure(d2);
    return d1 == d2;
}

bool DBElement::setPrimaryKey(int primaryKey)
{
    setElemKey(primaryKey);
    return true;
}

bool DBElement::validateApply() const
{
    DBDataStructure d = dataStructure();
    if(d.constraints().isEmpty())
        return true;

    QVariantList l;
    bindValues(l);

    foreach(const DBConstraint& c, d.constraints()){
        int fieldId = d.fieldsName().indexOf(c.column());
        if(fieldId == -1 || !l.at(fieldId).canConvert<SDBElement>())
            continue;
        SDBElement ele = l.at(fieldId).value<SDBElement>();
        if(!ele)
            continue;
        if(!ele->apply())
            return false;
    }

    return true;
}

DBElement *DBElement::generateElement() const
{
    DBElement* e = create();
    return e;
}




/********************************************************************
 *                           DBConstraint                           *
 ********************************************************************/
DBConstraint::DBConstraint(QString column, DBBaseTable *foreignTable, bool notNull, QString foreignKey)
    :_column(column), _foreignKey(foreignKey), _foreignTable(foreignTable), _notNull(notNull)
{
}

DBConstraint::DBConstraint(QString column, DBBaseTable *foreignTable, QString foreignKey)
    : DBConstraint(column, foreignTable, true, foreignKey)
{
}

QString DBConstraint::generateQuery() const{
    return "FOREIGN KEY ("+_column+") REFERENCES "+_foreignTable->getName()+"("+_foreignKey+") ";
}

QString DBConstraint::generateDefinition() const
{
    return _column + " INTEGER" + (_notNull?" NOT NULL":"");
}



/********************************************************************
 *                            DBStructure                           *
 ********************************************************************/

void DBDataStructure::add(const QString &type)
{
    _definitions << type;
    QString name = type.left(type.indexOf(' '));
    _names << name;
}

void DBDataStructure::add(const DBConstraint &constraint)
{
    add(constraint.generateDefinition());
    _constraints.append(constraint);
}

QStringList DBDataStructure::definitions(bool includeID) const 
{
    if(includeID){
        QStringList s(_definitions);
        s.prepend("id INTEGER PRIMARY KEY");
        return s;
    }
    return _definitions;
}

QStringList DBDataStructure::fieldsName(bool includeID) const  {
    if(includeID){
        QStringList s(_names);
        s.prepend("id");
        return s;
    }
    return _names;
}

int DBDataStructure::size(bool includeID) const {
    return _definitions.size() + (includeID?1:0);
}

const DBConstraint* DBDataStructure::constraintByName(QString name) const
{
    for(int i=0; i<_constraints.size(); i++){
        if(name == _constraints.at(i).column())
            return &_constraints.at(i);
    }

    return 0;
}
