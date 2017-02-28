#ifndef DBCOMPTEVIEW_H
#define DBCOMPTEVIEW_H

#include <QQuickWidget>
#include <QQmlProperty>
#include "DB/dbcomptes.h"

class DBCompteView: public QQuickWidget
{
public:
    DBCompteView(SDBCompte c, QWidget* parent=0);

protected:
    SDBCompte _compte;
};

#endif // DBCOMPTEVIEW_H
