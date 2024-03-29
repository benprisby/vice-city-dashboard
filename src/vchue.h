#ifndef VCHUE_H_
#define VCHUE_H_

#include "huedevice.h"
#include "vcplugin.h"

class VCHue : public VCPlugin {
    Q_OBJECT

    // clang-format off
    Q_PROPERTY(QList<HueDevice*> devices                           READ devices          NOTIFY devicesChanged)
    Q_PROPERTY(int onDevicesCount                                  READ onDevicesCount   NOTIFY onDevicesCountChanged)
    Q_PROPERTY(QString bridgeIPAddress                             READ bridgeIPAddress  NOTIFY bridgeIPAddressChanged)
    Q_PROPERTY(QString bridgeUsername      MEMBER bridgeUsername_                        NOTIFY bridgeUsernameChanged)
    Q_PROPERTY(QVariantMap mapModel        MEMBER mapModel_                              NOTIFY mapModelChanged)
    // clang-format on

 public:
    explicit VCHue(const QString& name, QObject* parent = nullptr);

    const QList<HueDevice*>& devices() const { return devices_; }
    int onDevicesCount() const;
    const QString& bridgeIPAddress() const { return bridgeIPAddress_; }
    const QString& bridgeUsername() const { return bridgeUsername_; }

    void commandDeviceState(int id, const QJsonObject& parameters);

 signals:
    void devicesChanged();
    void onDevicesCountChanged();
    void bridgeIPAddressChanged();
    void bridgeUsernameChanged();
    void mapModelChanged();

 public slots:
    void refresh() override;
    void refreshGroups();

 private slots:
    void handleZeroConfServiceFound(const QString& serviceType, const QString& ipAddress);
    void handleNetworkReply(int statusCode, QObject* sender, const QJsonDocument& body);
    void updateBaseURL();

 private:
    QList<HueDevice*> devices_;
    QHash<int, HueDevice*> deviceTable_;  // Key: ID, Value: device
    QString bridgeIPAddress_;
    QString bridgeUsername_;
    QVariantMap mapModel_;

    QUrl lightsURL_;
    QUrl groupsURL_;

    Q_DISABLE_COPY_MOVE(VCHue)
};

#endif  // VCHUE_H_
