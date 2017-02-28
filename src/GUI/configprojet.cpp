#include "configprojet.h"
#include "GUI/customwidget.h"

#include "global.h"

DialSetupProjet::DialSetupProjet(QWidget *parent)
    : QWidget(parent)
{

    projet = Projet::Instance();

    QTabWidget *w = new QTabWidget(this);
    ui.setupUi(w);


    // -- GENERALS CONTROLS --
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(accepted()), this, SLOT(hide()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(cancel()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(hide()));


    // -- COMPTES CONTROLS --
    connect(ui.addCompte, SIGNAL(pressed()), this, SLOT(on_addCompte_triggered()));
    connect(ui.deleteCompte, SIGNAL(pressed()), this, SLOT(on_deleteCompte_triggered()));

    ui.compteView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui.compteView->horizontalHeader()->resizeSection(0, 40);
    ui.compteView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui.compteView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui.compteView->horizontalHeader()->resizeSection(2, 90);
    ui.compteView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    ui.compteView->horizontalHeader()->resizeSection(3, 40);

    // -- BUDGETS CONTROLS --
    connect(ui.addRubrique, SIGNAL(pressed()), this, SLOT(on_addRubrique_triggered()));
    connect(ui.addSubRubrique, SIGNAL(pressed()), this, SLOT(on_addSubRubrique_triggered()));
    connect(ui.deleteRubrique, SIGNAL(pressed()), this, SLOT(on_deleteRubrique_triggered()));
    connect(ui.rubriquesView, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(on_rubriqueView_itemChanged(QTreeWidgetItem*)));
    connect(ui.rubriquesView, SIGNAL(itemSelectionChanged()), this, SLOT(on_rubriqueView_itemSelectionChanged()));
    connect(ui.rubriqueColor, SIGNAL(pressed()), this, SLOT(on_rubriqueColor_pressed()));
    connect(ui.rubriqueNom, SIGNAL(editingFinished()), this, SLOT(on_rubriqueProperties_editingFinished()));
    connect(ui.rubrqueSolde, SIGNAL(editingFinished()), this, SLOT(on_rubriqueProperties_editingFinished()));
    
    ui.rubriquesView->viewport()->installEventFilter(this);
    QHeaderView* h = ui.rubriquesView->header();
        h->setStretchLastSection(false);
        h->setSectionResizeMode(1, QHeaderView::Fixed);
        h->resizeSection(1,100);
        h->setSectionResizeMode(0, QHeaderView::Stretch);
    ui.rubriquesView->setHeader(h);
    ui.rubriquesView->setHeaderHidden(false);
    colorPicker = new GColorSelector(this);
    connect(colorPicker, SIGNAL(colorChanged(QColor)), this, SLOT(setCurrentRubriqueColor(QColor)));

    // -- ADVANCED CONTROLS  --
    currencyTable = new DBTableEditor();
        QVBoxLayout* l = new QVBoxLayout(); l->addWidget(currencyTable);
        ui.monnaieWidget->setLayout(l);

    typeCompteTable = new DBTableEditor();
        l = new QVBoxLayout(); l->addWidget(typeCompteTable);
        ui.typeCompteWidget->setLayout(l);


    // -- FINISH DIALOG --
    l = new QVBoxLayout(this);
    l->addWidget(w);
    l->addWidget(buttons);

    reset();
}

bool DialSetupProjet::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui.rubriquesView->viewport()){
        if(event->type() == QEvent::Drop){
            QDropEvent *e = static_cast<QDropEvent*>(event);
            dropEventRubrique(e);
        }
    }

    return false;
}

/******************************************
 ------------  dial control  -------------
 *****************************************/
void DialSetupProjet::option(){
    readProject();
    setWindowTitle("Options du projet");
    show();
    _newProjet = false;
}

void DialSetupProjet::newProjet()
{
    option();
    setWindowTitle("Nouveau Projet");
    _newProjet = true;
}

void DialSetupProjet::accept(){

    // -- APPLY COMPTES --
    foreach(SDBCompte c, _comptesAdded)
        projet->getDatabase()->comptes()->add(c);
    _comptesAdded.clear();
    projet->getDatabase()->comptes()->applyCache();

    // -- APPLY BUDGETS --
    projet->getDatabase()->rubriques()->applyCache();

    // -- APPLY ADVANCED --
    projet->getDatabase()->typesCompte()->applyCache();
    projet->getDatabase()->currencies()->applyCache();


    // -- FINISHING --
    if(_newProjet)
        projet->validateCreation();

    readProject();
}

