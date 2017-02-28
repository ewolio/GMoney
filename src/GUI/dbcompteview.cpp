#include "dbcompteview.h"

#include <QQmlContext>

DBCompteView::DBCompteView(SDBCompte c, QWidget *parent)
    : QQuickWidget(parent), _compte(c)
{
    setSource(QUrl("qrc:/qml/DBCompteView.qml"));
}
