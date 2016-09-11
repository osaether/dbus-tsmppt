#include "v_busitem_private_cons.h"

VBusItemPrivateCons::VBusItemPrivateCons(VBusItem *parent) :
	VBusItemPrivate(parent),
	mProxy(0),
	mWatcher(0)
{
	invalidateProperties();
}

VBusItemPrivateCons::~VBusItemPrivateCons()
{
}

// value should be consumed from the dbus.
bool VBusItemPrivateCons::qConsume(const QDBusConnection &connection, const QString &service,
											const QString &path)
{
	delete mProxy;

	mProxy = new VBusItemProxy(service, path, connection, this);
	connect(mProxy, SIGNAL(PropertiesChanged(const QVariantMap &)), this, SLOT(PropertiesChanged(const QVariantMap &)));

	/* monitor bus on / bus off */
	mWatcher = new QDBusServiceWatcher(service, connection, QDBusServiceWatcher::WatchForOwnerChange, this);
	connect(mWatcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)), this, SLOT(serviceOwnerChanged(QString,QString,QString)));

	return true;
}

/*
 * qtDbus creates a variant of QDBusArgument for all none basic types
 * encountered in case of a QDBusVariant. These can be be casted/demarshalled to
 * a explicit type. However qml does will not perform such casts and complains
 * it can't deal with a QDBusArgument. In order to make this work, the casts are
 * performed here. This is in no way complete. QVariantMap and arrays of basic
 * types are supported.
 *
 * NOTE: The remote c side has no onChange support at the moment (june 2014)
 */
void VBusItemPrivateCons::demarshallVariantForQml(QVariant &variant)
{
	if (variant.canConvert<QDBusArgument>())
	{
		const QDBusArgument &dbusArgument = qvariant_cast<QDBusArgument>(variant);

		switch (dbusArgument.currentType())
		{
		case QDBusArgument::MapType:
			/* QVariantMap support */
			if (dbusArgument.currentSignature() == "a{sv}")
			{
				QVariantMap map = qdbus_cast<QVariantMap>(dbusArgument);
				for (QVariantMap::iterator i = map.begin(); i != map.end(); i++)
					demarshallVariantForQml(i.value());
				variant = QVariant::fromValue(map);
			} else {
				qDebug() << "unsupported MapType, your likely toasted...";
			}
			break;

		case QDBusArgument::ArrayType:
		{
			// An empty array represents an invalid value for busitems,
			// simply because there is no simple method to invalidate
			// a value on the dbus at the moment.
			dbusArgument.beginArray();
			if (dbusArgument.atEnd()) {
				dbusArgument.endArray();
				variant = VBusItem::invalid;
				break;
			}

			// Upcast everything to a QVariantList as qml understands that
			// by default..
			QVariantList list;
			while (!dbusArgument.atEnd())
				list.append(dbusArgument.asVariant());
			dbusArgument.endArray();

			for (QVariantList::iterator i = list.begin(); i != list.end(); i++)
				demarshallVariantForQml(*i);

			variant = QVariant::fromValue(list);
			break;
		}

		default:
			qDebug() << "unsupported QDBusArgument, your likely toasted...";
		}
	}
}

void VBusItemPrivateCons::valueObtained(QDBusPendingCallWatcher *call)
{
	Q_Q(VBusItem);
	QDBusPendingReply<QDBusVariant> reply = *call;

	mValueObtained = true;
	QVariant value = (reply.isError() ? VBusItem::invalid : reply.value().variant());

	/* Treat an empty array as undefined */
	if (!reply.isError()) {
		demarshallVariantForQml(value);
	} else {
		qDebug() << "Error value:" << q->mBind << reply.error();
	}

	if (mValue != value) {
		mValue = value;
		emit q->valueChanged();
	}

	call->deleteLater();
}

QVariant VBusItemPrivateCons::qGetValue()
{
	if (mValueObtained)
		return mValue;

	QDBusPendingReply<QDBusVariant> value = mProxy->GetValue();
	QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(value, this);
	this->connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(valueObtained(QDBusPendingCallWatcher*)));

	return VBusItem::invalid;
}


void VBusItemPrivateCons::minObtained(QDBusPendingCallWatcher *call)
{
	Q_Q(VBusItem);
	QDBusPendingReply<QDBusVariant> reply = *call;

	mMinObtained = true;
	QVariant min = (reply.isError() ? VBusItem::invalid : reply.value().variant());
	demarshallVariantForQml(min);

	if (reply.isError())
		qDebug() << "Error min:" << q->mBind << reply.error();

	if (mMin != min) {\
		mMin = min;
		emit q->minChanged();
	}
	call->deleteLater();
}

QVariant VBusItemPrivateCons::qGetMin()
{
	if (mMinObtained)
		return mMin;

	QDBusPendingReply<QDBusVariant> value = mProxy->GetMin();
	QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(value, this);
	this->connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(minObtained(QDBusPendingCallWatcher*)));

	return VBusItem::invalid;
}

void VBusItemPrivateCons::maxObtained(QDBusPendingCallWatcher *call)
{
	Q_Q(VBusItem);
	QDBusPendingReply<QDBusVariant> reply = *call;

	mMaxObtained = true;
	QVariant max = (reply.isError() ? VBusItem::invalid : reply.value().variant());
	demarshallVariantForQml(max);

	if (reply.isError())
		qDebug() << "Error max:" << q->mBind << reply.error();

	if (mMax != max) {
		mMax = max;
		emit q->maxChanged();
	}
	call->deleteLater();
}

