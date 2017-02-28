#ifndef DBTRANSACTIONS_H
#define DBTRANSACTIONS_H

#include "dbtable.h"
#include "dbtiers.h"


class Transaction;
typedef gmisc::SMARTP<Transaction> STransaction;


class Transaction : public DBElement
{
public:
    explicit Transaction(DBTable<Contractant>* _contractants);

    enum TypeTransaction{
        DEPENSE=1,
        RECETTE=2,
        TRANSFERT=3
    };

    bool bindValues(QVariantList& v) const;
    QString toString() const;

    //Getter, setter
    double getSomme() const {return _somme;}
    void setSomme(double value);

    QDate getDate() const {return _date;}
    void setDate(const QDate &value);

    QString getCommentaire() const {return _commentaire;}
    void setCommentaire(const QString &value);

    SContractant getEmetteur() const {return _emetteur;}
    bool setEmetteur(SContractant value);

    SContractant getDestinataire() const {return _destinataire;}
    bool setDestinataire(SContractant value);

    TypeTransaction getType();

protected:
    DBElement *create() const {return new Transaction(_contractants);}
    void dataStructure(DBDataStructure& s) const;
    bool readResult(const QVariant &result, int id);
    
    SContractant _emetteur, _destinataire;
    double _somme;
    QDate _date;
    QString _commentaire;

    DBTable<Contractant>* _contractants;
};

#endif // DBTRANSACTIONS_H
