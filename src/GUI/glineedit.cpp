#include "glineedit.h"

#include <QPainter>

GLineEdit::GLineEdit(QWidget *parent)
    :QLineEdit(parent)
{
    setAlignment(Qt::AlignVCenter|Qt::AlignRight);
}

void GLineEdit::paintEvent(QPaintEvent *e)
{
    QLineEdit::paintEvent(e);
    QPainter painter( this );

    int topMargin, leftMargin, bottomMargin, rightMargin;
    getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

    int fontSize = height()*0.5;
    painter.setFont(QFont("Candara", fontSize, 2));
    painter.setPen(QColor("#6F868A"));
    painter.drawText(0,height()-0.6*fontSize,_prepandString);
}
