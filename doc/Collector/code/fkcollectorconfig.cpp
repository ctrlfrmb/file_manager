// fkcollectorconfig.cpp
#include "fkcollector/fkcollectorconfig.h"
#include "common/commonutils.h"
#include <QDir>
#include <QSettings>

const QString FkCollectorConfig::CAN_MODULE_DIR = "config/collector/can/";
const QString FkCollectorConfig::CANFD_MODULE_DIR = "config/collector/can_fd/";

const QString FkCollectorConfig::DEFAULT_DBC_ENCODING = "ASCII";

const QString FkCollectorConfig::TEMPERATURE_DBC_FILE = "temperature.dbc";
const QString FkCollectorConfig::VOLTAGE_DBC_FILE = "voltage.dbc";
const QString FkCollectorConfig::CURRENT_DBC_FILE = "current.dbc";
const QString FkCollectorConfig::FREQUENCY_DBC_FILE = "frequency.dbc";

FkCollectorConfig::FkCollectorConfig()
{
    // 初始化所有支持的模块
    config_.modules[FKCOLLECTOR_TEMPERATURE] = FkCollectorConfigModuleInfo();
    config_.modules[FKCOLLECTOR_VOLTAGE] = FkCollectorConfigModuleInfo();
    config_.modules[FKCOLLECTOR_CURRENT] = FkCollectorConfigModuleInfo();
    config_.modules[FKCOLLECTOR_FREQUENCY] = FkCollectorConfigModuleInfo();

    loadConfig();
}

FkCollectorConfig& FkCollectorConfig::getInstance() {
    static FkCollectorConfig instance;
    return instance;
}

QString FkCollectorConfig::getModuleTypeName(FkCollectorModuleType type) const {
    switch (type) {
        case FKCOLLECTOR_TEMPERATURE: return QObject::tr("Temperature");
        case FKCOLLECTOR_VOLTAGE: return QObject::tr("Voltage");
        case FKCOLLECTOR_CURRENT: return QObject::tr("Current");
        case FKCOLLECTOR_FREQUENCY: return QObject::tr("Frequency");
        default: return QObject::tr("Unknown");
    }
}

void FkCollectorConfig::loadConfig() {
    QSettings settings(COLLECTOR_CONFIG_FILE, QSettings::IniFormat);

    // 加载基本设置
    settings.beginGroup("Settings");
    dbcFileEncoding_ = settings.value("dbcFileEncoding", DEFAULT_DBC_ENCODING).toUInt();
    config_.assignId = settings.value("assignId", DEFAULT_ASSIGN_ID).toUInt();
    config_.canType = settings.value("canType", FKCOLLECTOR_CAN).toUInt();
    discoveryTimeout_ = settings.value("discoveryTimeout", 3000).toUInt();
    settings.endGroup();

    // 加载模块配置
    for (auto& [type, moduleInfo] : config_.modules) {
        QString groupName = getModuleTypeName(static_cast<FkCollectorModuleType>(type));
        settings.beginGroup(groupName);

        moduleInfo.startMessageId = settings.value("startMessageId",
            DEFAULT_START_MESSAGE_ID + (static_cast<uint32_t>(type) - 1) * 0x200).toUInt();
        moduleInfo.frequency = settings.value("frequency", DEFAULT_FREQUENCY).toUInt();

        moduleInfo.startPoint = QPointF(
            settings.value("startX", DEFAULT_START_X).toDouble(),
            settings.value("startY", DEFAULT_START_Y).toDouble()
        );
        moduleInfo.endPoint = QPointF(
            settings.value("endX", DEFAULT_END_X).toDouble(),
            settings.value("endY", DEFAULT_END_Y).toDouble()
        );

        settings.endGroup();
    }
}

