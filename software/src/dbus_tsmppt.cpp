#include <QsLog.h>
#include <velib/qt/v_busitem.h>
#include "dbus_tsmppt.h"
#include "tsmppt.h"
#include "dbus_tsmppt_bridge.h"

static const QString PortNumberPath = "/Settings/TristarMPPT/PortNumber";
static const QString IpAddressPath = "/Settings/TristarMPPT/IPAddress";

DBusTsmppt::DBusTsmppt(QObject *parent): 
QObject(parent), mTsmpptBridge(0), mIpAddress(new VBusItem(this)), mPortNumber(new VBusItem(this))
{
    connect(mPortNumber, SIGNAL(valueChanged()), this, SLOT(onPortNumberChanged()));
    mPortNumber->consume("com.victronenergy.settings", PortNumberPath);
    mPortNumber->getValue();
    connect(mIpAddress, SIGNAL(valueChanged()), this, SLOT(onIpAddressChanged()));
    mIpAddress->consume("com.victronenergy.settings", IpAddressPath);
    mIpAddress->getValue();
}

void DBusTsmppt::CreateTsmppt()
{
    if (mIpAddress->getValue().toString() == QString(""))
       return;
    if (mPortNumber->getValue().toInt() == 0)
       return;
    QLOG_DEBUG() << "CreateTsmppt:mIpAddress=" << mIpAddress->getValue();
    QLOG_DEBUG() << "CreateTsmppt:mPortNumber=" << mPortNumber->getValue();
    Tsmppt *mTsmppt = new Tsmppt(mIpAddress->getValue().toString(), mPortNumber->getValue().toInt());
    connect(mTsmppt, SIGNAL(connectionLost()), this, SLOT(onConnectionLost()));
    mTsmpptBridge = new DBusTsmpptBridge(mTsmppt, this);
}

void DBusTsmppt::onIpAddressChanged()
{
    delete mTsmpptBridge;
    CreateTsmppt();
}

void DBusTsmppt::onPortNumberChanged()
{
    delete mTsmpptBridge;
    CreateTsmppt();
}

void DBusTsmppt::onConnectionLost()
{
    delete mTsmpptBridge;
    CreateTsmppt();
}



