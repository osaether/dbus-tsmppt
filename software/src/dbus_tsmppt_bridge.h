#ifndef DBUS_TSMPPT_BRIDGE_H
#define DBUS_TSMPPT_BRIDGE_H

#include <QObject>
#include "dbus_bridge.h"

class Tsmppt;

class DBusTsmpptBridge : public DBusBridge
{
    Q_OBJECT
public:
    DBusTsmpptBridge(Tsmppt *tsmppt, QObject *parent = 0);
    ~DBusTsmpptBridge();

private slots:
    void onTsmpptConnected();

private:
    Tsmppt *mTsmppt;
};

#endif // DBUS_TSMPPT_BRIDGE_H