bool FkCollectorConfig::saveConfig() const {
    try {
        // 确保配置文件目录存在
        if (!CommonUtils::ensureFilePath(COLLECTOR_CONFIG_FILE.toStdString())) {
            LOG_WARN("Failed to ensure config file path: {}", COLLECTOR_CONFIG_FILE.toStdString());
        }

        QSettings settings(COLLECTOR_CONFIG_FILE, QSettings::IniFormat);

        // 保存基本设置
        settings.beginGroup("Settings");
        settings.setValue("assignId", config_.assignId);
        settings.setValue("canType", config_.canType);
        settings.setValue("discoveryTimeout", discoveryTimeout_);
        settings.endGroup();

        // 保存模块配置
        for (const auto& [type, moduleInfo] : config_.modules) {
            QString groupName = getModuleTypeName(static_cast<FkCollectorModuleType>(type));
            settings.beginGroup(groupName);

            settings.setValue("startMessageId", moduleInfo.startMessageId);
            settings.setValue("frequency", moduleInfo.frequency);

            settings.setValue("startX", moduleInfo.startPoint.x());
            settings.setValue("startY", moduleInfo.startPoint.y());
            settings.setValue("endX", moduleInfo.endPoint.x());
            settings.setValue("endY", moduleInfo.endPoint.y());

            settings.endGroup();
        }

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Error saving config: {}", e.what());
        return false;
    }
}

uint8_t FkCollectorConfig::getCanType() const {
    return config_.canType;
}

void FkCollectorConfig::setCanType(uint8_t type) {
    config_.canType = type;
    saveConfig();
}

uint32_t FkCollectorConfig::getAssignID() const {
    return config_.assignId;
}

void FkCollectorConfig::setDiscoveryTimeout(uint32_t timeout) {
    discoveryTimeout_ = timeout;
    saveConfig();
}

bool FkCollectorConfig::getModuleFile(FkCollectorModuleType type, QString& filePath, QString& fileEncoding) const {
    QString fileDir = CAN_MODULE_DIR;
    if (getCanType() == FKCOLLECTOR_CANFD) {
        fileDir = CANFD_MODULE_DIR;
    }

    fileEncoding = dbcFileEncoding_;
    switch (type) {
    case FKCOLLECTOR_TEMPERATURE: {
        filePath = fileDir+TEMPERATURE_DBC_FILE;
        return true;
    }
    case FKCOLLECTOR_VOLTAGE: {
        filePath = fileDir+VOLTAGE_DBC_FILE;
        return true;
    }
    case FKCOLLECTOR_CURRENT: {
        filePath = fileDir+CURRENT_DBC_FILE;
        return true;
    }
    case FKCOLLECTOR_FREQUENCY: {
        filePath = fileDir+FREQUENCY_DBC_FILE;
        return true;
    }
    default:
        break;
    }
    return false;
}

uint32_t FkCollectorConfig::getModuleMessageDefaultId(FkCollectorModuleType type) const {
    auto it = config_.modules.find(type);
    if (it != config_.modules.end()) {
        return it->second.startMessageId;
    }
    return config_.assignId + (static_cast<uint32_t>(type) - 1) * 0x200;
}

uint32_t FkCollectorConfig::getModuleMessageId(FkCollectorModuleType type) {
    auto it = config_.modules.find(type);
    if (it != config_.modules.end()) {
        return it->second.startMessageId++;
    }
    return config_.assignId++;
}

uint32_t FkCollectorConfig::getModuleFrequency(FkCollectorModuleType type) const {
    auto it = config_.modules.find(type);
    return it != config_.modules.end() ? it->second.frequency : DEFAULT_FREQUENCY;
}

std::pair<QPointF, QPointF> FkCollectorConfig::getModulePoints(FkCollectorModuleType type) const {
    auto it = config_.modules.find(type);
    if (it != config_.modules.end()) {
        return {it->second.startPoint, it->second.endPoint};
    }
    return {QPointF(DEFAULT_START_X, DEFAULT_START_Y),
            QPointF(DEFAULT_END_X, DEFAULT_END_Y)};
}

void FkCollectorConfig::setModuleStartMessageId(FkCollectorModuleType type, uint32_t startId) {
    if (auto it = config_.modules.find(type); it != config_.modules.end()) {
        it->second.startMessageId = startId;
        saveConfig();
    }
}

void FkCollectorConfig::setModuleFrequency(FkCollectorModuleType type, uint32_t frequency) {
    if (auto it = config_.modules.find(type); it != config_.modules.end()) {
        it->second.frequency = frequency;
        saveConfig();
    }
}

void FkCollectorConfig::setModulePoints(FkCollectorModuleType type,
                                      const QPointF& start, const QPointF& end) {
    if (auto it = config_.modules.find(type); it != config_.modules.end()) {
        it->second.startPoint = start;
        it->second.endPoint = end;
        saveConfig();
    }
}