void DialSetupProjet::cancel()
{

    // -- CANCEL COMPTES --
    _comptesAdded.clear();
    projet->getDatabase()->comptes()->resetCache();


    // -- CANCEL BUDGETS --
    projet->getDatabase()->rubriques()->resetCache();

    // -- APPLY ADVANCED --
    projet->getDatabase()->typesCompte()->resetCache();
    projet->getDatabase()->currencies()->resetCache();


    // -- FINISHING --
    if(_newProjet){
        projet->close();
        reset();
    }else
        readProject();
}

/******************************************
 ------------  intern function  -----------
 *****************************************/


void DialSetupProjet::reset(){

    resetComptes();
    resetBudgets();


    //___________ Reset Advanced  _____________
    ui.advanceSelector->setCurrentRow(0);
    currencyTable->reset();
    typeCompteTable->reset();

}

void DialSetupProjet::readProject(){
    readComptes();
    readBudgets();
    currencyTable->readFromTable(projet->getDatabase()->currencies());
    typeCompteTable->readFromTable(projet->getDatabase()->typesCompte());
}

//_________________gestion Comptes____________________________________
void DialSetupProjet::readComptes()
{
    resetComptes();
    
    foreach(SDBCompte c, projet->getDatabase()->comptes()->list()){
        int row = ui.compteView->rowCount();
        ui.compteView->setRowCount(row+1);

        ui.compteView->setItem(row, 0, new QTableWidgetItem(c->getType()->getIcon(), c->getType()->getName()));
        ui.compteView->setItem(row, 1, new QTableWidgetItem( c->getName()));
        ui.compteView->setItem(row, 2, new QTableWidgetItem(QString().setNum(c->getTotal())));
        ui.compteView->setItem(row, 3, new QTableWidgetItem(c->getCurrency()->symbol()));
    }
}

void DialSetupProjet::resetComptes()
{
    ui.compteView->clearContents();
    ui.compteView->setRowCount(0);

    _comptesAdded.clear();
}


void DialSetupProjet::on_addCompte_triggered(){
    createCompteLine();
}

void DialSetupProjet::createCompteLine(SDBCompte compte, bool locked)
{
    int row = ui.compteView->rowCount();
    ui.compteView->insertRow(row);

    if(!compte){
        SCurrency c = projet->getDatabase()->currencies()->at(1);
        compte = new DBCompte("", projet->getDatabase()->typesCompte()->at(1), c);
        _comptesAdded.append(compte);
        locked = false;
    }

    // -- NOM --
    QLineEdit* name = new QLineEdit();
    if(!compte->getName().isEmpty())
        name->setText(compte->getName());
    else
        name->setPlaceholderText("Nom");
    connect(name, &QLineEdit::textChanged, [this, compte, row, name](){
        compte->setName(name->text());
    });
    ui.compteView->setCellWidget(row, 1, name);

    // -- TYPE --
    QComboBox* type = new QComboBox();
    foreach(STypeCompte t, projet->getDatabase()->typesCompte()->list()){
        type->addItem( t->getIcon(), "", t->id());
        type->setItemData(type->count()-1, t->getName(), Qt::ToolTipRole);
    }
    type->setCurrentIndex(0);
    connect(type, QtOverload<int>::of(&QComboBox::currentIndexChanged),
            [this, compte, type, row, name] ( int newIndex) {
                compte->setType(projet->getDatabase()->typesCompte()->at(type->itemData(newIndex).toInt()));
                name->setText(compte->getType()->getName());
            }  );
    ui.compteView->setCellWidget(row,0, type);

    // -- SOLDE INI --
    GSpinBox* soldeInit = new GSpinBox();
        soldeInit->setValue(0);
    ui.compteView->setCellWidget(row, 2, soldeInit);
    connect(soldeInit, QtOverload<double>::of(&GSpinBox::valueChanged), [compte](double value){
        compte->setSommeInit(value);
    });

    // -- MONNAIE --
    QComboBox* currency = new QComboBox();
    foreach(SCurrency c, projet->getDatabase()->currencies()->list()){
        currency->addItem( c->symbol(), c->id());
        currency->setItemData(currency->count()-1, c->name(), Qt::ToolTipRole);
    }
    connect(currency, QtOverload<int>::of(&QComboBox::currentIndexChanged),
            [this, compte, currency](int newIndex) {
                compte->setCurrency(projet->getDatabase()->currencies()->at(currency->itemData(newIndex).toInt()));
            }  );
    ui.compteView->setCellWidget(row, 3, currency);

    // non editable
    if(locked){
        type->setEnabled(false);
        name->setEnabled(false);
        soldeInit->setEnabled(false);
        currency->setEnabled(false);
    }
}

