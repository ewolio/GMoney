#ifndef DBTABLE_H
#define DBTABLE_H

#include <QtCore>
#include <QtSql>
#include "smartcache.h"

class DBElement;
typedef gmisc::SMARTP<DBElement> SDBElement;

class DBConstraint;
class DBDataStructure;

class DBBaseTable : public QObject, protected gmisc::smartcache_interface<DBElement>
{
    Q_OBJECT
public:
    virtual ~DBBaseTable();
    virtual bool construct(SDBElement sampleElement);

    bool add(SDBElement e, bool toCache=true);
    SDBElement addNewElement();
    bool remove(int primaryKey, bool fromCache=true);
    bool remove(SDBElement e,  bool fromCache=true);

    bool applyCache();
    void resetCache();

    SDBElement elementFromRecord(const QSqlRecord& record);
    DBElement *detachedElementFromRecord(const QSqlRecord& record) const;

    SDBElement at(int primaryKey);
    SDBElement detachedElementAt(int primaryKey, bool notFromCache=false);

    QSqlRecord resultAt(int primaryKey);

    bool isValid() const;
    QSqlError lastError() const { return _lastQuery->lastError(); }
    QString getName() const { return _name; }
    int getTableID() const { return _id; }
    void connectToDatabase(int id) { _id = id; }
    
    const DBDataStructure& dataStructure() const;
    const SDBElement sample() const {return _sample;}

    int size() const {return _cache.size();}
    bool isValidPK(int primaryKey);

    QSqlQueryModel* generateGlobalModel(QObject* parent=0);
    QSqlQueryModel* updateGlobalModel(QAbstractItemModel *model);
    QSqlQuery& getSelectAllQuery();
    
signals:
    void changed();

protected:
    DBBaseTable(QString name);    

    int lastID();
    bool isValid(SDBElement e);

    SDBElement _sample;
    QString _name;
    int _id;

    QSqlQuery _selectAll, _selectPrimaryKeyQ, _insertQ, _updateQ, _selectMaxPK;
    QSqlQuery* _lastQuery;

    bool bindQuery(DBElement *e, QSqlQuery& query) const;

    // Cache handling
    gmisc::smartcache<DBElement> _cache;
    virtual bool readData(int elementKey, DBElement*& element);
    virtual bool writeData(DBElement* element);
    virtual bool addData(DBElement* e, int& key);
    virtual bool deleteData(DBElement* element);
    virtual int  readSize();
    virtual void cacheChanged();
    virtual DBElement* newElement();

public:
    class iterable_list
    {
    public:
        iterable_list(DBBaseTable* baseTable) :_baseTable(baseTable) {}
        
        class iterator
        {
        public:
            iterator():_end(true){}
            iterator( DBBaseTable* parent): _parent(parent){ 
                _query.setForwardOnly(true);
                _end=!_query.exec("SELECT * FROM "+parent->getName());
                if(!_query.first()){
                    if(_parent->_cache.newElements().size()) _newId = 0;
                    else _end = true;
                }
            }
            SDBElement operator*(){
                if(_end) return 0;
                if(_newId<0) return _parent->elementFromRecord(_query.record());
                return _parent->_cache.newElements().at(_newId);
            }
            bool operator!=( const iterator& it) {
                if(_end != it._end) return true;
                if(_end) return false;
                if(_parent != it._parent) return true;
                if(_newId>=0 || it._newId>=0) return _newId == it._newId;
                return _query.value("id").toInt() != it._query.value("id").toInt();
            }
            iterator& operator++() {
                if(_end) return *this;

                if(_newId<0 && _query.next())
                    return *this;
                if((size_t)_newId+1 < _parent->_cache.newElements().size())
                    _newId++;
                else
                    _end = true;

                return *this;
            }
        protected:
            QSqlQuery _query;
            int _newId=-1;
            bool _end = false;
            DBBaseTable* _parent=0;

        };
        iterator begin() const {return iterator(_baseTable);}
        iterator end() const {return iterator();}

        typedef iterator const_iterator; // Brute Force compatibility with qt foreach loop

    private:
        DBBaseTable *_baseTable;
    };

    iterable_list list(){return iterable_list(this);}
};


class DBConstraint{
public:
    DBConstraint(QString column, DBBaseTable* foreignTable, QString foreignKey = "id");
    DBConstraint(QString column, DBBaseTable* foreignTable, bool notNull, QString foreignKey = "id");

    const QString& column() const {return _column;}
    const QString& getForeignKey() const {return _foreignKey;}
    DBBaseTable* foreignTable() const {return _foreignTable;}
    bool isNotNullable() const {return _notNull;}

    QString generateQuery() const;
    QString generateDefinition() const;

private:
    QString _column, _foreignKey;
    DBBaseTable* _foreignTable;
    bool _notNull;
};


class DBDataStructure{
    QStringList _definitions;
    QStringList _names;
    QList<DBConstraint> _constraints;

    public:
    DBDataStructure& operator <<( const QString& type)              {add(type);       return *this;}
    DBDataStructure& operator <<( const DBConstraint& constraint)   {add(constraint); return *this;}

    void add(const QString& type);
    void add(const DBConstraint &constraint);

