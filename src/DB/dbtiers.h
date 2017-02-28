#ifndef TIERS_H
#define TIERS_H

#include "dbtable.h"
#include "dbcomptes.h"

class Tiers;
typedef gmisc::SMARTP<Tiers> STiers;

class Contractant;
typedef gmisc::SMARTP<Contractant> SContractant;

class Tiers : public DBElement
{
public:
    Tiers(QString _name="");

public:
    bool bindValues(QVariantList& values) const;
    QString toString() const { return _name; }

    QString getName() const;
    void setName(const QString &value);

protected:
    DBElement *create() const;
    void dataStructure(DBDataStructure& s) const;
    bool readResult(const QVariant &result, int id);
    
    QString _name;
};

class Contractant: public DBElement
{
public:
    Contractant(DBTable<DBCompte> *comptesT, DBTable<Tiers> *tiersT);

public:

    bool bindValues(QVariantList& values) const;
    QString toString() const {return name();}

    SDBCompte toCompte() const {return _c;}
    STiers toTiers() const {return _t;}
    QString name() const;
    bool isCompte() const {return _c != NULL;}
    bool isTiers() const {return _t != NULL;}

protected:
    DBElement *create() const;
    void dataStructure(DBDataStructure& s) const;
    bool readResult(const QVariant &result, int id);
    
    DBTable<DBCompte>* comptesT;
    DBTable<Tiers>* tiersT;

    SDBCompte _c;
    STiers _t;
};

#endif // TIERS_H
