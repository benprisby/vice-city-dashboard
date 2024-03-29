#include "vcconfig.h"

#include <QDebug>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

#include "vchub.h"
/*--------------------------------------------------------------------------------------------------------------------*/

namespace {
VCConfig* instance_ = nullptr;
}  // namespace
/*--------------------------------------------------------------------------------------------------------------------*/

VCConfig::VCConfig(QObject* parent) : QObject(parent) {
    // Find our save slot in the Meta-Object system.
    const QMetaObject* meta = metaObject();
    for (int i = 0; i < meta->methodCount(); i++) {
        QMetaMethod method = meta->method(i);
        if (method.name() == "save") {
            saveMethod_ = method;
            break;
        }
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/

VCConfig* VCConfig::instance() {
    if (!instance_) {
        instance_ = new VCConfig(VCHub::instance());  // BDP: The parent allows key resolution.
    }

    return instance_;
}
/*--------------------------------------------------------------------------------------------------------------------*/

bool VCConfig::load(const QString& path) {
    if (!QFile::exists(path) || !QFileInfo(path).isWritable()) {
        qWarning() << "Ignoring config file path because it does not refer to an existing, writable file: " << path;
        return false;
    }

    QFile configFile(path);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open config file: " << path;
        return false;
    }

    qDebug() << "Loading config file: " << path;
    QJsonDocument configDocument = QJsonDocument::fromJson(configFile.readAll());
    if (!configDocument.isObject()) {
        qWarning() << "Failed to parse config file structure: " << path;
        configFile.close();
        return false;
    }

    QJsonObject config = configDocument.object();
    keys_.clear();
    const QStringList keys = config.keys();
    for (const auto& key : keys) {
        KeyContext context = keyToContext(key);
        QObject* object = context.first;
        QString property = context.second;

        if (!object || property.isEmpty()) {
            qWarning() << "Ignoring config key because it does not resolve: " << key;
            continue;
        }

        if (!object->property(property.toStdString().c_str()).isValid()) {
            qWarning() << "Ignoring config key because the property does not exist: " << key;
            continue;
        }

        // Find the property's NOTIFY signal to drive saving back the file.
        QMetaMethod notifySignal;
        const QMetaObject* meta = object->metaObject();
        for (int i = 0; i < meta->propertyCount(); i++) {
            QMetaProperty metaProperty = meta->property(i);
            if (property == metaProperty.name()) {
                if (metaProperty.hasNotifySignal()) {
                    notifySignal = metaProperty.notifySignal();

                    // Break any existing connection to prevent an unintended save while loading.
                    disconnect(object, notifySignal, this, saveMethod_);
                    break;
                } else {
                    qWarning() << "No NOTIFY signal for property in config key: " << key;
                }
            }
        }

        // Set the property to the value specified in the file.
        object->setProperty(property.toStdString().c_str(), config.value(key).toVariant());

        // Save the key name to track it now that is has been validated.
        keys_.append(key);

        // Connect or reconnect the notify signal.
        if (notifySignal.isValid()) {
            connect(object, notifySignal, this, saveMethod_);
        }
    }

    path_ = path;
    configFile.close();
    return true;
}
/*--------------------------------------------------------------------------------------------------------------------*/

bool VCConfig::save() {
    if (path_.isEmpty() || keys_.isEmpty()) {
        qDebug() << "Not saving config because one was not previously loaded";
        return false;
    }

    bool success = false;
    QJsonObject config;
    for (const auto& key : qAsConst(keys_)) {
        KeyContext context = keyToContext(key);
        QObject* object = context.first;
        QString property = context.second;

        // This should always resolve since this is a validated key, but do a sanity check anyway.
        if (!object || property.isEmpty()) {
            qWarning() << "Not saving config key because it does not resolve: " << key;
            continue;
        }

        // Record the latest property value.
        config[key] = QJsonValue::fromVariant(object->property(property.toStdString().c_str()));
    }

    if (!config.isEmpty()) {
        QFile configFile(path_);
        if (configFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            configFile.write(QJsonDocument(config).toJson());
            configFile.close();
            qDebug() << "Saved config file: " << path_;
            success = true;
        } else {
            qWarning() << "Failed to open config file when trying to save: " << path_;
        }
    } else {
        qWarning() << "Failed to build config file structure when trying to save";
    }

    return success;
}
/*--------------------------------------------------------------------------------------------------------------------*/

KeyContext VCConfig::keyToContext(const QString& key) {
    QObject* object = nullptr;
    QString propertyName;

    // Keys are references into the Meta-Object system using object names, with the property to set at the end.
    QStringList parts = key.split('.');
    if (parts.size() == 2) {
        // Drop the property name.
        propertyName = parts.takeLast();

        // Treat the parent as the top-most item in the heirarchy.
        object = parent();

        // If the object name is not the parent's, recurse down into the children to find it.
        if (object->objectName() != parts.first()) {
            object = object->findChild<QObject*>(parts.first());
        }
    }

    return {object, propertyName};
}
/*--------------------------------------------------------------------------------------------------------------------*/
