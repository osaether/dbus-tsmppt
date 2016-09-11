#ifndef V_BUSITEM_PRIVATE_PROD_H
#define V_BUSITEM_PRIVATE_PROD_H

#include <QObject>

#include "v_busitem_private.h"
#include "v_busitem_adaptor.h"

class VBusItemPrivateProd : public VBusItemPrivate
{
	Q_OBJECT

public:
	VBusItemPrivateProd(VBusItem *parent);

	bool qProduce(QDBusConnection& cnx, QString& str, QString& description, QVariant& value, QString& unit, int precision);

	virtual QVariant qGetValue();
	virtual int qSetValue(QVariant const& value);
	virtual QString qGetText();
	virtual QVariant qGetMin() { return QVariant(); }
	virtual QVariant qGetMax() { return QVariant(); }
	virtual QVariant qGetDefault() { return QVariant(); }
	virtual int qSetDefault() { return -1; }

public slots:
	/* Dbus side */
	virtual QDBusVariant GetValue();
	virtual QString GetText();
	int SetValue(const QDBusVariant &value);
	QString GetDescription(const QString &language, int length);

	virtual void serviceOwnerChanged(QString,QString,QString) {}

signals:
	void PropertiesChanged(const QVariantMap &map);

private:
	VBusItemAdaptor* mAdp;

	QVariant mValue;
	QString mUnit;
	QString mDescription;
	int mPrecision;
};

#endif
