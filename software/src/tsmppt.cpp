#include <errno.h>
#include <QsLog.h>
#include <qtimer.h>
#include "tsmppt.h"

const int REG_V_PU          = 0;
const int REG_I_PU          = 2;
const int REG_VER_SW        = 4;
const int REG_FIRST_DYN     = 24;
const int REG_V_BAT         = 24;
const int REG_V_PV          = 27;
const int REG_I_PV          = 29;
const int REG_I_CC          = 28;
const int REG_T_BAT         = 37;
const int REG_I_CC_1M       = 39;
const int REG_CHARGE_STATE  = 50;
const int REG_KWH_TOTAL_RES = 56;
const int REG_KWH_TOTAL     = 57;
const int REG_POUT          = 58;
const int REG_V_BAT_MIN     = 64;
const int REG_V_BAT_MAX     = 65;
const int REG_V_PV_MAX      = 66;
const int REG_WHC_DAILY     = 68;
const int REG_POUT_MAX_DAILY= 70;
const int REG_T_FLOAT       = 79;
const int REG_T_ABS         = 77;
const int REG_LAST_DYN      = 79;
const int REG_EHW_VERSION   = 57549;
const int REG_ESERIAL       = 57536;
const int REG_EMODEL        = 57548;

const int CS_NIGHT          = 3;
const int CS_BULK           = 5;


