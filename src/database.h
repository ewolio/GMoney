#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include "DB/dbtable.h"

#include "DB/dbcomptes.h"
#include "DB/dbrubrique.h"
#include "DB/dbtiers.h"
#include "DB/dbtransactions.h"

class DataBase: public QObject{

    QSqlDatabase* _database;
    QList<DBBaseTable*> _tables;

    DBTable<TypeCompte>* _typesCompte;
    DBTable<Currency>* _currencies;
    DBTable<DBCompte>* _comptes;
    DBTable<Tiers>* _tiers;
    DBTable<Contractant>* _contractants;
    DBTable<Transaction>* _transactions;
    DBTable<Rubrique>* _rubriques;
    DBTable<Ventilation>* _ventilations;

public:
    DataBase(QObject* parent = 0);
    ~DataBase();

    bool connectDB(QSqlDatabase *dB);
    bool disconnectDB();

    bool applyAll();
    bool addTable(DBBaseTable *table, SDBElement sample);
    bool initDB();
    bool isConnected();

    QSqlDatabase *db(){return _database;}
    DBTable<TypeCompte>* typesCompte() const {return _typesCompte;}
    DBTable<Currency>* currencies() const {return _currencies;}
    DBTable<DBCompte>* comptes() const {return _comptes;}
    DBTable<Tiers>* tiers() const {return _tiers;}
    DBTable<Contractant>* contractants() const {return _contractants;}
    DBTable<Transaction>* transactions() const {return _transactions;}
    DBTable<Rubrique>* rubriques() const {return _rubriques;}
    DBTable<Ventilation>* ventilations() const {return _ventilations;}

    QStringListModel *queryStrings(QSqlQuery query);
    QStringListModel *queryStrings(QString query);
    QSqlQueryModel *query(QSqlQuery query);
    QSqlQueryModel *query(QString query);

    void populate(QStringListModel *model, QSqlQuery query);
    void populate(QSqlQueryModel *model, QSqlQuery query);
};

#endif // DATABASE_H
