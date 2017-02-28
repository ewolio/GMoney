#include "gcolorselector.h"

#include <QQuickItem>

GColorSelector::GColorSelector(QWidget *parent)
    : QQuickWidget(parent)
{
    setSource(QUrl("qrc:/qml/GColorSelector.qml"));
    hide();
    _color = QQmlProperty(rootObject(), "color");
    connect(rootObject(), SIGNAL(editingFinished()), this, SLOT(reportColorChanged()));
}

void GColorSelector::forceColor(QColor c)
{
    _color.write(c);
}

void GColorSelector::showAt(QPoint pos)
{
    move(pos);
    show();
    setFocus();
}

void GColorSelector::reportColorChanged()
{
    emit(colorChanged(_color.read().value<QColor>()));
}

void GColorSelector::focusOutEvent(QFocusEvent *)
{
    hide();
}

