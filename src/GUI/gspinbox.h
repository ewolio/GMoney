#ifndef GSPINBOX_H
#define GSPINBOX_H

#include <QDoubleSpinBox>
#include <QtGui>

class GLineEdit;

class GSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit GSpinBox(QWidget *parent = 0);
    QValidator::State validate ( QString & input, int & pos ) const;

    enum Modifier{
        NONE,
        ADD,
        SUBSTRACT,
        MULTIPLY,
        DIVIDE
    };

    double maximum() const;
    double minimum() const;
    void setMaximum(double maximum);
    void setMinimum(double minimum);

    void setValue(double val);
    double value() const;
    
    void setDoubleEdit(bool doubleEdit=true);
    bool isDoubleEdit() const {return _doubleEdit;}
    
    void setSingleStep(double step);

    Modifier getModifier();

    QString fixup(QString t);

signals:

public slots:
    void setupModifier(Modifier newModifier);
    void applyModifier();
    void clearModifier();

protected:
    void keyPressEvent(QKeyEvent* event);
    void focusOutEvent(QFocusEvent *event);
    void paintPrepand();
    double max, min, previousValue;
    bool _doubleEdit = true;
    Modifier modifier;

private:
    GLineEdit* lineEdit;

};

#endif // GSPINBOX_H