void DialSetupProjet::on_deleteCompte_triggered(){
    int ok = QMessageBox::warning(this, tr("Attention"), tr("Suppression du compte.\n""Êtes vous sur de supprimer le compte?"),QMessageBox::Ok| QMessageBox::Cancel);
    if(ok != QMessageBox::Ok)
        return;
}


//_________________gestion Budgets____________________________________

void DialSetupProjet::readBudgets()
{
    resetBudgets();
    
    for(int lvl=0; lvl<5; lvl++){
        foreach(SRubrique c, projet->getDatabase()->rubriques()->list()){
            if(c->rubriqueLvl() == lvl){
                QTreeWidgetItem* item = new QTreeWidgetItem({c->getNom(), QString().setNum(c->getSommeMensuelle(),'g',2)+" €"});
                QVariant v; v.setValue((SDBElement)c);
                item->setData(0, Qt::UserRole, v);

                item->setIcon(0,generateIcon(c->getCouleur()));

                item->setTextAlignment(1, Qt::AlignLeft);
                item->setFlags(item->flags()|Qt::ItemIsEditable);
                if(!lvl)
                    ui.rubriquesView->addTopLevelItem(item);
                else{
                    QTreeWidgetItem* parent = findParentItem(c);
                    if(!parent)
                        break;
                    parent->addChild(item);
                    parent->setExpanded(true);
                }
            }
        }
    }
}

QTreeWidgetItem *DialSetupProjet::findParentItem(SRubrique rubrique) const
{
    QTreeWidgetItem* parent = ui.rubriquesView->invisibleRootItem();

    int iLvl = 0;
    while(parent && iLvl < rubrique->rubriqueLvl()){

        for(int i=0; i<parent->childCount(); i++){
            if(parent->child(i)->data(0,Qt::UserRole).value<SDBElement>() == rubrique->getParents().at(iLvl))
                parent = parent->child(i);
        }
        iLvl++;
    }

    return parent;
}

QTreeWidgetItem *DialSetupProjet::findChildItem(SRubrique rubrique, QTreeWidgetItem *parent) const
{
    if(parent && parent->data(0,Qt::UserRole).value<SDBElement>() == rubrique)
        return parent;


    QList<QTreeWidgetItem*> l;
    if(parent){
        for(int i=0; i<parent->childCount(); i++)
            l.append(parent->child(i));
    }else{
        for(int i=0; i<ui.rubriquesView->topLevelItemCount(); i++)
            l.append(ui.rubriquesView->topLevelItem(i));
    }

    QTreeWidgetItem* r=0;
    foreach (QTreeWidgetItem* i, l) {
        if( (r=findChildItem(rubrique, i)) )
            return r;
    }

    return 0;
}

void DialSetupProjet::resetBudgets()
{
    setCurrentRubrique(0);
    ui.rubriquesView->clear();
    _currentRubrique = 0;
}

void DialSetupProjet::setCurrentRubrique(SRubrique r)
{
    _currentRubrique = r;
    if(!r){
        ui.rubriqueNom->clear();
        ui.rubrqueSolde->setValue(0);
        setCurrentRubriqueColor(QColor("white"));
        ui.rubriqueProperty->setEnabled(false);
    }else{
        ui.rubriqueNom->setText(r->getNom());
        setCurrentRubriqueColor(r->getCouleur());
        ui.rubrqueSolde->setValue(r->getSommeMensuelle());
        ui.rubriqueProperty->setEnabled(true);
    }
}

QIcon DialSetupProjet::generateIcon(QColor color) const
{
    QPixmap icon(50,50); QPainter p(&icon);
    p.fillRect(0,0,50,50,color); p.end();
    QPixmap mask(50,50); QPainter pMask(&mask);
    pMask.fillRect(0,0,50,50, QColor(255,255,255));
    pMask.setBrush(QColor(0,0,0)); pMask.drawEllipse(10,10,40,40);
    icon.setMask(QBitmap(mask));

    return icon;
}

