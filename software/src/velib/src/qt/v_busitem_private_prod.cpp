#include "v_busitem_private_prod.h"

VBusItemPrivateProd::VBusItemPrivateProd(VBusItem *parent) :
	VBusItemPrivate(parent),
	mAdp(0),
	mPrecision(-1)
{
}

// value should be produced on the dbus.
bool VBusItemPrivateProd::qProduce(QDBusConnection& cnx, QString& name, QString& description, QVariant& value,
								QString& unit, int precision)
{
	if (mAdp)
		return false;

	mDescription = description;
	mValue = value;
	mUnit = unit;
	mPrecision = precision;

	mAdp = new VBusItemAdaptor(this);

	return cnx.registerObject(name, this);
}

QVariant VBusItemPrivateProd::qGetValue()
{
	return mValue;
}

int VBusItemPrivateProd::qSetValue(const QVariant &value)
{
	Q_Q(VBusItem);

	if (mValue == value)
		return 0;

	mValue = value;
	emit q->valueChanged();

	QVariantMap map;
	map.insert("Value", value);
	map.insert("Text", qGetText());
	emit PropertiesChanged(map);

	return 0;
}

QString VBusItemPrivateProd::qGetText()
{
	QString s;
	if (mPrecision >= 0 && mValue.type() == QVariant::Double) {
		s.setNum(mValue.toDouble(), 'f', mPrecision);
	} else {
		s = mValue.toString();
	}
	if (!s.isEmpty())
		s += mUnit;
	return s;
}

/*==== DBUS ====*/

// Request from dbus
QDBusVariant VBusItemPrivateProd::GetValue()
{
	return QDBusVariant(mValue);
}

QString VBusItemPrivateProd::GetText()
{
	return qGetText();
}

// Update from dbus.
int VBusItemPrivateProd::SetValue(const QDBusVariant &value)
{
	return qSetValue(value.variant());
}

// Request from dbus
QString VBusItemPrivateProd::GetDescription(const QString &language, int length)
{
	Q_UNUSED(language)
	Q_UNUSED(length)

	return mDescription;
}
