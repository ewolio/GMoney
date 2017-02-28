#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "projet.h"
#include "GUI/configprojet.h"
#include "GUI/welcomepanel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    static MainWindow* Instance();
    static void resetInstance();

public slots:
    void newProjet();
    void open();
    void save();
    void saveAs();

    void projetClosed();
    void projetOpened();

    void projetChanged();
    void projetSaved();

    //void updateRubriques();

    void addCompteWindow(int i);

    void projetActionSucceed(Projet::Action a);
    void projetActionFailed(Projet::Action a);

protected:
    void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *e);

private:
    MainWindow();
    ~MainWindow();

    Ui::MainWindow *ui;
    DialSetupProjet* dialogConfigProject;
    WelcomePanel* welcomePanel;
    bool quitResquested;

    static MainWindow* _instance;

};

#endif // MAINWINDOW_H
