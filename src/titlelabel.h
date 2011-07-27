#ifndef TITLELABEL_H
#define TITLELABEL_H

#include <QLabel>

class TitleLabel : public QLabel
{
    Q_OBJECT

public:
    TitleLabel(QWidget *parent);
    ~TitleLabel();
};
#endif // TITLELABEL_H
