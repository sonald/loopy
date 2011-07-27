#include "timelabel.h"

#include <QTime>

TimeLabel::TimeLabel(QWidget *parent) : QLabel("00:00:00 / 00:00:00", parent)
    , m_currentTime(0)
{
    setAlignment(Qt::AlignCenter);
}

TimeLabel::~TimeLabel()
{
}

void TimeLabel::updateTime()
{
    long len = m_totalTime;
    long pos = m_currentTime;
    QString timeString;

    if (pos || len) {
        int sec = pos / 1000;
        int min = sec / 60;
        int hour = min / 60;
        int msec = pos;

        QTime playTime(hour % 60, min % 60, sec % 60, msec % 1000);
        sec = len / 1000;
        min = sec / 60;
        hour = min / 60;
        msec = len;

        QTime stopTime(hour % 60, min % 60, sec % 60, msec % 1000);
        QString timeFormat = "hh:mm:ss";
        timeString = playTime.toString(timeFormat);

        if (len)
            timeString += " / " + stopTime.toString(timeFormat);
        if (len == -1)
            timeString += "00:00:00";

    }
    setText(timeString);
}

void TimeLabel::setCurrentTime(qint64 time)
{
    m_currentTime = time;
    updateTime();
}

void TimeLabel::setTotalTime(qint64 time)
{
    m_totalTime = time;
    m_currentTime = 0;
    updateTime();
}
