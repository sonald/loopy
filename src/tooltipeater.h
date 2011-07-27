#ifndef TOOLTIPEATER_H
#define TOOLTIPEATER_H

#include <QObject>
#include <QEvent>

class ToolTipEater : public QObject
{
    Q_OBJECT

    public:
        ToolTipEater(QObject * parent = 0) : QObject(parent) {};
        ~ToolTipEater() {};

    protected:
        bool eventFilter(QObject *obj, QEvent *event);
};

#endif // TOOLTIPEATER_H
