#include <QsLog.h>
#include <velib/qt/v_busitem.h>
#include "dbus_tsmppt.h"
#include "tsmppt.h"
#include "dbus_tsmppt_bridge.h"

static const QString PortNumberPath = "/Settings/TristarMPPT/PortNumber";
static const QString IpAddressPath = "/Settings/TristarMPPT/IPAddress";
static const QString IntervalPath = "/Settings/TristarMPPT/Interval";

DBusTsmppt::DBusTsmppt(QObject *parent): 
QObject(parent), mTsmpptBridge(0), mIpAddress(new VBusItem(this)), mPortNumber(new VBusItem(this)), mInterval(new VBusItem(this))
{
    connect(mPortNumber, SIGNAL(valueChanged()), this, SLOT(onPortNumberChanged()));
    mPortNumber->consume("com.victronenergy.settings", PortNumberPath);
    mPortNumber->getValue();
    connect(mIpAddress, SIGNAL(valueChanged()), this, SLOT(onIpAddressChanged()));
    mIpAddress->consume("com.victronenergy.settings", IpAddressPath);
    mIpAddress->getValue();
    connect(mInterval, SIGNAL(valueChanged()), this, SLOT(onIntervalChanged()));
    mInterval->consume("com.victronenergy.settings", IntervalPath);
    mInterval->getValue();
}

void DBusTsmppt::CreateTsmppt()
{
    if (mIpAddress->getValue().toString() == QString(""))
       return;
    if (mPortNumber->getValue().toInt() == 0)
       return;
    if (mInterval->getValue().toInt() == 0)
       return;
    Tsmppt *mTsmppt = new Tsmppt(mIpAddress->getValue().toString(), mPortNumber->getValue().toInt(), mInterval->getValue().toInt());
    connect(mTsmppt, SIGNAL(connectionLost()), this, SLOT(onConnectionLost()));
    mTsmpptBridge = new DBusTsmpptBridge(mTsmppt, this);
}

void DBusTsmppt::onIpAddressChanged()
{
    QLOG_INFO() << "IP Address changed";
    delete mTsmpptBridge;
    CreateTsmppt();
}

void DBusTsmppt::onIntervalChanged()
{
    QLOG_INFO() << "Logging interval changed";
    delete mTsmpptBridge;
    CreateTsmppt();
}

void DBusTsmppt::onPortNumberChanged()
{
    QLOG_INFO() << "Port number changed";
    delete mTsmpptBridge;
    CreateTsmppt();
}

void DBusTsmppt::onConnectionLost()
{
    delete mTsmpptBridge;
    CreateTsmppt();
}



