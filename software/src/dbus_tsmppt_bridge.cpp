#include <QsLog.h>
#include <QCoreApplication>
#include <QStringList>
#include "dbus_tsmppt_bridge.h"
#include "tsmppt.h"

#define VE_PROD_ID_TRISTAR_MPPT_60A 0xABCD


static const QString service = "com.victronenergy.solarcharger.tsmppt";

DBusTsmpptBridge::DBusTsmpptBridge(Tsmppt *tsmppt, QObject *parent):
    DBusBridge(service, parent),
    mTsmppt(tsmppt) 
{
    Q_ASSERT(tsmppt != 0);
    connect(tsmppt, SIGNAL(tsmpptConnected()), this, SLOT(onTsmpptConnected()));

    QString processName = QCoreApplication::arguments()[0];
    produce("/Mgmt/ProcessName", processName);
    produce("/Mgmt/ProcessVersion", QCoreApplication::applicationVersion());
    produce("/Mgmt/Connection", "Modbus-TCP");
    produce("/ProductId", VE_PROD_ID_TRISTAR_MPPT_60A);
    produce("/DeviceInstance", 0);
    produce("/ErrorCode", 0);

    produce(mTsmppt, "arrayCurrent", "/Pv/I", "A", 2);
    produce(mTsmppt, "arrayVoltage", "/Pv/V", "V", 2);
    produce(mTsmppt, "outputPower", "/Yield/Power", "W", 0);
    produce(mTsmppt, "chargingCurrent", "/Dc/0/Current", "A", 2);
    produce(mTsmppt, "batteryVoltage", "/Dc/0/Voltage", "V", 2); 
    produce(mTsmppt, "batteryTemperature", "/Dc/0/Temperature");
    produce(mTsmppt, "chargeState", "/State");

    produce("/History/Overall/DaysAvailable", 1);
    produce(mTsmppt, "batteryVoltageMaxDaily", "/History/Daily/0/MaxBatteryVoltage", "V", 2);
    produce(mTsmppt, "batteryVoltageMinDaily", "/History/Daily/0/MinBatteryVoltage", "V", 2);
    produce(mTsmppt, "powerMaxDaily", "/History/Daily/0/MaxPower", "W", 0);
    produce(mTsmppt, "arrayVoltageMaxDaily", "/History/Daily/0/MaxPvVoltage", "V", 2);
    produce(mTsmppt, "wattHoursDaily", "/History/Daily/0/Yield", "kWh", 2);
    produce(mTsmppt, "timeInAbsorption", "/History/Daily/0/TimeInAbsorption");
    // TSMPPT does not give time in bulk:
    // produce(mTsmppt, "timeInBulk", "/History/Daily/0/TimeInBulk");
    produce(mTsmppt, "timeInFloat", "/History/Daily/0/TimeInFloat");

    //registerService();
}

DBusTsmpptBridge::~DBusTsmpptBridge()
{
   QLOG_DEBUG() << "DBusTsmpptBridge::~DBusTsmpptBridge()";
   delete mTsmppt;
}


void DBusTsmpptBridge::onTsmpptConnected()
{
    produce("/Connected", 1);
    produce("/Mode", 1);
    QString fwver = mTsmppt->firmwareVersion();
    produce("/FirmwareVersion", fwver.toInt());
    QString hwver = mTsmppt->hardwareVersion();
    produce("/HardwareVersion", hwver);
    produce("/ProductName", mTsmppt->productName());
    uint64_t serial = mTsmppt->serialNumber();
    produce("/Serial", QString::number(serial));
    QString logmsg = mTsmppt->productName() + " (serial #" + QString::number(serial) + ", controler v" + hwver + "." + fwver + ") connected";
    QLOG_INFO() << logmsg.toStdString().c_str();
    registerService();
}
