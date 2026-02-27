// fkcollectorutils.cpp
#include "fkcollector/fkcollectorutils.h"
#include "fkcollector/fkcollectorhelper.h"
#include <QMessageBox>

// 定义错误消息模板
const QString FkCollectorUtils::MSG_NO_MODULES = QObject::tr("No modules available.");
const QString FkCollectorUtils::MSG_NO_SELECTION = QObject::tr("Please select a module type or module.");
const QString FkCollectorUtils::MSG_INVALID_MODULE = QObject::tr("Invalid module selected.");
const QString FkCollectorUtils::MSG_NO_VALID_MODULES = QObject::tr("No valid modules available for binding.");
const QString FkCollectorUtils::MSG_NO_MATCHING_DEVICES = QObject::tr("No devices found for the selected module type.");
const QString FkCollectorUtils::MSG_INVALID_MODULE_ID = QObject::tr("Module '%1': Config module DBC output ID(%2) is invalid, range %3-%4.");
const QString FkCollectorUtils::MSG_INVALID_FREQUENCY = QObject::tr("Module '%1': Config module transmission frequency is invalid, range %2-%3.");
const QString FkCollectorUtils::MSG_INVALID_CHANNEL = QObject::tr("Module '%1': Config module channel is invalid, range 0x%2~0x%3.");
const QString FkCollectorUtils::MSG_INVALID_MEASURE_RANGE = QObject::tr("Module '%1': Config module sampling measureRange is invalid, range 0x%2-0x%3.");
const QString FkCollectorUtils::MSG_INVALID_OUT_VOLTAGE = QObject::tr("Module '%1': Config module output voltage is invalid, 0:NC  1:5V  2:12V.");

bool FkCollectorUtils::validateModuleInfos(
    const std::map<int, FkCollectorModuleInfo>& moduleInfos)
{
    if (moduleInfos.empty()) {
        QMessageBox::warning(nullptr, QObject::tr("Warning"), MSG_NO_MODULES);
        return false;
    }

    for (const auto& [moduleId, moduleInfo] : moduleInfos) {
        if (!validateModuleConfig(moduleInfo)) {
            return false;
        }
    }

    return true;
}

bool FkCollectorUtils::validateModuleType(
    QTreeWidgetItem* currentItem,
    const std::map<int, FkCollectorModuleInfo>& moduleInfos,
    FkCollectorModuleType& outType)
{
    if (!currentItem) {
        QMessageBox::warning(nullptr, QObject::tr("Warning"), MSG_NO_SELECTION);
        return false;
    }

    if (FkCollectorHelper::isModuleItem(currentItem)) {
        int moduleId = currentItem->data(0, Qt::UserRole).toInt();
        auto it = moduleInfos.find(moduleId);
        if (it == moduleInfos.end()) {
            QMessageBox::warning(nullptr, QObject::tr("Warning"), MSG_INVALID_MODULE);
            return false;
        }
        outType = it->second.type;
    } else {
        outType = static_cast<FkCollectorModuleType>(
            currentItem->data(0, Qt::UserRole).toInt());
    }

    bool hasValidModule = false;
    for (const auto& [moduleId, moduleInfo] : moduleInfos) {
        if (moduleInfo.type == outType) {
            hasValidModule = true;
            if (!validateModuleConfig(moduleInfo)) {
                return false;
            }
        }
    }

    if (!hasValidModule) {
        QMessageBox::warning(nullptr, QObject::tr("Warning"), MSG_NO_VALID_MODULES);
        return false;
    }

    return true;
}

bool FkCollectorUtils::validateDevicesForType(
    const std::map<uint32_t, FkCollectorDeviceInfo>& devices,
    FkCollectorModuleType type)
{
    bool hasMatchingDevice = false;
    for (const auto& [deviceId, deviceInfo] : devices) {
        if (deviceInfo.type == type) {
            hasMatchingDevice = true;
            break;
        }
    }

    if (!hasMatchingDevice) {
        QMessageBox::warning(nullptr, QObject::tr("Warning"), MSG_NO_MATCHING_DEVICES);
        return false;
    }
    return true;
}

bool FkCollectorUtils::validateModuleConfig(const FkCollectorModuleInfo& moduleInfo)
{
    // 校验 messages[0].id 范围
    if (moduleInfo.messages.empty() ||
        moduleInfo.messages.front().id < MODULE_ID_MIN ||
        moduleInfo.messages.front().id > MODULE_ID_MAX) {
        QMessageBox::warning(nullptr, QObject::tr("Warning"),
            MSG_INVALID_MODULE_ID.arg(moduleInfo.name)
                               .arg(moduleInfo.messages.front().id)
                               .arg(MODULE_ID_MIN)
                               .arg(MODULE_ID_MAX));
        return false;
    }

    // 校验 frequency 范围
    if (moduleInfo.frequency < FREQUENCY_MIN ||
        moduleInfo.frequency > FREQUENCY_MAX) {
        QMessageBox::warning(nullptr, QObject::tr("Warning"),
            MSG_INVALID_FREQUENCY.arg(moduleInfo.name)
                               .arg(FREQUENCY_MIN)
                               .arg(FREQUENCY_MAX));
        return false;
    }

    // 对电压和频率类型的模块进行额外验证
    if (moduleInfo.type == FKCOLLECTOR_VOLTAGE ||
        moduleInfo.type == FKCOLLECTOR_FREQUENCY) {
        for (const auto& message : moduleInfo.messages) {
            for (const auto& signal : message.signalList) {
                // 校验 channel 范围
                if (signal.channel < CHANNEL_MIN ||
                    signal.channel >= CHANNEL_MAX) {
                    QMessageBox::warning(nullptr, QObject::tr("Warning"),
                        MSG_INVALID_CHANNEL.arg(moduleInfo.name)
                                        .arg(CHANNEL_MIN, 2, 16, QChar('0'))
                                        .arg(CHANNEL_MAX - 1, 2, 16, QChar('0')));
                    return false;
                }

                // 校验 measureRange 范围
                if (signal.measureRange < MEASURE_RANGE_BEGIN_INDEX ||
                    signal.measureRange > MEASURE_RANGE_END_INDEX) {
                    QMessageBox::warning(nullptr, QObject::tr("Warning"),
                        MSG_INVALID_MEASURE_RANGE.arg(moduleInfo.name)
                                               .arg(MEASURE_RANGE_BEGIN_INDEX, 2, 16, QChar('0'))
                                               .arg(MEASURE_RANGE_END_INDEX, 2, 16, QChar('0')));
                    return false;
                }

                // 校验 outVoltage 范围
                if (signal.outVoltage < OUT_VOLTAGE_BEGIN_INDEX ||
                    signal.outVoltage > OUT_VOLTAGE_END_INDEX) {
                    QMessageBox::warning(nullptr, QObject::tr("Warning"),
                        MSG_INVALID_OUT_VOLTAGE.arg(moduleInfo.name));
                    return false;
                }
            }
        }
    }

    return true;
}