void DialSetupProjet::setCurrentRubriqueColor(QColor color)
{
    QString ss =  "background-color: rgb(%1, %2, %3);";
    ss = ss.arg(color.red()).arg(color.green()).arg(color.blue());
    ui.rubriqueColor->setStyleSheet(ss);

    if(colorPicker->color()!=color) colorPicker->forceColor(color);
    if(_currentRubrique){
        _currentRubrique->setCouleur(color);
        if(!ui.rubriquesView->selectedItems().isEmpty())
            ui.rubriquesView->selectedItems().first()->setIcon(0,generateIcon(color));
        else
            readBudgets();
    }
}

void DialSetupProjet::addRubrique(bool subrubrique)
{
    SRubrique r = projet->getDatabase()->rubriques()->addNewElement();
    r->setNom("rubrique");
    r->setCouleur(QColor(255,100,100));

    if(_currentRubrique){
        if(subrubrique){
            if(!r->setParent(_currentRubrique) )
                r->setParent(_currentRubrique->parent());
        }else if(_currentRubrique->parent())
            r->setParent(_currentRubrique->parent());
        if(r->parent())
            r->setCouleur(r->parent()->getCouleur());
    }
    readBudgets();
    ui.rubriquesView->setItemSelected(findChildItem(r), true);
}

void DialSetupProjet::dropEventRubrique(QDropEvent *event)
{
    if(_currentRubrique){
        QTreeWidgetItem* newParentItem =  ui.rubriquesView->itemAt(event->pos());
        if(newParentItem){
            SDBElement newParent = newParentItem->data(0,Qt::UserRole).value<SDBElement>();
            if(newParent != _currentRubrique->parent()){
                _currentRubrique->setParent(DBTable<Rubrique>::cast(newParent));
                newParentItem->setExpanded(true);
            }
        }
    }
}

void DialSetupProjet::on_addRubrique_triggered(){
    addRubrique(false);
}

void DialSetupProjet::on_addSubRubrique_triggered()
{
    addRubrique(true);
}

void DialSetupProjet::on_rubriqueView_itemSelectionChanged()
{
    QList<QTreeWidgetItem*> l = ui.rubriquesView->selectedItems();
    if(l.isEmpty()){
        setCurrentRubrique(0);
        return;
    }

    SDBElement e = l.first()->data(0, Qt::UserRole).value<SDBElement>();
    SRubrique r = DBTable<Rubrique>::cast(e);
    setCurrentRubrique(r);
}

void DialSetupProjet::on_rubriqueColor_pressed()
{
    if(colorPicker->isVisible()){
        colorPicker->hide();
        update();
    }else{
        colorPicker->showAt(ui.rubriqueColor->mapTo(this,QPoint(0,0)) - QPoint(colorPicker->width(),colorPicker->height()) );
    }
}

void DialSetupProjet::on_rubriqueProperties_editingFinished()
{
    if(!_currentRubrique || ui.rubriquesView->selectedItems().isEmpty())
        return;

    QTreeWidgetItem* item = ui.rubriquesView->selectedItems().first();

    if(_currentRubrique->getNom() != ui.rubriqueNom->text())
        item->setText(0, ui.rubriqueNom->text());
    if(_currentRubrique->getSommeMensuelle() != ui.rubrqueSolde->value())
        item->setText(1, QString().setNum(ui.rubrqueSolde->value()));
}
void DialSetupProjet::on_rubriqueView_itemChanged(QTreeWidgetItem* item)
{
    SDBElement e = item->data(0, Qt::UserRole).value<SDBElement>();
    SRubrique r = DBTable<Rubrique>::cast(e);

    bool changed = false;
    if( (changed|=r->getNom()!=item->text(0)) )
        r->setNom(item->text(0));

    QStringList soldeValue = item->text(1).split(' ',QString::SkipEmptyParts);
    double sum = -1;
    if(!soldeValue.isEmpty())
        sum = soldeValue.first().replace(',','.').toFloat();
    if(sum!=-1 && (changed|=r->getSommeMensuelle()!=sum))
        r->setSommeMensuelle(sum);
    item->setText(1, QString().setNum(r->getSommeMensuelle(),'g',2)+" €");

    if(r==_currentRubrique && changed)
        setCurrentRubrique(r);
}

void DialSetupProjet::on_deleteRubrique_triggered(){
    if(_currentRubrique)
        projet->getDatabase()->rubriques()->remove(_currentRubrique->id());
    readBudgets();
}
