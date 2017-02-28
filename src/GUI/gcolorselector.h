#ifndef GCOLORSELECTOR_H
#define GCOLORSELECTOR_H

#include <QQuickWidget>
#include <QQmlProperty>

class GColorSelector : public QQuickWidget
{
    Q_OBJECT
public:
    GColorSelector(QWidget* parent=0);

    QColor color() const {return _color.read().value<QColor>();}

signals:
    void colorChanged(QColor c);
public slots:
    void forceColor(QColor c);
    void showAt(int x, int y){showAt(QPoint(x,y));}
    void showAt(QPoint pos);

protected slots:
    void reportColorChanged();
protected:
    QQmlProperty _color;

protected:
    virtual void focusOutEvent(QFocusEvent *event);
};

#endif // GCOLORSELECTOR_H