QVariant VBusItemPrivateCons::qGetMax()
{
	if (mMaxObtained)
		return mMax;

	QDBusPendingReply<QDBusVariant> value = mProxy->GetMax();
	QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(value, this);
	this->connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(maxObtained(QDBusPendingCallWatcher*)));

	return VBusItem::invalid;
}

void VBusItemPrivateCons::defaultObtained(QDBusPendingCallWatcher *call)
{
	Q_Q(VBusItem);
	QDBusPendingReply<QDBusVariant> reply = *call;

	mDefaultObtained = true;
	QVariant def = (reply.isError() ? VBusItem::invalid : reply.value().variant());
	demarshallVariantForQml(def);

	if (reply.isError())
		qDebug() << "Error default:" << q->mBind << reply.error();

	if (mDefault != def) {
		mDefault = def;
		emit q->defaultChanged();
	}
	call->deleteLater();
}

QVariant VBusItemPrivateCons::qGetDefault()
{
	if (mDefaultObtained)
		return mDefault;

	QDBusPendingReply<QDBusVariant> value = mProxy->GetDefault();
	QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(value, this);
	this->connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(defaultObtained(QDBusPendingCallWatcher*)));

	return VBusItem::invalid;
}

void VBusItemPrivateCons::textObtained(QDBusPendingCallWatcher *call)
{
	Q_Q(VBusItem);
	QDBusPendingReply<QString> reply = *call;

	mTextObtained = true;
	QString text = (reply.isError() ? VBusItem::invalid.toString() : reply.value());

	if (reply.isError())
		qDebug() << "Error text:" << q->mBind << reply.error();

	if (mText != text) {
		mText = text;
		emit q->textChanged();
	}
	call->deleteLater();
}

QString VBusItemPrivateCons::qGetText()
{
	if (mTextObtained)
		return mText;

	QDBusPendingReply<QString> text = mProxy->GetText();
	QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(text, this);
	this->connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(textObtained(QDBusPendingCallWatcher*)));

	return VBusItem::invalid.toString();
}

void VBusItemPrivateCons::descriptionObtained(QDBusPendingCallWatcher *call)
{
	Q_Q(VBusItem);
	QDBusPendingReply<QString> reply = *call;

	mDescriptionObtained = true;
	QString description = (reply.isError() ? VBusItem::invalid.toString() : reply.value());

	if (reply.isError())
		qDebug() << "Error description:" << q->mBind << reply.error();

	if (mDescription != description) {
		mDescription = description;
		emit q->descriptionChanged();
	}
	call->deleteLater();
}

void VBusItemPrivateCons::setValueDone(QDBusPendingCallWatcher *call)
{
	call->deleteLater();
}

int VBusItemPrivateCons::qSetValue(const QVariant &value)
{
	if (mValue == value)
		return 0;

	QDBusPendingReply<int> set = mProxy->SetValue(QDBusVariant(value));
	QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(set, this);
	this->connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(setValueDone(QDBusPendingCallWatcher*)));

	return 0;
}

void VBusItemPrivateCons::setDefaultDone(QDBusPendingCallWatcher *call)
{
	call->deleteLater();
}

int VBusItemPrivateCons::qSetDefault()
{
	QDBusPendingReply<int> set = mProxy->SetDefault();
	QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(set, this);
	this->connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(setDefaultDone(QDBusPendingCallWatcher*)));

	return 0;
}

void VBusItemPrivateCons::invalidateProperties()
{
	mValueObtained = false;
	mTextObtained = false;
	mDescriptionObtained = false;
	mMinObtained = false;
	mMaxObtained = false;
	mDefaultObtained = false;
}

void VBusItemPrivateCons::invalidatePropertiesAndSignal()
{
	Q_Q(VBusItem);

	invalidateProperties();
	emit q->valueChanged();
	emit q->textChanged();
	emit q->descriptionChanged();
	emit q->minChanged();
	emit q->maxChanged();
	emit q->defaultChanged();
}

void VBusItemPrivateCons::PropertiesChanged(const QVariantMap &changes)
{
	QMapIterator<QString, QVariant> i(changes);
	Q_Q(VBusItem);

	while (i.hasNext()) {
		i.next();
		if (i.key() == "Value") {
			QVariant value(i.value());
			demarshallVariantForQml(value);

			mValueObtained = true;
			if (mValue == value)
				continue;
			mValue = value;

			emit q->valueChanged();

		} else if (i.key() == "Text") {
			QString text(i.value().toString());

			mTextObtained = true;
			if (mText == text)
				continue;
			mText = text;

			emit q->textChanged();

		} else if (mMinObtained && i.key() == "Min") {
			QVariant value(i.value());
			demarshallVariantForQml(value);

			mMinObtained = true;
			if (mMin == value)
				continue;
			mMin = value;

			emit q->minChanged();

		} else if (mMaxObtained && i.key() == "Max") {
			QVariant value(i.value());
			demarshallVariantForQml(value);

			mMaxObtained = true;
			if (mMax == value)
				continue;
			mMax = value;

			emit q->maxChanged();

		} else if (mDefaultObtained && i.key() == "Default") {
			QVariant value(i.value());
			demarshallVariantForQml(value);

			mDefaultObtained = true;
			if (mDefault == value)
				continue;
			mDefault = value;

			emit q->defaultChanged();

		}
	}
}

void VBusItemPrivateCons::serviceOwnerChanged(QString, QString, QString)
{
	invalidatePropertiesAndSignal();
}
