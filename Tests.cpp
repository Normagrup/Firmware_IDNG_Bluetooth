#include "Tests.h"

Tests::Tests(QObject *parent)
    : QObject{parent}
{

}

void Tests::setFunctionalEnable(bool isFunctionalEnable)
{
    _isFunctionalEnable = isFunctionalEnable;
}

void Tests::setDurationEnable(bool isDurationEnable)
{
    _isDurationEnable = isDurationEnable;
}

void Tests::setGroupAddress(QString groupAddress)
{
    _groupAddress = groupAddress;
}

void Tests::setFunctionalDays(QString functionalDays)
{
    _functionalDays = functionalDays;
}

void Tests::setFunctionalTime(QString functionalTime)
{
    _functionalTime = functionalTime;
}

void Tests::setDurationPeriodicity(QString durationPeriodicity)
{
    _durationPeriodicity = durationPeriodicity;
}

void Tests::setDurationDate(QString durationDate)
{
    _durationDate = durationDate;
}

void Tests::setDurationTime(QString durationTime)
{
    _durationTime = durationTime;
}

bool Tests::getFunctionalEnable()
{
    return _isFunctionalEnable;
}

bool Tests::getDurationEnable()
{
    return _isDurationEnable;
}

QString Tests::getGroupAddress()
{
    return _groupAddress;
}

QString Tests::getFunctionalDays()
{
    return _functionalDays;
}

QString Tests::getFunctionalTime()
{
    return _functionalTime;
}

QString Tests::getDurationPeriodicity()
{
    return _durationPeriodicity;
}

QString Tests::getDurationDate()
{
    return _durationDate;
}

QString Tests::getDurationTime()
{
    return _durationTime;
}
