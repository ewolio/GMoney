#include "dbcomptes.h"

TypeCompte::TypeCompte():
    DBElement()
{
    _name = "Compte";
    _iconPath = ":/imgs/IcoCCheque.png";
    _isBankCheckable=false;
}

TypeCompte::TypeCompte(QString name, QString iconPath, bool isBankCheckable):
    TypeCompte()
{
    _name = name;
    _iconPath = iconPath;
    _isBankCheckable = isBankCheckable;
}

void TypeCompte::dataStructure(DBDataStructure& s) const
{
    s<<"name VARCHAR(40) NOT NULL";
    s<<"iconPath VARCHAR(250) NOT NULL";
    s<<"isBankCheckable BOOLEAN NOT NULL";
}

bool TypeCompte::bindValues(QVariantList& values ) const
{
    addTo( values, _name);
    addTo( values, _iconPath);
    addTo( values, _isBankCheckable);
    return true;
}

bool TypeCompte::readResult(const QVariant &result, int id)
{
    switch(id){
    case 0: _name     = result.toString(); break;
    case 1: _iconPath = result.toString(); break;
    case 2: _isBankCheckable = result.toBool(); break;
    }
    return true;
}

DBElement *TypeCompte::create() const
{
    return new TypeCompte();
}

QString TypeCompte::getIconPath() const
{
    return _iconPath;
}

QIcon TypeCompte::getIcon() const
{
    return QIcon(getIconPath());
}

void TypeCompte::setIconPath(const QString &value)
{
    _iconPath = value;
    reportChange();
}

bool TypeCompte::getIsBankCheckable() const
{
    return _isBankCheckable;
}

void TypeCompte::setIsBankCheckable(bool value)
{
    _isBankCheckable = value;
    reportChange();
}



DBCompte::DBCompte(DBTable<TypeCompte> *types, DBTable<Currency> *currencies)
    : _types(types), _currencies(currencies)
{
}

DBCompte::DBCompte(QString name, STypeCompte type, SCurrency currency, double sommeInit)
    : DBCompte(dynamic_cast<DBTable<TypeCompte>*> (type->getTable()), dynamic_cast<DBTable<Currency>*> (currency->getTable()) )
{
    _name = name;
    _type = type;
    _currency = currency;
    _sommeInit = sommeInit;
}

void DBCompte::dataStructure(DBDataStructure &s) const
{
    s << "nom VARCHAR(50) NOT NULL";
    s << DBConstraint("type", _types);
    s << "sommeInit DOUBLE";
    s << DBConstraint("currency", _currencies);
}


bool DBCompte::bindValues(QVariantList& v) const
{
    addTo( v, _name);
    addTo( v, _type);
    addTo( v, _sommeInit);
    addTo( v, _currency);

    return true;
}

bool DBCompte::readResult(const QVariant &r, int id)
{
    switch (id) {
    case 0: _name = r.toString(); break;
    case 1: _type = DBTable<TypeCompte>::cast(r.value<SDBElement>()); break;
    case 2: _sommeInit = r.toDouble(); break;
    case 3: _currency = DBTable<Currency>::cast(r.value<SDBElement>()); break;
    }

    return true;
}

void DBCompte::setName(const QString &value) {
    _name = value;
    reportChange();
}

void DBCompte::setType(STypeCompte value) {
    _type = value;
    reportChange();
}

void DBCompte::setSommeInit(double value) {
    _sommeInit = value;
    reportChange();
}

void DBCompte::setCurrency(SCurrency currency) {
    _currency = currency;
    reportChange();
}

DBElement *DBCompte::create() const
{
    return new DBCompte(_types, _currencies);
}

double DBCompte::getTotal() const
{
    QSqlQuery q;
    double sum = _sommeInit;

    q.prepare("SELECT id FROM Contractants WHERE idCompte=:id");
    q.bindValue(":id", id());
    q.exec();
    if(!q.first())
        return sum;

    int idContractant = q.value(0).toDouble();

    q.prepare("SELECT SUM(somme) FROM Transactions WHERE emetteur=:id");
    q.bindValue(":id", idContractant);
    q.exec();
    if(!q.first())
        return sum;

    sum -= q.value(0).toDouble();

    q.clear();
    q.prepare("SELECT SUM(somme) FROM Transactions WHERE destinataire=:id");
    q.bindValue(":id", idContractant);
    q.exec();
    if(!q.first())
        return 0;

    sum += q.value(0).toDouble();

    return sum;
}





Currency::Currency(QString symbol, QString name, double taux)
    :_symbol(symbol), _name(name), _taux(taux)
{
}

void Currency::dataStructure(DBDataStructure& d) const
{
    d << "symbol VARCHAR(4) NOT NULL";
    d << "name VARCHAR(50)";
    d << "taux DOUBLE NOT NULL";
}

bool Currency::bindValues(QVariantList &v) const
{
    addTo( v, _symbol);
    addTo( v, _name);
    addTo( v, _taux);

    return true;
}

bool Currency::readResult(const QVariant &result, int id)
{
    switch (id) {
    case 0: _symbol = result.toString(); break;
    case 1: _name   = result.toString(); break; 
    case 2: _taux   = result.toDouble(); break; 
    }

    return true;
}

void Currency::setSymbol(const QString &symbol) {
    _symbol = symbol;
    reportChange();
}

void Currency::setName(const QString &name) {
    _name = name;
    reportChange();
}

void Currency::setTaux(double taux)
{
    _taux = taux;
    reportChange();
}
