#ifndef WELCOMEPANEL_H
#define WELCOMEPANEL_H

#include <QtCore>
#include <QtWidgets>

namespace Ui {
class welcomePanel;
}

class WelcomePanel : public QFrame
{
    Q_OBJECT
    
public:
    explicit WelcomePanel(QWidget *parent = 0);
    ~WelcomePanel();

public slots:
    void recentButtonsClicked();
    void setupForLoading(QFile* f =0);

signals:
    void open();
    void openFiles(QFile*);
    void newProjet();
    
protected:
    void showEvent(QShowEvent *);

private:
    Ui::welcomePanel *ui;
};

#endif // WELCOMEPANEL_H
