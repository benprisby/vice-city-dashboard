#include "vcpihole.h"

#include <QJsonObject>

#include "networkinterface.h"
/*--------------------------------------------------------------------------------------------------------------------*/

VCPiHole::VCPiHole(const QString& name, QObject* parent)
    : VCPlugin(name, parent),
      serverPort_(80),
      isEnabled_(false),
      totalQueries_(0),
      blockedQueries_(0),
      percentBlocked_(qQNaN()),
      blockedDomains_(0) {
    // Don't start refreshing until the Pi-hole server has been found.
    updateTimer_.stop();
    setUpdateInterval(1000);

    // Handle network responses.
    connect(NetworkInterface::instance(), &NetworkInterface::jsonReplyReceived, this, &VCPiHole::handleNetworkReply);

    // Look for the Pi-hole server when the hostname is populated.
    connect(this, &VCPiHole::serverHostnameChanged, this, [this] {
        if (!serverHostname_.isEmpty()) {
            (void)QHostInfo::lookupHost(serverHostname_, this, &VCPiHole::handleHostLookup);
        }
    });
}
/*--------------------------------------------------------------------------------------------------------------------*/

void VCPiHole::refresh() {
    NetworkInterface::instance()->sendJSONRequest(summaryDestination_, this);
}
/*--------------------------------------------------------------------------------------------------------------------*/

void VCPiHole::refreshHistoricalData() {
    NetworkInterface::instance()->sendJSONRequest(historicalDataDestination_, this);
}
/*--------------------------------------------------------------------------------------------------------------------*/

void VCPiHole::handleHostLookup(const QHostInfo& host) {
    if (host.error() != QHostInfo::NoError) {
        qDebug() << "Failed to find Pi-hole server on the local network with error: " << host.errorString();
    }

    const QList<QHostAddress> addresses = host.addresses();
    for (const auto& address : addresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            serverIPAddress_ = address.toString();
            qDebug() << "Pi-hole server found at IP address: " << serverIPAddress_;

            // Update the destination URL and start refreshing information.
            QString baseURL = QString("http://%1:%2/admin/api.php").arg(serverIPAddress_).arg(serverPort_);
            summaryDestination_ = QUrl(QString("%1?%2").arg(baseURL, "summaryRaw"));
            historicalDataDestination_ = QUrl(QString("%1?%2").arg(baseURL, "overTimeData10mins"));
            updateTimer_.start();
            refresh();
            refreshHistoricalData();
            break;
        } else {
            // IPv6 is the protocol of the future and always will be.
        }
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/

void VCPiHole::handleNetworkReply(int statusCode, QObject* sender, const QJsonDocument& body) {
    if (sender != this) {
        // Not for us, ignore.
        return;
    }
    if (statusCode != 200) {
        qDebug() << "Ignoring unsuccessful reply from Pi-hole server with status code: " << statusCode;
        return;
    }
    if (!body.isObject()) {
        qDebug() << "Failed to parse response from Pi-hole server";
        return;
    }

    QJsonObject responseObject = body.object();
    if (responseObject.contains("status")) {
        bool enabled = (responseObject.value("status").toString() == "enabled");
        if (isEnabled_ != enabled) {
            isEnabled_ = enabled;
            emit isEnabledChanged();
        }
    }
    if (responseObject.contains("dns_queries_today")) {
        int totalQueries = responseObject.value("dns_queries_today").toInt();
        if (totalQueries_ != totalQueries) {
            totalQueries_ = totalQueries;
            emit totalQueriesChanged();
        }
    }
    if (responseObject.contains("ads_blocked_today")) {
        int blockedQueries = responseObject.value("ads_blocked_today").toInt();
        if (blockedQueries_ != blockedQueries) {
            blockedQueries_ = blockedQueries;
            emit blockedQueriesChanged();
        }
    }
    if (responseObject.contains("ads_percentage_today")) {
        // BDP: We could easily calculate this, but since it's included in the response just use it.
        double percentBlocked = responseObject.value("ads_percentage_today").toDouble();
        if (percentBlocked_ != percentBlocked) {
            percentBlocked_ = percentBlocked;
            emit percentBlockedChanged();
        }
    }
    if (responseObject.contains("domains_being_blocked")) {
        int blockedDomains = responseObject.value("domains_being_blocked").toInt();
        if (blockedDomains_ != blockedDomains) {
            blockedDomains_ = blockedDomains;
            emit blockedDomainsChanged();
        }
    }
    if (responseObject.contains("domains_over_time") && responseObject.contains("ads_over_time")) {
        QJsonObject domainsOverTime = responseObject.value("domains_over_time").toObject();
        QJsonObject adsOverTime = responseObject.value("ads_over_time").toObject();
        if (!domainsOverTime.isEmpty() && (domainsOverTime.size() == adsOverTime.size())) {
            QVariantList timestamps;
            QVariantList totalQueries;
            QVariantList blockedQueries;
            QVariantList allowedQueries;
            QVariantList blockPercentages;
            int maxTotalQueries = 0;
            double minBlockPercentage = 100.0;
            double maxBlockPercentage = 0.0;

            const QStringList keys = domainsOverTime.keys();
            for (const auto& key : keys) {
                timestamps.append(key.toInt());
            }
            std::sort(timestamps.begin(), timestamps.end());  // Arrange chronologically
            for (const auto& timestamp : timestamps) {
                int total = domainsOverTime.value(timestamp.toString()).toInt();
                int blocked = adsOverTime.value(timestamp.toString()).toInt();
                int allowed = total - blocked;
                double blockPercentage = (static_cast<double>(blocked) / total) * 100.0;

                totalQueries.append(total);
                blockedQueries.append(blocked);
                allowedQueries.append(allowed);
                blockPercentages.append(blockPercentage);

                // Keep track of minimums and maximums in the sets.
                if (maxTotalQueries < total) {
                    maxTotalQueries = total;
                }
                if (minBlockPercentage > blockPercentage) {
                    minBlockPercentage = blockPercentage;
                }
                if (maxBlockPercentage < blockPercentage) {
                    maxBlockPercentage = blockPercentage;
                }
            }

            // Expose as a model for graphing in QML, with lists of data points and some statistics.
            historicalData_ = QVariantMap{{"timestamps", timestamps},
                                          {"totalQueries", totalQueries},
                                          {"maxTotalQueries", maxTotalQueries},
                                          {"blockedQueries", blockedQueries},
                                          {"allowedQueries", allowedQueries},
                                          {"blockPercentages", blockPercentages},
                                          {"minBlockPercentage", minBlockPercentage},
                                          {"maxBlockPercentage", maxBlockPercentage}};
            emit historicalDataChanged();
        }
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
