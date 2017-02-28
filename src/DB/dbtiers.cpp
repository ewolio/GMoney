#include "dbtiers.h"

Tiers::Tiers(QString name):
    DBElement()
{
    this->_name = name;
}


void Tiers::dataStructure(DBDataStructure &s) const
{
    s << "nom VARCHAR(50) NOT NULL";
}

bool Tiers::bindValues(QVariantList &v) const
{
    addTo( v, _name);
    return true;
}

bool Tiers::readResult(const QVariant &result, int id)
{
    switch(id){
    case 0: _name = result.toString(); break;
    }

    return true;
}



DBElement *Tiers::create() const
{
    return new Tiers();
}
QString Tiers::getName() const
{
    return _name;
}

void Tiers::setName(const QString &value)
{
    _name = value;
    reportChange();
}


Contractant::Contractant(DBTable<DBCompte> *comptesT, DBTable<Tiers> *tiersT)
{
    this->comptesT = comptesT;
    this->tiersT = tiersT;
}


void Contractant::dataStructure(DBDataStructure& s) const
{
    s << DBConstraint("idCompte", comptesT, false);
    s << DBConstraint("idTiers", tiersT, false);
}

bool Contractant::bindValues(QVariantList &v) const
{
    addTo( v, _c);
    addTo( v, _t);
    return true;
}

bool Contractant::readResult(const QVariant &result, int id)
{
    switch (id) {
    case 0: _c = DBTable<DBCompte>::cast(result.value<SDBElement>()); break;
    case 1: _t = DBTable<Tiers>::cast(result.value<SDBElement>()); break;
    }
    return true;
}

QString Contractant::name() const
{
    if(isCompte())
        return _c->getName();
    if(isTiers())
        return _t->getName();

    return "";
}

DBElement *Contractant::create() const
{
    return new Contractant(comptesT, tiersT);
}
