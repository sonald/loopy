#include "titlelabel.h"

TitleLabel::TitleLabel(QWidget *parent) : QLabel(" No Media ", parent)
{
    setObjectName("fullscreenbarTitlelabel");
    setAlignment(Qt::AlignCenter);
}

TitleLabel::~TitleLabel()
{
}
