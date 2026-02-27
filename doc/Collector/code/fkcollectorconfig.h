// fkcollectorconfig.h
#ifndef FK_COLLECTOR_CONFIG_H
#define FK_COLLECTOR_CONFIG_H

#include "fkcollector/fkcollectordef.h"
#include "common/loggermacros.h"
#include <QString>
#include <QPointF>
#include <map>

class FkCollectorConfig
{
public:
    // 禁用拷贝和移动
    FkCollectorConfig(const FkCollectorConfig&) = delete;
    FkCollectorConfig(FkCollectorConfig&&) = delete;
    FkCollectorConfig& operator=(const FkCollectorConfig&) = delete;
    FkCollectorConfig& operator=(FkCollectorConfig&&) = delete;

    static FkCollectorConfig& getInstance();

    const FkCollectorConfigInfo& getConfig() const { return config_; }

    // 基本配置获取和设置
    uint8_t getCanType() const;
    void setCanType(uint8_t type);

    uint8_t getDeviceIndex() const { return deviceIndex_; }
    void setDeviceIndex(uint8_t deviceIndex) { deviceIndex_ = deviceIndex; };

    uint8_t getChannelIndex() const { return channelIndex_; }
    void setChannelIndex(uint8_t channelIndex) { channelIndex_ = channelIndex; };

    uint32_t getAssignID() const;

    uint32_t getDiscoveryTimeout() const { return discoveryTimeout_; }
    void setDiscoveryTimeout(uint32_t timeout);

    // 模块相关配置
    bool getModuleFile(FkCollectorModuleType type, QString& filePath, QString& fileEncoding) const;
    uint32_t getModuleMessageDefaultId(FkCollectorModuleType type) const;
    uint32_t getModuleMessageId(FkCollectorModuleType type);
    uint32_t getModuleFrequency(FkCollectorModuleType type) const;
    std::pair<QPointF, QPointF> getModulePoints(FkCollectorModuleType type) const;
    QString getModuleTypeName(FkCollectorModuleType type) const;

    // 模块配置设置接口
    void setModuleStartMessageId(FkCollectorModuleType type, uint32_t startId);
    void setModuleFrequency(FkCollectorModuleType type, uint32_t frequency);
    void setModulePoints(FkCollectorModuleType type, const QPointF& start, const QPointF& end);

    bool saveConfig() const;

private:
    // 默认值常量
    static constexpr uint32_t DEFAULT_ASSIGN_ID = 0x123;
    static constexpr uint32_t DEFAULT_START_MESSAGE_ID = 0x101;
    static constexpr uint32_t DEFAULT_FREQUENCY = 100;
    static constexpr double DEFAULT_START_X = 0.0;
    static constexpr double DEFAULT_START_Y = 0.0;
    static constexpr double DEFAULT_END_X = 100.0;
    static constexpr double DEFAULT_END_Y = 100.0;

    static const QString CAN_MODULE_DIR;
    static const QString CANFD_MODULE_DIR;
    static const QString DEFAULT_DBC_ENCODING;
    static const QString TEMPERATURE_DBC_FILE;
    static const QString VOLTAGE_DBC_FILE;
    static const QString CURRENT_DBC_FILE;
    static const QString FREQUENCY_DBC_FILE;

    const QString COLLECTOR_CONFIG_FILE = "config/collector/collector_config.ini";

    uint8_t deviceIndex_{0};
    uint8_t channelIndex_{FKVCI_DEVICE_CHANNEL_BEGIN};

    QString dbcFileEncoding_;
    FkCollectorConfigInfo config_;
    uint32_t discoveryTimeout_{3000}; // 设备发现的等待时间

    FkCollectorConfig();
    ~FkCollectorConfig() = default;

    void loadConfig();
};

#endif // FK_COLLECTOR_CONFIG_H
