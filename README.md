dbus-tsmppt
===========

This application reads essential charging data from a Morningstar Tristar MPPT charge controller and publish it on the D-Bus. It is designed to run on the Victron Energy Venus OS but can easily be modified to run on other systems.

Building
========

In order to build the application for the CCGX you need a linux system, a recent version of the QT libraries and the CCGX SDK. You can find instructions on installing the CCGX SDK here:

https://github.com/victronenergy/venus/wiki/howto-install-and-use-the-sdk

You will also need libmodbus v3.1.2 or higher (http://libmodbus.org/).

Clone repo with `git clone --recursive https://github.com/osaether/dbus-tsmppt.git`

    cd software
    qmake LIBS+=-L/usr/local/lib/ INCLUDEPATH+=/usr/local/include/modbus
    make

Change LIBS and INCLUDEPATH above according to your installation.

Running the application on CCGX
===============================

The application expect the IP-address/hostname and the Modbus IP-port number stored in the CCGX settings. You can either edit the /data/conf/settings.xml file manually (not recommended) or use the CCGX settings menu. To use the settings menu copy the file PageSettingsTsmppt.qml in the qml folder to the folder on CCGX where the qml files are stored (usually /opt/color-control/gui/qml). Then add a link to PageSettingsTsmppt.qml in the main settings qml-file, PageSettings.qml found in the same folder:

    MbSubMenu {
        description: qsTr("TriStar MPPT 60 Solar Charger")
        subpage: Component { PageSettingsTsmppt {} }
    }

Change "TriStar MPPT 60" to "TriStar MPPT 45" or "TriStar MPPT 30" according to the Tristar MPPT version you have.

To display the data in the CCGX gui you also need to add these lines to the isModelSupported function in PageSolarCharger.qml:

    /* Morningstar TriStar MPPT */
    if (productId.value === 0xABCD)
        return true

Testing on Linux
================

To compile and run on linux you will need the QT SDK (version 4.8.x), including QT D-Bus support. Because you do not have access to the system D-Bus (unless you run as root or adjust the D-Bus configuration) you should start the application with: 'dbus-tsmppt --dbus session'. Note that QT for windows does not support D-Bus, so you cannot build a windows executable.

The dbus-tsmppt executable expects the CCGX settings manager (localsettings) to be running. localsettings is available on github:

https://github.com/victronenergy/localsettings

The README.md of localsettings contains some information on how to run localsettings on linux.

