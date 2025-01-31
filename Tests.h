#ifndef TESTS_H
#define TESTS_H

#include <QObject>

class Tests : public QObject
{
    Q_OBJECT
public:
    explicit Tests(QObject *parent = nullptr);

    void setGroupAddress(QString groupAddress);
    void setFunctionalEnable(bool isFunctionalEnable);
    void setDurationEnable(bool isDurationEnable);
    void setFunctionalDays(QString functionalDays);
    void setFunctionalTime(QString functionalTime);
    void setDurationPeriodicity(QString durationPeriodicity);
    void setDurationDate(QString durationDate);
    void setDurationTime(QString durationTime);

    QString getGroupAddress(void);
    bool getFunctionalEnable(void);
    bool getDurationEnable(void);
    QString getFunctionalDays(void);
    QString getFunctionalTime(void);
    QString getDurationPeriodicity(void);
    QString getDurationDate(void);
    QString getDurationTime(void);

signals:

private:
    QString _groupAddress;
    bool _isFunctionalEnable;
    bool _isDurationEnable;
    QString _functionalDays;
    QString _functionalTime;
    QString _durationPeriodicity;
    QString _durationDate;
    QString _durationTime;
};

#endif // TESTS_H
