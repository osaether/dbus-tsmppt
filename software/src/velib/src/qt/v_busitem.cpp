#include <velib/qt/v_busitem.h>
#include <velib/qt/v_busitems.h>

#include "v_busitem_adaptor.h"
#include "v_busitem_private.h"
#include "v_busitem_private_cons.h"
#include "v_busitem_private_prod.h"

QVariant VBusItem::invalid = QVariant();

VBusItem::VBusItem(QObject *parent) :
	QObject(parent),
	mBind(""),
	d_ptr(0)
{
}

VBusItem::~VBusItem()
{
	delete d_ptr;
}

VBusItem::VBusItem(QDBusConnection& cnx, QString str, QString description, QVariant value, QString unit, QObject *parent) :
	QObject(parent),
	mBind(""),
	d_ptr(0)
{
	produce(cnx, str, description, value, unit);
}

// Produce the busitem on the dbus so other can read / set it..
bool VBusItem::produce(QDBusConnection& cnx, QString str, QString description, QVariant value, QString unit, int precision)
{
	VBusItemPrivateProd *d = new VBusItemPrivateProd(this);
	delete d_ptr;
	d_ptr = d;
	return d->qProduce(cnx, str, description, value, unit, precision);
}

bool VBusItem::consume(QDBusConnection& cnx, QString service, QString path)
{
	VBusItemPrivateCons *d = new VBusItemPrivateCons(this);
	delete d_ptr;
	d_ptr = d;
	mBind = service + path;
	return d->qConsume(cnx, service, path);
}

bool VBusItem::consume(QString service, QString path)
{
	return consume(VBusItems::getConnection(), service, path);
}

QVariant VBusItem::getValue()
{
	Q_D(VBusItem);

	if (!d)
		return VBusItem::invalid;

	return d->qGetValue();
}

int VBusItem::setValue(QVariant const& value)
{
	Q_D(VBusItem);

	if (!d)
		return -1;

	return d->qSetValue(value);
}

QString VBusItem::getText()
{
	Q_D(VBusItem);

	if (!d)
		return VBusItem::invalid.toString();

	return d->qGetText();
}

QVariant VBusItem::getMin()
{
	Q_D(VBusItem);

	if (!d)
		return VBusItem::invalid;

	return d->qGetMin();
}

QVariant VBusItem::getMax()
{
	Q_D(VBusItem);

	if (!d)
		return VBusItem::invalid;

	return d->qGetMax();
}

QVariant VBusItem::getDefault()
{
	Q_D(VBusItem);

	if (!d)
		return VBusItem::invalid;

	return d->qGetDefault();
}

void VBusItem::setDefault()
{
	Q_D(VBusItem);

	if (d)
		d->qSetDefault();
}

/*
 * 1 Bus names that start with a colon (':') character are unique
 *   connection names. Other bus names are called well-known bus names.
 * 2 Bus names are composed of 1 or more elements separated by a period
 *   ('.') character. All elements must contain at least one character.
 * 3 Each element must only contain the ASCII characters "[A-Z][a-z][0-9]_-".
 *   Only elements that are part of a unique connection name may begin with
 *   a digit, elements in other bus names must not begin with a digit.
 * 4 Bus names must contain at least one '.' (period) character
 *   (and thus at least two elements).
 * 5 Bus names must not begin with a '.' (period) character.
 * 6 Bus names must not exceed the maximum name length.
 */

bool VBusItem::isValidBusName(QString busname)
{
	/* 4, 5, 6 */
	if (busname.length() == 0 || busname.length() > 255 || busname.indexOf('.') < 0 || busname.startsWith('.'))
		return false;

	/* 1 */
	bool unique = busname.startsWith(":");
	if (unique)
		busname = busname.mid(1);

	/* 2 at least one character */
	if (busname.indexOf("...") >= 0)
		return false;

	/* 2 remaining, 3 */
	QStringList elements = busname.split(".");
	QRegExp rx(QString("^[A-Za-z0-9_\\-]") + (unique ? "*" : "+") + "$");
	foreach (QString element, elements) {
		if (rx.indexIn(element) != 0)
			return false;
	}

	return true;
}


void VBusItem::setBind(QString const &path)
{
	if (mBind == path)
		return;
	mBind = path;

	if (!path.length())
		return;

	int pos = path.indexOf('/', 0);
	QString service = path.left(pos);
	QString objectPath = path.mid(pos);

	/*
	 * Don't rely on these checked, just don't pass invalid paths!!!
	 * Behaviour is undetermined when doing so...
	 */
	if (QDBusObjectPath(objectPath).path().isNull()) {
		qDebug() << "Error: invalid objectpath: " << path;
		goto error;
	}

	if (!isValidBusName(service)) {
		qDebug() << "Error: invalid busname: " << service;
		goto error;
	}

	consume(VBusItems::getConnection(), service, objectPath);

	/*
	 * QML relies on the binding for an Item to be created. It does therefore
	 * not really matter in which order properties are set / get since they will
	 * emit change signal events when updated. In this case the values can only be
	 * obtained the dbus path is known, so get the properties now, if it was
	 * requested before (and that will emit the onchange signal).
	 */
	emit valueChanged();
	emit textChanged();
	emit minChanged();
	emit maxChanged();
	emit defaultChanged();
	emit bindChanged();
	emit descriptionChanged();

	//qDebug() << "Called the C++ method with" << service << ":" << objectPath;
	return;

error:
	mBind = "";
	emit bindChanged();
}
