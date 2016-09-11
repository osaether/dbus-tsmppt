#include <unistd.h>
#include <QCoreApplication>
#include <QsLog.h>
#include <QStringList>
#include <velib/qt/v_busitem.h>
#include <velib/qt/v_busitems.h>
#include "dbus_tsmppt.h"

void initLogger(QsLogging::Level logLevel)
{
    QsLogging::Logger &logger = QsLogging::Logger::instance();
    QsLogging::DestinationPtr debugDestination(
            QsLogging::DestinationFactory::MakeDebugOutputDestination());
    logger.addDestination(debugDestination);
    logger.setIncludeTimestamp(false);

    QLOG_INFO() << "dbus-tsmppt" << "v"VERSION << "started";
    QLOG_INFO() << "Built with Qt" << QT_VERSION_STR << "running on" << qVersion();
    QLOG_INFO() << "Built on" << __DATE__ << "at" << __TIME__;
    logger.setLoggingLevel(logLevel);
}


void initDBus(const QString &dbusAddress)
{
    VBusItems::setDBusAddress(dbusAddress);

    QLOG_INFO() << "Wait for local settings on DBus(" << dbusAddress << ")...";
    VBusItem settings;
    settings.consume("com.victronenergy.settings", "/Settings/TristarMPPT/IPAddress");
    for (;;) {
        QVariant reply = settings.getValue();
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        if (reply.isValid())
            break;
        usleep(500000);
        QLOG_INFO() << "Waiting...";
    }
    QLOG_INFO() << "Local settings found";
}

bool addSetting(const QString &path,
                const QVariant &defaultValue,
                const QVariant &minValue,
                const QVariant &maxValue)
{
    if (!path.startsWith("/Settings"))
        return false;
    int groupStart = path.indexOf('/', 1);
    if (groupStart == -1)
        return false;
    int nameStart = path.lastIndexOf('/');
    if (nameStart <= groupStart)
        return false;
    QChar type;
    switch (defaultValue.type()) {
    case QVariant::Int:
        type = 'i';
        break;
    case QVariant::Double:
        type = 'f';
        break;
    case QVariant::String:
        type = 's';
        break;
    default:
        return false;
    }
    QString group = path.mid(groupStart + 1, nameStart - groupStart - 1);
    QString name = path.mid(nameStart + 1);
    QDBusConnection &connection = VBusItems::getConnection();
    QDBusMessage m = QDBusMessage::createMethodCall(
                         "com.victronenergy.settings",
                         "/Settings",
                         "com.victronenergy.Settings",
                         "AddSetting")
                     << group
                     << name
                     << QVariant::fromValue(QDBusVariant(defaultValue))
                     << QString(type)
                     << QVariant::fromValue(QDBusVariant(minValue))
                     << QVariant::fromValue(QDBusVariant(maxValue));
    QDBusMessage reply = connection.call(m);
    return reply.type() == QDBusMessage::ReplyMessage;
}

extern "C"
{

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationVersion(VERSION);

    initLogger(QsLogging::InfoLevel);

    bool expectVerbosity = false;
    bool expectDBusAddress = false;
    QString dbusAddress = "system";
    QStringList args = app.arguments();
    args.pop_front();
    foreach (QString arg, args) {
        if (expectVerbosity) {
            QsLogging::Logger &logger = QsLogging::Logger::instance();
            QsLogging::Level logLevel = static_cast<QsLogging::Level>(qBound(
                static_cast<int>(QsLogging::TraceLevel),
                arg.toInt(),
                static_cast<int>(QsLogging::OffLevel)));
            logger.setLoggingLevel(logLevel);
            expectVerbosity = false;
        } else if (expectDBusAddress) {
            dbusAddress = arg;
            expectDBusAddress = false;
        } else if (arg == "-h" || arg == "--help") {
            QLOG_INFO() << app.arguments().first();
            QLOG_INFO() << "\t-h, --help";
            QLOG_INFO() << "\t Show this message.";
            QLOG_INFO() << "\t-V, --version";
            QLOG_INFO() << "\t Show the application version.";
            QLOG_INFO() << "\t-d level, --debug level";
            QLOG_INFO() << "\t Set log level";
            QLOG_INFO() << "\t-b, --dbus";
            QLOG_INFO() << "\t dbus address or 'session' or 'system'";
            exit(1);
        } else if (arg == "-V" || arg == "--version") {
            QLOG_INFO() << VERSION;
            exit(0);
        } else if (arg == "-d" || arg == "--debug") {
            expectVerbosity = true;
        } else if (arg == "-t" || arg == "--timestamp") {
            QsLogging::Logger &logger = QsLogging::Logger::instance();
            logger.setIncludeTimestamp(true);
        } else if (arg == "-b" || arg == "--dbus") {
            expectDBusAddress = true;
        }
    }

    //addSetting("/Settings/TristarMPPT/IPAddress", "", 0, 0);
    //addSetting("/Settings/TristarMPPT/PortNumber", 502, 0, 0);
    initDBus(dbusAddress);

    DBusTsmppt a(0);

    app.connect(&a, SIGNAL(terminateApp()), &app, SLOT(quit()));

    return app.exec();
}
}

