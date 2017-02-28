#include "mainwindow.h"

#include <QSettings>

#include "ui_mainwindow.h"

MainWindow *MainWindow::_instance = NULL;

MainWindow* MainWindow::Instance(){
    if(_instance == NULL){
        _instance = new MainWindow();
        connect(qApp, SIGNAL(aboutToQuit()), _instance, SLOT(deleteLater()));
    }

    return _instance;
}
void MainWindow::resetInstance(){
    disconnect(qApp, SIGNAL(aboutToQuit()), _instance, SLOT(deleteLater()));
    delete _instance;
    _instance = 0;
}


MainWindow::MainWindow() :
    QMainWindow(),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(Projet::Instance(), SIGNAL(compteAdded(int)), this, SLOT(addCompteWindow(int)));

    ui->budgetSplitter->setCollapsible(1, false);
    ui->actionOptionProjet->setEnabled(false);
    ui->actionSave->setEnabled(false);

    quitResquested = false;

    dialogConfigProject = new DialSetupProjet();
    welcomePanel = new WelcomePanel(this);
    connect(welcomePanel, SIGNAL(newProjet()), ui->actionNouveauProjet, SLOT(trigger()));
    connect(welcomePanel, SIGNAL(open()), ui->actionOpen, SLOT(trigger()));
    welcomePanel->show();

    connect(ui->actionNouveauProjet, &QAction::triggered, this, &MainWindow:: newProjet);
    connect(ui->actionOpen,          &QAction::triggered, this, &MainWindow::open);
    connect(ui->actionSave,          &QAction::triggered, this, &MainWindow::save);
    connect(ui->actionSaveAs,        &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->actionOptionProjet, SIGNAL(triggered()), dialogConfigProject, SLOT(option()));
    connect(Projet::Instance(), SIGNAL(closed()), this, SLOT(projetClosed()));
    connect(Projet::Instance(), SIGNAL(oppened()), this, SLOT(projetOpened()));
    connect(Projet::Instance(), SIGNAL(saved()), this, SLOT(projetSaved()));
    connect(Projet::Instance(), SIGNAL(changed()), this, SLOT(projetChanged()));
    connect(Projet::Instance(), SIGNAL(actionFailed(Projet::Action)), this, SLOT(projetActionFailed(Projet::Action)));
    connect(Projet::Instance(), SIGNAL(actionSucceed(Projet::Action)), this, SLOT(projetActionSucceed(Projet::Action)));

    connect(this, SIGNAL(destroyed()), qApp, SLOT(quit()));

    Projet::Instance()->checkForRecovery();

}

MainWindow::~MainWindow()
{
    delete ui;
}

/*********************************
 *      Projet Handling          *
 * *******************************/

void MainWindow::newProjet()
{
    Projet::Instance()->setupCreation();
}

void MainWindow::open(){

    QString filepath = QFileInfo(QFileDialog::getOpenFileName()).absoluteFilePath();
    Projet::Instance()->open(filepath);
}

void MainWindow::save()
{
    if(!Projet::Instance()->hasCurrentFile())
        return saveAs();
    Projet::Instance()->save();
}

void MainWindow::saveAs()
{
    QString filepath = QFileDialog::getSaveFileName(this, tr("Enregistrer le projet"),
                                                    "", tr("Projet GMoney (*.gmon)"));
    if(!filepath.endsWith(".gmon"))
        filepath.append(".gmon");

    Projet::Instance()->save(filepath);
}




void MainWindow::addCompteWindow(int /*i*/){


//    QMdiSubWindow *window = ui->comptesArea->addSubWindow(table);
//    window->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint);
//    window->setAttribute(Qt::WA_DeleteOnClose, false);
//        connect(compte, SIGNAL(destroyed()), window, SLOT(deleteLater()));


//    if(!ui->compteBar->layout()->isEmpty()){
//        QFrame *HBar = new QFrame(ui->compteBar);
//        HBar->setFrameShape(QFrame::HLine);
//        HBar->setFrameShadow(QFrame::Raised);
//        ui->compteBar->layout()->addWidget(HBar);
//        connect(compte, SIGNAL(destroyed()), HBar, SLOT(deleteLater()));
//    }


//    QPushButton *buttonCompte = new QPushButton(compte->getIcon(), compte->getName(), ui->compteBar);
//        buttonCompte->setCheckable(true);
//        buttonCompte->setChecked(true);
//        connect(buttonCompte, SIGNAL(toggled(bool)), window, SLOT(setVisible(bool)));
//        connect(buttonCompte, SIGNAL(clicked()), ui->comptesArea, SLOT(tileSubWindows()));
//        connect(compte, SIGNAL(destroyed()), buttonCompte, SLOT(deleteLater()));
//    ui->compteBar->layout()->addWidget(buttonCompte);

//    ui->comptesArea->tileSubWindows();
}

