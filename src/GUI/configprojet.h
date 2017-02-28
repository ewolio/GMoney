#ifndef CONFIG_PROJET_H
#define CONFIG_PROJET_H

#include <QtGui>
#include <QtWidgets>
#include "../projet.h"
#include "gcolorselector.h"
#include "dbtablewidget.h"

#include "ui_dialSetupProjet.h"

class DialSetupProjet: public QWidget
{

    Q_OBJECT

public:
    DialSetupProjet(QWidget *parent=0);


public slots:
    void option();
    void newProjet();

    void accept();
    void cancel();

protected slots:
    void reset();
    void readProject();

    void on_addCompte_triggered();
    void on_deleteCompte_triggered();

    void on_addRubrique_triggered();
    void on_addSubRubrique_triggered();
    void on_rubriqueView_itemChanged(QTreeWidgetItem *item);
    void on_rubriqueView_itemSelectionChanged();
    void on_rubriqueColor_pressed();
    void on_rubriqueProperties_editingFinished();
    void on_deleteRubrique_triggered();

    void setCurrentRubriqueColor(QColor color);

protected:

    bool eventFilter(QObject *watched, QEvent *event);
    
    void createCompteLine(SDBCompte compte=0, bool locked = true);
    void readComptes();
    void resetComptes();
    
    void readBudgets();
    QTreeWidgetItem* findParentItem(SRubrique rubrique) const;
    QTreeWidgetItem* findChildItem(SRubrique rubrique, QTreeWidgetItem* parent=0) const;
    void resetBudgets();
    void setCurrentRubrique(SRubrique r);
    QColor currentRubriqueColor() const;
    QIcon generateIcon(QColor color) const;
    void addRubrique(bool subrubrique);
    void dropEventRubrique(QDropEvent* event);

private:
    Projet* projet;
    Ui::DialSetupProjet ui;
    QDialogButtonBox *buttons;
    GColorSelector* colorPicker;
    
    QList<SDBCompte> _comptesAdded;
    SRubrique _currentRubrique;

    DBTableEditor* currencyTable, *typeCompteTable;

    bool _newProjet = false;
};


#endif // CONFIG_PROJET_H
