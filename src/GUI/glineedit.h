#ifndef GLINEEDIT_H
#define GLINEEDIT_H

#include <QLineEdit>

class GLineEdit : public QLineEdit
{
public:
    GLineEdit(QWidget* parent=0);

    void setPrepand(const QString& text) {_prepandString = text;}
    void clearPrepand() {_prepandString.clear();}
    const QString& getPrepand() const {return _prepandString;}

protected:
    void paintEvent(QPaintEvent *e);

private:
    QString _prepandString;

};

#endif // GLINEEDIT_H
