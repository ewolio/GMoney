#include "dbtransactions.h"


Transaction::Transaction(DBTable<Contractant> *contractants):
    DBElement()
{
    _somme = 0;
    _commentaire = "";
    _date = QDate::currentDate();
    this->_contractants = contractants;
}


void Transaction::dataStructure(DBDataStructure& s) const
{
    s << DBConstraint("emetteur", _contractants);
    s << DBConstraint("destinataire", _contractants);
    s << "somme DOUBLE NOT NULL";
    s << "date DATE NOT NULL";
    s << "commentaire VARCHAR(500)";
}

bool Transaction::bindValues(QVariantList &v) const
{
    addTo( v, _emetteur);
    addTo( v, _destinataire);
    addTo( v, _somme);
    addTo( v, _date);
    addTo( v, _commentaire);

    return true;
}

bool Transaction::readResult(const QVariant &result, int id)
{
    switch(id){
    case 0: _emetteur = DBTable<Contractant>::cast(result.value<SDBElement>()); break;
    case 1: _destinataire = DBTable<Contractant>::cast(result.value<SDBElement>()); break;
    case 2: _somme = result.toDouble(); break;
    case 3: _date  = result.toDate(); break;
    case 4: _commentaire = result.toString(); break;
    }

    return true;
}

QString Transaction::toString() const
{
    QString s = "%1 -> %2: %3";
    s.arg(_emetteur->name()).arg(_destinataire->name()).arg(_somme);
    return s;
}

void Transaction::setSomme(double value) {
    _somme = value;
    reportChange();
}

void Transaction::setDate(const QDate &value) {
    _date = value;
    reportChange();
}

void Transaction::setCommentaire(const QString &value) {
    _commentaire = value;
    reportChange();
}


bool Transaction::setEmetteur(SContractant value)
{
    if(!value->isRegistered())
        return false;

    _emetteur = value;
    reportChange();
    return true;
}

bool Transaction::setDestinataire(SContractant value)
{
    if(!value->isRegistered())
        return false;

    _destinataire = value;
    reportChange();
    return true;
}

Transaction::TypeTransaction Transaction::getType()
{
    if(_emetteur->isCompte()){
        if(_destinataire->isCompte())
            return TRANSFERT;
        else
            return DEPENSE;
    }
    return RECETTE;
}


