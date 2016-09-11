#ifndef _VELIB_QT_V_BUSITEM_H_
#define _VELIB_QT_V_BUSITEM_H_

#include <QObject>
#include <QString>
#include <QVariant>
#include <QDBusVariant>
#include <QtDBus/QtDBus>

class VBusItemPrivate;
class VBusItemPrivateCons;
class VBusItemPrivateProd;

class VBusItem : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QVariant value READ getValue NOTIFY valueChanged WRITE setValue)
	Q_PROPERTY(QVariant text READ getText NOTIFY textChanged)
	Q_PROPERTY(QString bind READ getBind WRITE setBind NOTIFY bindChanged)
	Q_PROPERTY(QVariant min READ getMin NOTIFY minChanged)
	Q_PROPERTY(QVariant max READ getMax NOTIFY maxChanged)
	Q_PROPERTY(QVariant defaultValue READ getDefault NOTIFY defaultChanged)

public:
	explicit VBusItem(QObject *parent = 0);
	~VBusItem();

	VBusItem(QDBusConnection& cnx, QString str, QString description, QVariant value, QString unit = "", QObject *parent = 0);

	bool produce(QDBusConnection& cnx, QString str, QString description, QVariant value, QString unit = "", int precision = -1);
	bool consume(QDBusConnection& cnx, QString service, QString path);
	bool consume(QString service, QString path);

	QVariant getValue();
	Q_INVOKABLE int setValue(QVariant const& value);
	QString getText();
	QVariant getMin();
	QVariant getMax();
	QVariant getDefault();
	Q_INVOKABLE void setDefault();

	static QVariant invalid;

public:
	QString getBind(void) { return mBind; }
	void setBind(const QString &uri);

	static bool isValidBusName(QString busname);
signals:
	void valueChanged();
	void descriptionChanged();
	void bindChanged();
	void minChanged();
	void maxChanged();
	void defaultChanged();
	void textChanged();

private:
	QString mBind;
	VBusItemPrivate *d_ptr;
	Q_DECLARE_PRIVATE(VBusItem)
	friend class VBusItemPrivateCons;
	friend class VBusItemPrivateProd;
};

#endif
