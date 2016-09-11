#ifndef _VELIB_QT_V_BUSITEM_PRIVATE_H_
#define _VELIB_QT_V_BUSITEM_PRIVATE_H_

#include <QObject>
#include <velib/qt/v_busitem.h>

/**
 * A busitem can be synchronized over DBus. The Qt implementation is created as a
 * PIMPel class. To the outside world the class uses Qt/QML datatypes and can be
 * used in QML in the same way the busitem can be used in python and other dbus
 * aware languages. The private part handles type conversion and onchange message
 * as QML and dbus signals. The DbusAdaptor and DbusProxy are autogenerate stubs
 * from the dbus definition file. Besides the initiation, there should be no
 * distinction if busitems are local or remote values, or used in another language.
 * Schematically the classes relate in the following manner:
 *
 * | -> DBusAdaptor - DBusItemPrivateProd - Dbusitem->produce()
 * | -> DBusProxy	- DbusItemPrivateCons - DbusItem->consume()
 * | -> DBusProxy	- DbusItemPrivateCons - DbusItem->consume()
 * | -> python
 * | -> perl
 * | -> ...
 */

class VBusItemPrivate : public QObject
{
	Q_OBJECT

public:
	VBusItemPrivate(VBusItem *parent) :
		q_ptr(parent)
	{
	}

	~VBusItemPrivate()
	{
	}

	virtual QVariant qGetValue() = 0;
	virtual int qSetValue(QVariant const& value) = 0;
	virtual QString qGetText() = 0;
	virtual QVariant qGetMin() = 0;
	virtual QVariant qGetMax() = 0;
	virtual QVariant qGetDefault() = 0;
	virtual int qSetDefault() = 0;

public slots:
	/*
	 * This is a bit silly, but when not defined here moc seems to be
	 * unable to find them in inherited classes.
	 */

	/* consumer */
	virtual void PropertiesChanged(const QVariantMap &) = 0;
	virtual void serviceOwnerChanged(QString,QString,QString) = 0;

	/* producer, used by the adapter */
	virtual QDBusVariant GetValue() {
		return QDBusVariant();
	}

	virtual QString GetText()
	{
		return "";
	}

	virtual int SetValue(const QDBusVariant &value)
	{
		Q_UNUSED(value);

		return -1;
	}

	virtual QString GetDescription(const QString &language, int length)
	{
		Q_UNUSED(language);
		Q_UNUSED(length);

		return QString();
	}

	/* stubs */
	virtual void valueObtained(QDBusPendingCallWatcher *) {}
	virtual void textObtained(QDBusPendingCallWatcher *) {}
	virtual void minObtained(QDBusPendingCallWatcher *) {}
	virtual void maxObtained(QDBusPendingCallWatcher *) {}
	virtual void descriptionObtained(QDBusPendingCallWatcher *) {}
	virtual void setValueDone(QDBusPendingCallWatcher *) {}
	virtual void defaultObtained(QDBusPendingCallWatcher *) {}
	virtual void setDefaultDone(QDBusPendingCallWatcher *) {}

protected:
	VBusItem * const q_ptr;
	Q_DECLARE_PUBLIC(VBusItem)
};

#endif
