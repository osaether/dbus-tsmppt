#ifndef DBUS_TSMPPT_H
#define DBUS_TSMPPT_H

#include <QObject>

class DBusTsmpptBridge;
class VBusItem;

class DBusTsmppt : public QObject
{
    Q_OBJECT
public:
    DBusTsmppt(QObject *parent = 0);

signals:
    void terminateApp();

private slots:
    void onIpAddressChanged();
    void onPortNumberChanged();
    void onIntervalChanged();
    void onConnectionLost();

private:
    DBusTsmpptBridge *mTsmpptBridge;
    VBusItem *mIpAddress;
    VBusItem *mPortNumber;
    VBusItem *mInterval;
    void CreateTsmppt();
};

#endif // DBUS_TSMPPT_H
