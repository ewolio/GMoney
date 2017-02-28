#include "welcomepanel.h"
#include "ui_welcomepanel.h"
#include "projet.h"

WelcomePanel::WelcomePanel(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::welcomePanel)
{
    ui->setupUi(this);


    connect(ui->newB,SIGNAL(clicked()),this,SIGNAL(newProjet()));
    connect(ui->openB,SIGNAL(clicked()),this, SIGNAL(open()));

    connect(Projet::Instance(), SIGNAL(oppened()), this, SLOT(hide()));
    connect(Projet::Instance(), SIGNAL(closed()), this, SLOT(show()));
}

WelcomePanel::~WelcomePanel()
{
    delete ui;
}

void WelcomePanel::showEvent(QShowEvent *){
    while(ui->recentLayout->count()>0){
        QLayoutItem* l = ui->recentLayout->itemAt(0);
        ui->recentLayout->removeItem(l);
        delete l->widget();
        delete l;
    }

    QSettings s;
    foreach (QString i, s.value("App/recentFiles").toStringList()){
        QPushButton* b = new QPushButton(QFileInfo(i).baseName(),this);
        b->setFlat(true);
        b->setStyleSheet("text-align: left;");
        ui->recentLayout->addWidget(b);
        connect(b,SIGNAL(clicked()),this, SLOT(recentButtonsClicked()));
    }

    ui->recentLayout->addStretch();

    ui->ouvertureLabel->setHidden(true);
    ui->newB->setEnabled(true);
    ui->openB->setEnabled(true);
    ui->groupBox->setEnabled(true);
}

void WelcomePanel::setupForLoading(QFile *f){
    ui->newB->setEnabled(false);
    ui->openB->setEnabled(false);
    ui->groupBox->setEnabled(false);

    ui->ouvertureLabel->setVisible(true);
    if(f)
        ui->ouvertureLabel->setText("Ouverture du fichier: <i>"+QFileInfo(*f).fileName());
    else
        ui->ouvertureLabel->setText("Ouverture du fichier");

    this->repaint();
}

void WelcomePanel::recentButtonsClicked(){

    int i=ui->recentLayout->indexOf(qobject_cast<QWidget *>(sender()));

    QSettings s;

    if(i>=s.value("App/recentFiles").toStringList().size())
        return;
    qDebug()<<"Recent File :"<<s.value("App/recentFiles").toStringList().at(i);
    Projet::Instance()->open(s.value("App/recentFiles").toStringList().at(i));


}