void MainWindow::projetActionSucceed(Projet::Action a)
{
    QSettings settings;
    QStringList l;
    int id;

    switch(a){
    case Projet::initiateProjet:
        dialogConfigProject->newProjet();
        break;
    case Projet::createProjet:
        break;

    case Projet::openProjet:
        l = settings.value("App/recentFiles").toStringList();
        id=l.indexOf(Projet::Instance()->getCurrentFile());
        if(id!=-1){
            l.move(id,0);

        }else{
            if(l.size()>5)
                l.removeLast();
            l.prepend(Projet::Instance()->getCurrentFile());
        }
        settings.setValue("App/recentFiles", l);


        break;
    case Projet::saveProjet:
        return;
    case Projet::recoverProjet:
        break;
    case Projet::closeProjet:
        if(quitResquested)
            qApp->quit();
        return;
    default:
        return;
    }
    quitResquested = false;
}

void MainWindow::projetActionFailed(Projet::Action a)
{
    switch(a){
    case Projet::initiateProjet:
        break;
    case Projet::createProjet:
        break;

    case Projet::openProjet:
        QMessageBox::critical(this,tr("Echec lors de l'ouverture du projet"), Projet::Instance()->getLastError());
        break;

    case Projet::saveProjet:
        QMessageBox::critical(this,tr("Echec lors de la sauvegarde de l'archive' du projet"), Projet::Instance()->getLastError());
        break;

    case Projet::recoverProjet:
        QMessageBox::warning(this, "Echec de la récupération",
                             "Le projet précédant n'a pas pu être récupéré...", QMessageBox::Ok);
        break;
    case Projet::closeProjet:
        return;
    default:
        return;
    }
    quitResquested = false;
}


void MainWindow::projetClosed(){
    ui->actionOptionProjet->setEnabled(false);
}

void MainWindow::projetOpened(){
    ui->actionOptionProjet->setEnabled(true);
}

void MainWindow::projetChanged()
{
    ui->actionSave->setEnabled(true);
}

void MainWindow::projetSaved()
{
    ui->actionSave->setEnabled(false);
}

/*
void MainWindow::updateRubriques(){
    int month = 9 + ui->moisR->currentIndex();



    for(int i=0; i<projet->getBudget()->size(); i++){

    SRubrique r = projet->getBudget()->at(i);

    QGroupBox* resume = new QGroupBox(r->getName(), ui->rubriquesView);

    QProgressBar* progress = new QProgressBar(resume);
        progress->setMaximum(r->getMax());
        progress->setValue(r->getCurrentTotal(month, 2012));

        QLabel* lmax = new QLabel(QString("<b>Maximum</b>:")+QString().setNum(r->getMax())+QString::fromUtf8(" €"));
        QLabel* lcurrent = new QLabel(QString("<b>Actuel</b>:")+QString().setNum(r->getCurrentTotal(month, 2012))+QString::fromUtf8(" €"));
        QLabel* lrestant = new QLabel(QString("<b>Reste</b>:")+QString().setNum(r->getMax()-r->getCurrentTotal(month, 2012))+QString::fromUtf8(" €"));

        QVBoxLayout *l = new QVBoxLayout(resume);
        l->addWidget(lmax);
        l->addWidget(lcurrent);
        l->addWidget(lrestant);
        l->addWidget(progress);

        resume->setLayout(l);
        resume->setAutoFillBackground(true);

        ui->layoutR->addWidget(resume, i/2, i%2);
    }
}

*/

void MainWindow::resizeEvent(QResizeEvent *){
    ui->comptesArea->tileSubWindows();
    welcomePanel->setGeometry(width()/2-150, height()/2-100, 300, 200);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if(!Projet::Instance()->close())
        e->ignore();
}
