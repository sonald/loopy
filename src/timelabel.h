#ifndef TIMELABEL_H
#define TIMELABEL_H

#include <QLabel>

class TimeLabel : public QLabel
{
    Q_OBJECT

public:
    TimeLabel(QWidget *parent);
    ~TimeLabel();

public slots:
    void setCurrentTime(qint64);
    void setTotalTime(qint64);
    void updateTime();

private:
    qint64 m_currentTime;
    qint64 m_totalTime;
};
#endif // TIMELABEL_H
