#ifndef TSMPPT_H
#define TSMPPT_H

#include <modbus.h>
#include <stdint.h>
#include <QObject>

class QTimer;

class Tsmppt : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double batteryVoltage READ batteryVoltage WRITE setBatteryVoltage NOTIFY batteryVoltageChanged)
    Q_PROPERTY(double batteryVoltageMaxDaily READ batteryVoltageMaxDaily WRITE setBatteryVoltageMaxDaily NOTIFY batteryVoltageMaxDailyChanged)
    Q_PROPERTY(double batteryVoltageMinDaily READ batteryVoltageMinDaily WRITE setBatteryVoltageMinDaily NOTIFY batteryVoltageMinDailyChanged)
    Q_PROPERTY(double batteryTemperature READ batteryTemperature WRITE setBatteryTemperature NOTIFY batteryTemperatureChanged)
    Q_PROPERTY(double chargingCurrent READ chargingCurrent WRITE setChargingCurrent NOTIFY chargingCurrentChanged)
    Q_PROPERTY(double outputPower READ outputPower WRITE setOutputPower NOTIFY outputPowerChanged)
    Q_PROPERTY(double arrayCurrent READ arrayCurrent WRITE setArrayCurrent NOTIFY arrayCurrentChanged)
    Q_PROPERTY(double arrayVoltage READ arrayVoltage WRITE setArrayVoltage NOTIFY arrayVoltageChanged)
    Q_PROPERTY(double arrayVoltageMaxDaily READ arrayVoltageMaxDaily WRITE setArrayVoltageMaxDaily NOTIFY arrayVoltageMaxDailyChanged)
    Q_PROPERTY(double powerMaxDaily READ powerMaxDaily WRITE setPowerMaxDaily NOTIFY powerMaxDailyChanged)
    Q_PROPERTY(double wattHoursDaily READ wattHoursDaily WRITE setWattHoursDaily NOTIFY wattHoursDailyChanged)
    Q_PROPERTY(int chargeState READ chargeState WRITE setChargeState NOTIFY chargeStateChanged)
    Q_PROPERTY(int timeInAbsorption READ timeInAbsorption WRITE setTimeInAbsorption NOTIFY timeInAbsorptionChanged)
    Q_PROPERTY(int timeInFloat READ timeInFloat WRITE setTimeInFloat NOTIFY timeInFloatChanged)

public:
    Tsmppt(const QString &IPAddress, const int port = 502, int slave = 1, QObject *parent = 0);
    ~Tsmppt();

    double batteryVoltage() const;
    void setBatteryVoltage(double v);

    double batteryVoltageMaxDaily() const;
    void setBatteryVoltageMaxDaily(double v);

    double batteryVoltageMinDaily() const;
    void setBatteryVoltageMinDaily(double v);

    double batteryTemperature() const;
    void setBatteryTemperature(double v);

    double chargingCurrent() const;
    void setChargingCurrent(double v);

    double outputPower() const;
    void setOutputPower(double v);

    double arrayCurrent() const;
    void setArrayCurrent(double v);

    double arrayVoltage() const;
    void setArrayVoltage(double v);

    double arrayVoltageMaxDaily() const;
    void setArrayVoltageMaxDaily(double v);

    double powerMaxDaily() const;
    void setPowerMaxDaily(double v);

    double wattHoursDaily() const;
    void setWattHoursDaily(double v);

    int chargeState() const;
    void setChargeState(int v);

    int timeInAbsorption() const;
    void setTimeInAbsorption(int v);

    int timeInFloat() const;
    void setTimeInFloat(int v);

    QString firmwareVersion() const;
    QString hardwareVersion() const;
    uint64_t serialNumber() const;
    QString productName() const;

    bool initialize();

signals:
    void batteryVoltageChanged();
    void batteryVoltageMaxDailyChanged();
    void batteryVoltageMinDailyChanged();
    void batteryTemperatureChanged();
    void chargingCurrentChanged();
    void outputPowerChanged();
    void arrayCurrentChanged();
    void arrayVoltageChanged();
    void arrayVoltageMaxDailyChanged();
    void powerMaxDailyChanged();
    void wattHoursDailyChanged();
    void chargeStateChanged();
    void tsmpptConnected();
    void connectionLost();
    void timeInAbsorptionChanged();
    void timeInFloatChanged();

private slots:
    void onTimeout();

private:
    void readInputRegisters(int addr, int nb, uint16_t *dest);
    void updateValues();
    bool mInitialized;
    QTimer *mTimer;
    modbus_t *mCtx;
    uint16_t *mModbusRegs;
};

#endif // TSMPPT_H
