#include "gspinbox.h"
#include "glineedit.h"


GSpinBox::GSpinBox(QWidget *parent) :
    QDoubleSpinBox(parent)
{
    setAlignment(Qt::AlignRight);
    min = QDoubleSpinBox::minimum();
    max = QDoubleSpinBox::maximum();
    modifier = NONE;
    previousValue = value();

    lineEdit = new GLineEdit(this);
    setLineEdit(lineEdit);
}


QValidator::State GSpinBox::validate(QString &input, int &pos) const
{
    input.replace(".",",");
    if(!_doubleEdit && input.contains(','))
        return QValidator::Invalid;
    return QDoubleSpinBox::validate(input, pos);
}

void GSpinBox::keyPressEvent(QKeyEvent *event)
{

    if(event->key()==Qt::Key_Plus)
        setupModifier(ADD);
    else if(event->key()==Qt::Key_Minus)
        setupModifier(SUBSTRACT);
    else if(event->key()==Qt::Key_Asterisk)
         setupModifier(MULTIPLY);
    else if(event->key()==Qt::Key_Slash)
        setupModifier(DIVIDE);
    else if(event->key()==Qt::Key_Enter || event->key()==Qt::Key_Return){
        if(modifier!=NONE){
            applyModifier();
            selectAll();
        }else
            event->ignore();
    }else if(event->key()==Qt::Key_Escape)
        clearModifier();
    else{
        QDoubleSpinBox::keyPressEvent(event);
        if(modifier==NONE)
            lineEdit->clearPrepand();
        return;
    }

    event->accept();

}

void GSpinBox::setupModifier(Modifier newModifier){
    applyModifier();

    switch(newModifier){
    case ADD:
        lineEdit->setPrepand("+");
        QDoubleSpinBox::setMaximum(maximum()-previousValue);
        QDoubleSpinBox::setMinimum(qMax(minimum()-previousValue,0.0));
        break;
    case SUBSTRACT:
        lineEdit->setPrepand("-");
        QDoubleSpinBox::setMaximum(previousValue-minimum());
        QDoubleSpinBox::setMinimum(qMax(previousValue-maximum(),0.0));
        break;
    case MULTIPLY:
        lineEdit->setPrepand("x");
        QDoubleSpinBox::setMaximum(maximum()/previousValue);
        QDoubleSpinBox::setMinimum(minimum()/previousValue);
        break;
    case DIVIDE:
        lineEdit->setPrepand("/");
        QDoubleSpinBox::setMinimum(qMax(previousValue/maximum(),0.01));
        QDoubleSpinBox::setMaximum(previousValue/minimum());
        break;

    case NONE:
        clearModifier();
        break;
    }

    clear();
    modifier = newModifier;
}

void GSpinBox::applyModifier()
{
    if(modifier == ADD)
        previousValue += QDoubleSpinBox::value();
    else if(modifier == SUBSTRACT)
        previousValue -= QDoubleSpinBox::value();
    else if(modifier == MULTIPLY)
        previousValue *= QDoubleSpinBox::value();
   else if(modifier == DIVIDE){
        previousValue /= QDoubleSpinBox::value();
    }else
        previousValue = QDoubleSpinBox::value();

    clearModifier();
    lineEdit->setPrepand("=");
}

void GSpinBox::clearModifier()
{
    if(modifier == NONE)
        previousValue=QDoubleSpinBox::value();

    QDoubleSpinBox::setMaximum(maximum());
    QDoubleSpinBox::setMinimum(minimum());
    QDoubleSpinBox::setValue(previousValue);

    lineEdit->clearPrepand();

    modifier = NONE;
}



GSpinBox::Modifier GSpinBox::getModifier(){
    return modifier;
}

QString GSpinBox::fixup(QString t)
{
    qDebug()<<t;
    return t.replace(".",",");
}

void GSpinBox::focusOutEvent(QFocusEvent *event)
{
    clearModifier();
    QDoubleSpinBox::focusOutEvent(event);
}

double GSpinBox::maximum() const{
    return max;
}
double GSpinBox::minimum() const{
    return min;
}

void GSpinBox::setMaximum(double maximum){
    max = maximum;
}

void GSpinBox::setMinimum(double minimum){
    min = minimum;
}

void GSpinBox::setValue(double val)
{
    clearModifier();
    
    if(!_doubleEdit)
        val = round(val);
    QDoubleSpinBox::setValue(val);
    previousValue=val;
}

double GSpinBox::value() const
{
    if(modifier!=NONE)
        return previousValue;

    return QDoubleSpinBox::value();
}

void GSpinBox::setDoubleEdit(bool doubleEdit)
{
    if(doubleEdit == _doubleEdit)
        return;
    _doubleEdit = doubleEdit;
    if(!doubleEdit){
        setValue(round(value()));
        setSingleStep(round(singleStep()));
    }
}

void GSpinBox::setSingleStep(double step)
{
    if(!_doubleEdit)
        step = round(step);
    QDoubleSpinBox::setSingleStep(step);
}

