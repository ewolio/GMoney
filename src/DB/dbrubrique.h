#ifndef RUBRIQUE_H
#define RUBRIQUE_H

#include "dbtable.h"
#include "dbtransactions.h"

class Rubrique;
typedef gmisc::SMARTP<Rubrique> SRubrique;

class Ventilation;
typedef gmisc::SMARTP<Ventilation> SVentilation;

class Rubrique : public DBElement
{
public:
    Rubrique(DBTable<Rubrique> *_rubriques);

    bool bindValues(QVariantList& values) const;
    QString toString() const {return _name;}

    QString getNom() const;
    void setNom(const QString &name);

    double getSommeMensuelle() const;
    void setSommeMensuelle(double _sommeMensuelle);

    QColor getCouleur() const;
    void setCouleur(const QColor &color);

    const QList<SRubrique>& getParents() const {return _parents;}
    SRubrique parent() const {return _parents.isEmpty()?0:_parents.last();}
    bool setParent(SRubrique rParent);
    int rubriqueLvl() const {return _parents.size();}

protected:
    DBElement *create() const;
    void dataStructure(DBDataStructure& s) const;
    bool readResult(const QVariant &result, int id);
    
    DBTable<Rubrique>* _rubriques;

private:
    QString _name;
    double _sommeMensuelle=0;
    QColor _couleur;

    QList<SRubrique> _parents;

    bool updateParent(SRubrique parent);
    QList<SRubrique> _children;


};

class Ventilation : public DBElement{

public:
    Ventilation(DBTable<Transaction> *_transactions, DBTable<Rubrique> *_rubriques);

public:
    bool bindValues(QVariantList& v) const;
    QString toString() const {return QString().setNum(_transaction->getSomme()*_fraction);}

    STransaction getTransaction() const {return _transaction;}
    void setTransaction(STransaction value);

    SRubrique getRubrique() const {return _rubrique;}
    void setRubrique(SRubrique value);

    float getFraction() const {return _fraction;}
    void setFraction(float value);

protected:
    DBElement *create() const;
    void dataStructure(DBDataStructure& s) const;
    bool readResult(const QVariant &result, int id);

    DBTable<Transaction> *_transactions;
    DBTable<Rubrique> *_rubriques;

private:
    STransaction _transaction;
    SRubrique _rubrique;
    float _fraction;
};

#endif // RUBRIQUE_H
