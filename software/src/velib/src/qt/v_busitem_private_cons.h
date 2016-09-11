#ifndef V_BUSITEM_PRIVATE_CONS_H
#define V_BUSITEM_PRIVATE_CONS_H

#include "v_busitem_private.h"
#include "v_busitem_proxy.h"

class VBusItemPrivateCons : public VBusItemPrivate
{
public:
	VBusItemPrivateCons(VBusItem *parent);
	~VBusItemPrivateCons();

	bool qConsume(const QDBusConnection &connection, const QString &service, const QString &path);

	virtual QVariant qGetValue();
	virtual int qSetValue(QVariant const& value);
	virtual QString qGetText();
	virtual QVariant qGetMin();
	virtual QVariant qGetMax();
	virtual QVariant qGetDefault();
	virtual int qSetDefault();

public slots:
	virtual void PropertiesChanged(const QVariantMap &changes);
	virtual void serviceOwnerChanged(QString,QString,QString);

	virtual void valueObtained(QDBusPendingCallWatcher *call);
	virtual void textObtained(QDBusPendingCallWatcher *call);
	virtual void minObtained(QDBusPendingCallWatcher *call);
	virtual void maxObtained(QDBusPendingCallWatcher *call);
	virtual void descriptionObtained(QDBusPendingCallWatcher *call);
	virtual void setValueDone(QDBusPendingCallWatcher *call);
	virtual void defaultObtained(QDBusPendingCallWatcher *call);
	virtual void setDefaultDone(QDBusPendingCallWatcher *call);

private:
	void demarshallVariantForQml(QVariant &variant);

	void invalidateProperties();
	void invalidatePropertiesAndSignal();

	VBusItemProxy *mProxy;
	QDBusServiceWatcher *mWatcher;

	QVariant mValue;
	bool mValueObtained;

	QString mText;
	bool mTextObtained;

	QString mDescription;
	bool mDescriptionObtained;

	QVariant mMin;
	bool mMinObtained;

	QVariant mMax;
	bool mMaxObtained;

	QVariant mDefault;
	bool mDefaultObtained;
};

#endif
