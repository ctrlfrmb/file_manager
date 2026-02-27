// fkcollectorutils.h
#ifndef FK_COLLECTOR_UTILS_H
#define FK_COLLECTOR_UTILS_H

#include <QTreeWidgetItem>
#include <map>
#include "fkcollector/fkcollectordef.h"

class FkCollectorUtils {
public:
    // 禁用构造函数，因为这是一个纯工具类
    FkCollectorUtils() = delete;

    // 模块配置相关常量
    static constexpr uint16_t MODULE_ID_MIN = 100;
    static constexpr uint16_t MODULE_ID_MAX = 65535;
    static constexpr uint16_t FREQUENCY_MIN = 1;
    static constexpr uint16_t FREQUENCY_MAX = 1000;
    static constexpr uint8_t CHANNEL_MIN = 0x00;
    static constexpr uint8_t CHANNEL_MAX = FKCOLLECTOR_MODULE_CHANNEL_SIZE;
    static constexpr uint8_t MEASURE_RANGE_BEGIN_INDEX = 0x0;
    static constexpr uint8_t MEASURE_RANGE_END_INDEX = 0x07;
    static constexpr uint8_t OUT_VOLTAGE_BEGIN_INDEX = 0x0;
    static constexpr uint8_t OUT_VOLTAGE_END_INDEX = 0x02;

    // 验证模块配置
    static bool validateModuleInfos(const std::map<int, FkCollectorModuleInfo>& moduleInfos);

    // 验证选中的模块类型
    static bool validateModuleType(
        QTreeWidgetItem* currentItem,
        const std::map<int, FkCollectorModuleInfo>& moduleInfos,
        FkCollectorModuleType& outType
    );

    // 验证指定类型的设备
    static bool validateDevicesForType(
        const std::map<uint32_t, FkCollectorDeviceInfo>& devices,
        FkCollectorModuleType type
    );

private:
    // 验证单个模块配置
    static bool validateModuleConfig(const FkCollectorModuleInfo& moduleInfo);

    // 错误消息模板
    static const QString MSG_NO_MODULES;
    static const QString MSG_NO_SELECTION;
    static const QString MSG_INVALID_MODULE;
    static const QString MSG_NO_VALID_MODULES;
    static const QString MSG_NO_MATCHING_DEVICES;
    static const QString MSG_INVALID_MODULE_ID;
    static const QString MSG_INVALID_FREQUENCY;
    static const QString MSG_INVALID_CHANNEL;
    static const QString MSG_INVALID_MEASURE_RANGE;
    static const QString MSG_INVALID_OUT_VOLTAGE;
};

#endif // FK_COLLECTOR_UTILS_H