    QStringList definitions(bool includeID) const;
    QStringList fieldsName(bool includeID) const;
    const QStringList& definitions() const {return _definitions;}
    const QStringList& fieldsName() const {return _names;}
    const QList<DBConstraint>& constraints() const {return _constraints;}

    int size(bool includeID=false) const;
    const DBConstraint *constraintByName(QString name) const;

    bool operator ==(const DBDataStructure& d) const {return _definitions == d.definitions();}
    bool operator !=(const DBDataStructure& d) const {return ! operator ==(d);}
    protected:
};


class DBElement : public gmisc::smartcachable<int>{

public:
    DBElement();
    virtual ~DBElement();
    
    virtual bool bindValues(QVariantList &values) const =0;
    virtual bool readResult(const QSqlRecord &result);
    virtual QString toString() const =0;
    virtual QVariant shortDescriptor() const {return toString();}
    bool setValue(QVariant v, int id);

    int id() const {return key();}
    const DBDataStructure &dataStructure();
    const DBDataStructure dataStructure() const;
    QVariant value(int id, bool includeID = false);
    QVariant value(QString name);

    DBBaseTable* getTable() const;
    bool operator==(DBElement* e) const;
    bool operator !=(DBElement* e) const {return !operator==(e);}
    bool match(DBElement* e) const;

    bool setPrimaryKey(int primaryKey);

    virtual bool validateApply() const;

    DBElement* generateElement() const;

protected:
    virtual DBElement* create() const =0;
    virtual void dataStructure(DBDataStructure& d) const =0;
    virtual bool readResult(const QVariant &result, int id)=0;
    inline void addTo(QVariantList& l, const QVariant& v) const { l<<v; }
    inline void addTo(QVariantList& l, const SDBElement& e) const { QVariant v; v.setValue(e); addTo(l,v);}
    
    DBDataStructure* _structure=0;
};
Q_DECLARE_METATYPE(SDBElement)


template <typename T>
class DBTable: public DBBaseTable
{
public:
    DBTable(QString tableName):DBBaseTable(tableName){}

    gmisc::SMARTP<T> addNewElement(){return cast(DBBaseTable::addNewElement());}

    gmisc::SMARTP<T> at(int primaryKey){
        SDBElement e = DBBaseTable::at(primaryKey);
        return cast(e);
    }

    bool construct(gmisc::SMARTP<T> *sampleElement){return construct(cast(sampleElement));}
    bool construct(SDBElement* sampleElement){
        if(cast(sampleElement))
            return DBBaseTable::construct(sampleElement);
        return false;
    }

    static DBElement* cast(T* element){
        try{
            return dynamic_cast<DBElement*>(element);
        }
        catch(const std::exception& e){
            qDebug()<<"Conversion impossible: l'échantillon donné ne correspond pas au template";
            return 0;
            }
    }
    static T* cast(DBElement* element){
        try{
            return dynamic_cast<T*>(element);
        }
        catch(const std::exception& e){
            qDebug()<<"Conversion impossible: l'échantillon donné ne correspond pas au template";
            return 0;
        }
    }

    static const T* cast(const DBElement* element){
        try{
             return dynamic_cast<const T*>(element);
        }
        catch(const std::exception& e){
            qDebug()<<"Conversion impossible: l'échantillon donné ne correspond pas au template";
            return 0;
        }
    }

    static gmisc::SMARTP<T> cast(SDBElement element){
        try{
             DBElement* e =(DBElement*)element;
             gmisc::SMARTP<T> casted(dynamic_cast<T*>(e));
             return casted;
        }
        catch(const std::exception& e){
            qDebug()<<"Conversion impossible: l'échantillon donné ne correspond pas au template";
            return 0;
        }
    }



protected:
    virtual bool copy(const DBElement *from, DBElement *to){
        const T* fromT = cast(from);
        T* toT = cast(to);
        if(!fromT || !toT)
            return false;

        *toT = *fromT;
        return true;
    }

public:
    class iterable_list
    {
    public:
        iterable_list(DBBaseTable* baseTable) :_baseTable(baseTable) {}
        class iterator
        {
        public:
            iterator(DBBaseTable::iterable_list::iterator it): _it(it) {}
            const gmisc::SMARTP<T> operator*(){return cast(*_it);}
            bool operator!=( const iterator& it) {return _it != it._it;}
            iterator& operator++() {++_it; return *this;}
        protected:
            DBBaseTable::iterable_list::iterator _it;
        };
        iterator begin() const {return ((const DBBaseTable::iterable_list)_baseTable->list()).begin();}
        iterator end()   const {return ((const DBBaseTable::iterable_list)_baseTable->list()).end();}

        typedef iterator const_iterator; // Brute Force compatibility with qt foreach loop
        
//        class iterator
//        {
//        public:
//            iterator(DBBaseTable::iterable_list::iterator it): _it(it) {}
//            gmisc::SMARTP<T> operator*(){return cast(_it);}
//            bool operator!=( const iterator& it) const {return _it != it._it;}
//            const_iterator& operator++() {++_it; return *this;}
//        protected:
//            DBBaseTable::iterable_list::iterator _it;
//        };
//        iterator begin() {return _baseTable->list().begin();}
//        iterator end()   {return _baseTable->list().end();}

    private:
        DBBaseTable *_baseTable;
    };

    iterable_list list(){return iterable_list(this);}
};


#endif // DBTABLE_H
