#include "tooltipeater.h"

#include <QToolTip>

bool ToolTipEater::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        return true;
    }
    else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
