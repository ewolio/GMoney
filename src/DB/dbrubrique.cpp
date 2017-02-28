#include "dbrubrique.h"

Rubrique::Rubrique(DBTable<Rubrique>* rubriques)
    : _rubriques(rubriques)
{
}

void Rubrique::dataStructure(DBDataStructure &s) const
{
    s << "nom VARCHAR(50) NOT NULL";
    s << "sommeMensuelle DOUBLE";
    s << "couleur TEXT(6)";
    s << DBConstraint("P0", _rubriques);
    s << DBConstraint("P1", _rubriques);
    s << DBConstraint("P2", _rubriques);
    s << DBConstraint("P3", _rubriques);
    s << DBConstraint("P4", _rubriques);
    s << DBConstraint("P5", _rubriques);
}

bool Rubrique::bindValues(QVariantList &v) const
{
    addTo( v, _name);
    addTo( v, _sommeMensuelle);
    addTo( v, _couleur.name().remove(0,1));

    for (int i = 0; i <= 5; ++i) {
        if(_parents.size()<=i)
            addTo( v, SDBElement(0));
        else
            addTo( v, _parents.at(i));
    }

    return true;
}

bool Rubrique::readResult(const QVariant &result, int id)
{
    switch(id){
    case 0: _name = result.toString(); break;
    case 1: _couleur = QColor("#"+result.toString()); break;
    case 2: _sommeMensuelle = result.toDouble(); break;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        id -= 3;
        SRubrique r = DBTable<Rubrique>::cast(result.value<SDBElement>());
        if(!r)
            return false;
        while(id < _parents.size()-1)
            _parents.append(0);
        if(_parents.at(id))
            return false;
        _parents.replace(id, r);
    break;
    }

    return true;
}

DBElement *Rubrique::create() const
{
    return new Rubrique(_rubriques);
}

QString Rubrique::getNom() const
{
    return _name;
}

void Rubrique::setNom(const QString &name)
{
    _name = name;
    reportChange();
}
double Rubrique::getSommeMensuelle() const
{
    return _sommeMensuelle;
}

void Rubrique::setSommeMensuelle(double sommeMensuelle)
{
    _sommeMensuelle = sommeMensuelle;
    reportChange();
}
QColor Rubrique::getCouleur() const
{
    return _couleur;
}

void Rubrique::setCouleur(const QColor &color)
{
    _couleur = color;
    reportChange();
}

bool Rubrique::setParent(SRubrique rParent)
{
    if(!updateParent(rParent)){
        qDebug()<<"Nombre de parents de rubrique maximum atteint...";
        qDebug()<<QString("Impossible de apparenter %1 à %2").arg(_name).arg(rParent->getNom());
        return false;
    }
    if(rParent)
        rParent->_children.append(this);

    return true;
}

bool Rubrique::updateParent(SRubrique parent)
{
    if(parent && parent->_parents.size() >= 5)
        return false;

    QList<SRubrique> temp = _parents;
    if(parent){
        _parents = parent->_parents;
        _parents.append(parent);
    }else
        _parents.clear();

    if(temp!=_parents){
        foreach(SRubrique r, _children){
            if(!r->updateParent(this)){
                _parents = temp;
                updateParent(this);
                return false;
            }
        }

        reportChange();
    }
    return true;
}


Ventilation::Ventilation(DBTable<Transaction> *transactions, DBTable<Rubrique> *rubriques): DBElement()
{
    this->_transactions = transactions;
    this->_rubriques = rubriques;
}

void Ventilation::dataStructure(DBDataStructure& s) const
{
    s << DBConstraint("transactionID", _transactions);
    s << DBConstraint("rubriqueID", _rubriques);
    s << "fraction FLOAT NOT NULL";
}


bool Ventilation::bindValues(QVariantList &v) const
{
    addTo( v, _transaction);
    addTo( v, _rubrique);
    addTo( v, _fraction);

    return true;
}

bool Ventilation::readResult(const QVariant &result, int id)
{
    switch(id){
    case 0: _transaction = DBTable<Transaction>::cast(result.value<SDBElement>()); break;
    case 1: _rubrique = DBTable<Rubrique>::cast(result.value<SDBElement>()); break;
    case 2: _fraction = result.toFloat(); break;
    }

    return true;
}

void Ventilation::setTransaction(STransaction value) {
    _transaction = value;
    reportChange();
}

void Ventilation::setRubrique(SRubrique value) {
    _rubrique = value;
    reportChange();
}

void Ventilation::setFraction(float value) {
    _fraction = value;
    reportChange();
}



DBElement *Ventilation::create() const
{
    return new Ventilation(_transactions, _rubriques);
}



