#ifndef DBCOMPTES_H
#define DBCOMPTES_H

#include "dbtable.h"
class TypeCompte;
typedef gmisc::SMARTP<TypeCompte> STypeCompte;

class Currency;
typedef gmisc::SMARTP<Currency> SCurrency;

class DBCompte;
typedef gmisc::SMARTP<DBCompte> SDBCompte;


class TypeCompte: public DBElement{
public:
    TypeCompte();
    TypeCompte(QString _name, QString _iconPath, bool _isBankCheckable);

    bool bindValues(QVariantList &values) const;
    QString toString() const {return _name;}
    QVariant shortDescriptor() const {return getIcon();}

    bool getIsBankCheckable() const;
    void setIsBankCheckable(bool value);

    QString getIconPath() const;
    QIcon getIcon() const;
    void setIconPath(const QString &value);

    QString getName() const {return _name;}

protected:
    DBElement *create() const;
    void dataStructure(DBDataStructure &s) const;
    bool readResult(const QVariant &result, int id);
    
    QString _name;
    QString _iconPath;
    bool _isBankCheckable;
};

class Currency: public DBElement{

public:
    Currency(){}
    Currency(QString symbol, QString name, double taux=1);

    virtual bool bindValues(QVariantList& values) const;
    QString toString() const {return _symbol;}
    QVariant shortDescriptor() const {return symbol();}

    QString symbol() const {return _symbol;}
    void setSymbol(const QString& symbol);

    QString name() const {return _name;}
    void setName(const QString& name);

    double  taux() const {return _taux;}
    void setTaux(double taux);
    
protected:
    virtual DBElement *create() const {return new Currency();}
    virtual void dataStructure(DBDataStructure& s) const;
    virtual bool readResult(const QVariant &result, int id);

private:
    QString _symbol;
    QString _name;
    double  _taux = 1;
};


class DBCompte: public DBElement{
public:
    DBCompte(DBTable<TypeCompte> *types, DBTable<Currency> *currencies);
    DBCompte(QString name, STypeCompte type, SCurrency currency, double  sommeInit = 0);

    virtual bool bindValues(QVariantList &v) const;
    QString toString() const {return _name;}

    QString getName() const {return _name;}
    void setName(const QString &value);

    STypeCompte getType() const {return _type;}
    void setType(STypeCompte value);

    double getSommeInit() const {return _sommeInit;}
    void setSommeInit(double value);

    SCurrency getCurrency() const {return _currency;}
    void setCurrency(SCurrency currency);


    double getTotal() const;

protected:
    DBElement *create() const;
    virtual void dataStructure(DBDataStructure& s) const;
    virtual bool readResult(const QVariant &result, int id);

    DBTable<TypeCompte>*  _types=0;
    DBTable<Currency>*  _currencies=0;

    QString _name;
    STypeCompte _type=0;
    SCurrency _currency=0;
    double _sommeInit=0;
};

#endif // DBCOMPTES_H
