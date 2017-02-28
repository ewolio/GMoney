#include "database.h"
#include "projet.h"

DataBase::DataBase(QObject *parent) :
    QObject(parent), _tables()
{
    _database=0;
    _typesCompte = 0;
    _currencies = 0;
    _comptes = 0;
    _tiers = 0;
    _contractants = 0;
    _transactions = 0;
    _rubriques = 0;
    _ventilations = 0;
}

DataBase::~DataBase()
{
    delete _typesCompte;
    delete _currencies;
    delete _comptes;
    delete _tiers;
    delete _contractants;
    delete _transactions;
    delete _rubriques;
    delete _ventilations;
}

bool DataBase::connectDB(QSqlDatabase *dB)
{
    if(isConnected())
        disconnectDB();

    if(!dB->open())
        return false;

    _database=dB;

    if(!initDB())
        return false;


    return true;
}

bool DataBase::disconnectDB()
{
    _database->close();
    _database=NULL;
    return true;
}

bool DataBase::applyAll()
{
    foreach(DBBaseTable* t, _tables)
        if(!t->applyCache()) return false;
    return true;
}



bool DataBase::addTable(DBBaseTable *table, SDBElement sample)
{
    if(_tables.contains(table))
        return false;
    if(!table->construct(sample))
        return false;
    table->connectToDatabase(_tables.size());
    _tables.append(table);
    return true;
}

//-----------------  Controle de la base de données ---------------

bool DataBase::initDB()
{
    if(!_database->isOpen())
        return false;

    QSqlQuery query;
    bool ret = true;



    //________________CREATION DES TABLES__________________________

    _typesCompte = new DBTable<TypeCompte>("TypesCompte");
    _currencies = new DBTable<Currency>("Devises");
    _comptes = new DBTable<DBCompte>("Comptes");
    _tiers = new DBTable<Tiers>("Tiers");
    _contractants = new DBTable<Contractant>("Contractants");
    _transactions = new DBTable<Transaction>("Transactions");
    _rubriques = new DBTable<Rubrique>("Rubriques");
    _ventilations = new DBTable<Ventilation>("Ventilations");

    if(
        !addTable(_typesCompte, new TypeCompte())
     || !addTable(_currencies, new Currency())
     || !addTable(_comptes, new DBCompte(_typesCompte, _currencies))
     || !addTable(_tiers, new Tiers())
     || !addTable(_contractants, new Contractant(_comptes, _tiers))
     || !addTable(_transactions, new Transaction(_contractants))
     || !addTable(_rubriques, new Rubrique(_rubriques))
     || !addTable(_ventilations, new Ventilation(_transactions, _rubriques))
    ) return false;

   //________________INITIALISATIONS DES TABLES______________

    if(!_typesCompte->size()){
        bool r = true;
        r&= _typesCompte->add(new TypeCompte("Compte Chèques", ":/imgs/IcoCCheque.png", true));
        r&= _typesCompte->add(new TypeCompte("Compte Espèces", ":/imgs/IcoCEspece.png", false));
        r&= _typesCompte->add(new TypeCompte("Livret", ":/imgs/IcoCLivret.png", true));

        if(!r) return false;
    }

    if(!_currencies->size()){
        bool r = true;
        r&= _currencies->add(new Currency("€", "euro"));

        if(!r) return false;
    }

    if(!ret) return false;

    return applyAll();
}

bool DataBase::isConnected()
{
    return _database!=NULL;
}

QStringListModel *DataBase::queryStrings(QSqlQuery query)
{
    QStringListModel *r = new QStringListModel();
    populate(r, query);

    return r;
}

QStringListModel *DataBase::queryStrings(QString query)
{
    QSqlQuery q;
    q.prepare(query);
    return queryStrings(q);
}

QSqlQueryModel *DataBase::query(QSqlQuery query)
{
    QSqlQueryModel *r = new QSqlQueryModel();
    populate(r, query);

    return r;
}

QSqlQueryModel *DataBase::query(QString query)
{
    QSqlQuery q;
    q.prepare(query);
    QSqlQueryModel *m = new QSqlQueryModel();
    m->setQuery(q);
    return m;
}

void DataBase::populate(QStringListModel *model, QSqlQuery query)
{
    query.setForwardOnly(true);
    query.exec();
    QStringList l;
    while(query.next())
        l<<query.value(0).toString();

    model->setStringList(l);
}

void DataBase:: populate(QSqlQueryModel *model, QSqlQuery query){
    query.exec();
    model->setQuery(query);
    while(model->canFetchMore())
        model->fetchMore();
}