Tsmppt::Tsmppt(const QString &IPAddress, const int port, int interval, int slave, QObject *parent):
QObject(parent), mInitialized(false), mTimer(new QTimer(this)), m_interval(interval), m_cs(0), m_t_bulk(1),
m_t_bulk_ms(0), yield_user(0), yield_system(0)
{
    QLOG_DEBUG() << "Tsmppt::Tsmppt(" << IPAddress << ", " << port << ", " << interval << ", " << slave << ")";
    mCtx = modbus_new_tcp_pi(IPAddress.toStdString().c_str(), QString::number(port).toStdString().c_str());
    Q_ASSERT(mCtx != NULL);
    // modbus_set_debug(mCtx, true);
    modbus_set_error_recovery(mCtx, MODBUS_ERROR_RECOVERY_LINK);
#if LIBMODBUS_VERSION_CHECK(3, 1, 1)
    modbus_set_response_timeout(mCtx, 20, 0);
    modbus_set_byte_timeout(mCtx, 10, 0);
#else
    struct timeval to;
    to.tv_sec = 20;
    to.tv_usec = 0;
    modbus_set_response_timeout(mCtx, &to);
    to.tv_sec = 10;
    modbus_set_byte_timeout(mCtx, &to);
#endif
    modbus_set_slave(mCtx, slave);
    mTimer->setInterval(m_interval);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

Tsmppt::~Tsmppt()
{
    QLOG_DEBUG() << "Tsmppt::~Tsmppt()";
    modbus_free(mCtx);
}

bool Tsmppt::readInputRegisters(int addr, int nb, uint16_t *dest)
{
    int tries = 5;
    while (tries > 0)
    {
        if(modbus_read_input_registers(mCtx, addr, nb, dest) != -1)
        {
            return true;
        }
        if (tries > 1)
        {
            QLOG_ERROR() << "MODBUS:" << modbus_strerror(errno) << "Retrying (" << 5-tries+1 << ")...";
        }
        tries--;
    }
    modbus_close(mCtx);
    QLOG_ERROR() << "MODBUS:" << modbus_strerror(errno) << "Trying to reconnect";
    emit connectionLost();
    return false;
}

bool Tsmppt::initialize()
{
    Q_ASSERT(mCtx != NULL);

    QLOG_DEBUG() << "Tsmppt::initialize(start)";

    if (modbus_connect(mCtx) == -1)
    {
        QLOG_ERROR() << "MODBUS:" << modbus_strerror(errno);
        return false;
    }

    uint16_t regs[6];

    if (!readInputRegisters(REG_V_PU, 6, regs))
    {
        modbus_close(mCtx);
        return false;
    }
    
    // Voltage scaling:
    m_v_pu = (float)regs[1];
    m_v_pu /= 65536.0;
    m_v_pu += (float)regs[0];

    // Current scaling:
    m_i_pu = (float)regs[3];
    m_i_pu /= 65536.0;
    m_i_pu += (float)regs[2];
    QLOG_DEBUG() << "Tsmppt: m_v_pu =" << m_v_pu << " m_i_pu =" << m_i_pu;

    // Firmware version:
    uint16_t ver = ((regs[4] >> 12) & 0x0f) * 1000;
    ver += ((regs[4] >> 8) & 0x0f) * 100;
    ver += ((regs[4] >> 4) & 0x0f) * 10;
    ver += regs[4] & 0x0f;
    setFirmwareVersion(ver);

    // Hardware version:
    if (!readInputRegisters(REG_EHW_VERSION, 1, regs))
    {
        modbus_close(mCtx);
        return false;
    }
    setHardwareVersion(QString::number(regs[0] >> 8) + "." + QString::number(regs[0] & 0xff));

    // Model
    if (!readInputRegisters(REG_EMODEL, 1, regs))
    {
        modbus_close(mCtx);
        return false;
    }
    QString name;
    switch (regs[0])
    {
        case 0:
            name = "TriStar MPPT 45";
            break;

        case 1:
            name = "TriStar MPPT 60";
            break;

        case 2:
            name = "TriStar MPPT 30";
            break;

        default:
            name = "";
    }
    setProductName(name);

    // Serial number:
    if (!readInputRegisters(REG_ESERIAL, 4, regs))
    {
        modbus_close(mCtx);
        return false;
    }
    uint64_t serial = (uint64_t)((regs[0] & 0xff) - 0x30) * 10000000;
    serial += (uint64_t)((regs[0] >> 8) - 0x30) * 1000000;
    serial += (uint64_t)((regs[1] & 0xff) - 0x30) * 100000;
    serial += (uint64_t)((regs[1] >> 8) - 0x30) * 10000;
    serial += (uint64_t)((regs[2] & 0xff) - 0x30) * 1000;
    serial += (uint64_t)((regs[2] >> 8) - 0x30) * 100;
    serial += (uint64_t)((regs[3] & 0xff) - 0x30) * 10;
    serial += (uint64_t)((regs[3] >> 8) - 0x30);
    setSerialNumber(QString::number(serial));


    modbus_close(mCtx);
    mInitialized = true;
    emit tsmpptConnected();
    QLOG_DEBUG() << "Tsmppt::initialize(end)";
    QString logmsg = productName() + " (serial #" + serialNumber() + ", controler v" + hardwareVersion() + "." + firmwareVersion() + ") connected";
    QLOG_INFO() << logmsg.toStdString().c_str();
    return true;
}

void Tsmppt::updateValues()
{
    uint16_t reg[REG_LAST_DYN-REG_FIRST_DYN+1];

    QLOG_DEBUG() << "Tsmppt::updateValues()";

    if (modbus_connect(mCtx) == -1)
    {
        QLOG_ERROR() << "MODBUS:" << modbus_strerror(errno);
        return;
    }

    if (readInputRegisters(REG_FIRST_DYN, REG_LAST_DYN-REG_FIRST_DYN+1, reg))
    {
        // Battery voltage:
        double temp = (double)reg[REG_V_BAT-REG_FIRST_DYN] * m_v_pu / 32768.0;
        setBatteryVoltage(temp);

        // Max Battery voltage:
        temp = (double)reg[REG_V_BAT_MAX-REG_FIRST_DYN] * m_v_pu / 32768.0;
        setBatteryVoltageMaxDaily(temp);

        // Min Battery voltage:
        temp = (double)reg[REG_V_BAT_MIN-REG_FIRST_DYN] * m_v_pu / 32768.0;
        setBatteryVoltageMinDaily(temp);

        // Battery temperature:
        temp = (double)(int16_t)reg[REG_T_BAT-REG_FIRST_DYN];
        setBatteryTemperature(temp);

        // Charge current:
        temp = (double)(int16_t)reg[REG_I_CC_1M-REG_FIRST_DYN] * m_i_pu / 32768.0;
        if (temp < 0.0)
            temp = 0.0;
        setChargingCurrent(temp);
        
        // MPPT output power:
        temp = (double)reg[REG_POUT-REG_FIRST_DYN] * m_i_pu * m_v_pu / 131072.0;
        setOutputPower(temp);

        // PV array voltage:
        temp = (double)reg[REG_V_PV-REG_FIRST_DYN]  * m_v_pu / 32768.0;
        setArrayVoltage(temp);

        // Max PV array voltage:
        temp = (double)reg[REG_V_PV_MAX-REG_FIRST_DYN]  * m_v_pu / 32768.0;
        setArrayVoltageMaxDaily(temp);

        // PV array current:
        temp = (double)reg[REG_I_PV-REG_FIRST_DYN] * m_i_pu / 32768.0;
        setArrayCurrent(temp);

        // Whc daily:
        temp = (double)reg[REG_WHC_DAILY-REG_FIRST_DYN];
        // CCGX expects kWh:
        temp /= 1000.0;
        setWattHoursDaily(temp);

        // Whc total:
        temp = (double)reg[REG_KWH_TOTAL_RES-REG_FIRST_DYN];
        setWattHoursTotalResettable(temp);
        setYieldUser(m_whc+temp);

        // Whc total:
        temp = (double)reg[REG_KWH_TOTAL-REG_FIRST_DYN];
        setWattHoursTotal(temp);
        setYieldSystem(m_whc+temp);

        // Pmax daily:
        temp = (double)reg[REG_POUT_MAX_DAILY-REG_FIRST_DYN] * m_i_pu * m_v_pu / 131072.0;
        setPowerMaxDaily(temp);

        // Charge state:
        setChargeState(reg[REG_CHARGE_STATE-REG_FIRST_DYN]);

        if (m_cs == CS_BULK)
            m_t_bulk_ms += m_interval;
        else if (m_cs == CS_NIGHT)
            m_t_bulk_ms = 0;
        setTimeInBulk((int)(m_t_bulk_ms/(1000*60)));
        setTimeInAbsorption(reg[REG_T_ABS-REG_FIRST_DYN]/60);
        setTimeInFloat(reg[REG_T_FLOAT-REG_FIRST_DYN]/60);
    }
    modbus_close(mCtx);
}

int Tsmppt::firmwareVersion() const
{
    return m_fw_ver;
}

void Tsmppt::setFirmwareVersion(int v)
{
     if (m_fw_ver == v)
        return;
    m_fw_ver = v;
    emit firmwareVersionChanged();
}

QString Tsmppt::hardwareVersion() const
{
    return m_hw_ver;
}

void Tsmppt::setHardwareVersion(QString v)
{
    if (m_hw_ver == v)
        return;
    m_hw_ver = v;
    emit hardwareVersionChanged();
}

QString Tsmppt::serialNumber() const
{
    return m_serial;
}

void Tsmppt::setSerialNumber(QString v)
{
    if (m_serial == v)
        return;
    m_serial = v;
    emit serialNumberChanged();
}

double Tsmppt::batteryVoltage() const
{
    return m_v_bat;
}

void Tsmppt::setBatteryVoltage(double v)
{
    if (m_v_bat == v)
        return;
    m_v_bat = v;
    emit batteryVoltageChanged();
}

double Tsmppt::batteryVoltageMaxDaily() const
{
    return m_v_bat_max;
}

void Tsmppt::setBatteryVoltageMaxDaily(double v)
{
    if (m_v_bat_max == v)
        return;
    m_v_bat_max = v;
    emit batteryVoltageMaxDailyChanged();
}

double Tsmppt::batteryVoltageMinDaily() const
{
    return m_v_bat_min;
}

void Tsmppt::setBatteryVoltageMinDaily(double v)
{
    if (m_v_bat_min == v)
        return;
    m_v_bat_min = v;
    emit batteryVoltageMinDailyChanged();
}

double Tsmppt::batteryTemperature() const
{
    return m_t_bat;
}

void Tsmppt::setBatteryTemperature(double v)
{
    if (m_t_bat == v)
        return;
    m_t_bat = v;
    emit batteryTemperatureChanged();
}

double Tsmppt::chargingCurrent() const
{
    return m_i_cc;
}

double Tsmppt::outputPower() const
{
    return m_pout;
}

void Tsmppt::setChargingCurrent(double v)
{
    if (m_i_cc == v)
        return;
    m_i_cc = v;
    emit chargingCurrentChanged();
}

void Tsmppt::setOutputPower(double v)
{
    if (m_pout == v)
        return;
    m_pout = v;
    emit outputPowerChanged();
}

double Tsmppt::arrayCurrent() const
{
    return m_i_pv;
}

void Tsmppt::setArrayCurrent(double v)
{
    if (m_i_pv == v)
        return;
    m_i_pv = v;
    emit arrayCurrentChanged();
}

double Tsmppt::arrayVoltage() const
{
    return m_v_pv;
}

void Tsmppt::setArrayVoltage(double v)
{
    if (m_v_pv == v)
        return;
    m_v_pv = v;
    emit arrayVoltageChanged();
}

double Tsmppt::arrayVoltageMaxDaily() const
{
    return m_v_pv_max;
}

void Tsmppt::setArrayVoltageMaxDaily(double v)
{
    if (m_v_pv_max == v)
        return;
    m_v_pv_max = v;
    emit arrayVoltageMaxDailyChanged();
}

double Tsmppt::powerMaxDaily() const
{
    return m_p_max;
}

void Tsmppt::setPowerMaxDaily(double v)
{
    if (m_p_max == v)
        return;
    m_p_max = v;
    emit powerMaxDailyChanged();
}

double Tsmppt::wattHoursDaily() const
{
    return m_whc;
}

void Tsmppt::setWattHoursDaily(double v)
{
    if (m_whc == v)
        return;
    m_whc = v;
    emit wattHoursDailyChanged();
}

double Tsmppt::wattHoursTotal() const
{
    return m_whc_tot;
}

void Tsmppt::setWattHoursTotal(double v)
{
    if (m_whc_tot == v)
        return;
    m_whc_tot = v;
    emit wattHoursTotalChanged();
}

double Tsmppt::wattHoursTotalResettable() const
{
    return m_whc_tot_res;
}

void Tsmppt::setWattHoursTotalResettable(double v)
{
    if (m_whc_tot_res == v)
        return;
    m_whc_tot_res = v;
    emit wattHoursTotalResettableChanged();
}

int Tsmppt::chargeState() const
{
    // Will translate the TSMPPT charge state according to this table:
    // TSMPPT:              BlueSolar: 
    // 0 START              0 OFF
    // 1 NIGHT_CHECK        0 OFF
    // 2 DISCONNECT         0 OFF
    // 3 NIGHT              0 OFF
    // 4 FAULT              2 FAULT
    // 5 MPPT               3 BULK
    // 6 ABSORPTION         4 ABSORPTION
    // 7 FLOAT              5 FLOAT
    // 8 EQUALIZE           7 EQUALIZE
    // 9 SLAVE              11 OTHER
    const int bscs[10] = {0,0,0,0,2,3,4,5,7,11};
    return bscs[m_cs];
}

void Tsmppt::setChargeState(int v)
{
    if (m_cs == v)
        return;
    m_cs = v;
    emit chargeStateChanged();
}

int Tsmppt::timeInAbsorption() const
{
    return m_t_abs;
}

void Tsmppt::setTimeInAbsorption(int v)
{
    if (m_t_abs == v)
        return;
    m_t_abs = v;
    emit timeInAbsorptionChanged();
}

int Tsmppt::timeInFloat() const
{
    return m_t_float;
}


void Tsmppt::setTimeInBulk(int v)
{
    if (m_t_bulk == v)
        return;
    m_t_bulk = v;
    emit timeInBulkChanged();
}

int Tsmppt::timeInBulk() const
{
    return m_t_bulk;
}

void Tsmppt::setYieldUser(double v)
{
    if (yield_user == v)
        return;
    yield_user = v;
    emit yieldUserChanged();
}

double Tsmppt::yieldUser() const
{
    return yield_user;
}

void Tsmppt::setYieldSystem(double v)
{
    if (yield_system == v)
        return;
    yield_system = v;
    emit yieldSystemChanged();
}

double Tsmppt::yieldSystem() const
{
    return yield_system;
}

void Tsmppt::setTimeInFloat(int v)
{
    if (m_t_float == v)
        return;
    m_t_float = v;
    emit timeInFloatChanged();
}

QString Tsmppt::productName() const
{
    return m_name;
}


void Tsmppt::setProductName(QString v)
{
    if (m_name == v)
        return;
    m_name = v;
    emit productNameChanged();
}

void Tsmppt::onTimeout()
{
    mTimer->stop();
    if (!mInitialized)
    {
        initialize();
    }
    else
    {
        updateValues();
    }
    mTimer->start();
}

void Tsmppt::startLogging()
{
    mTimer->start();
}
