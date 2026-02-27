// fkcollectordef.h

#ifndef FK_COLLECTOR_DEF_H
#define FK_COLLECTOR_DEF_H

#include <stdint.h>
#include <QString>
#include <QPointF>
#include <unordered_set>

#include "can/dbcdef.h"
#include "common/commondef.h"

#define FKCOLLECTOR_MODULE_TYPE_COUNT                           4
#define FKCOLLECTOR_MODULE_CHANNEL_SIZE                         0x0F

#define FKCOLLECTOR_CONFIG_TABLE_COLUMN_CANID                   0
#define FKCOLLECTOR_CONFIG_TABLE_COLUMN_PERIOD                  1
#define FKCOLLECTOR_CONFIG_TABLE_COLUMN_COUNT                   2

#define FKCOLLECTOR_SIGNAL_TABLE_COLUMN_UNIT                    0
#define FKCOLLECTOR_SIGNAL_TABLE_COLUMN_MIN                     1
#define FKCOLLECTOR_SIGNAL_TABLE_COLUMN_MAX                     2
#define FKCOLLECTOR_SIGNAL_TABLE_COLUMN_FACTOR                  3
#define FKCOLLECTOR_SIGNAL_TABLE_COLUMN_OFFSET                  4
#define FKCOLLECTOR_SIGNAL_TABLE_COLUMN_PLOT_BUTTON             5
#define FKCOLLECTOR_SIGNAL_TABLE_COLUMN_CAHNNEL                 6
#define FKCOLLECTOR_SIGNAL_TABLE_COLUMN_MEASURE_RANGE           7
#define FKCOLLECTOR_SIGNAL_TABLE_COLUMN_OUT_VOLTAGE             8
#define FKCOLLECTOR_SIGNAL_TABLE_COLUMN_COUNT                   9

#define FKCOLLECTOR_DEVICE_RESPONSE_WAIT_TIME                   2000 // ms

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

enum FkCollectorCanType {
    FKCOLLECTOR_CAN=0x0,
    FKCOLLECTOR_CANFD=0x01
};

enum FkCollectorModuleType {
    FKCOLLECTOR_TEMPERATURE=0x01,
    FKCOLLECTOR_VOLTAGE,
    FKCOLLECTOR_CURRENT,
    FKCOLLECTOR_FREQUENCY,
};

struct FkCollectorConfigModuleInfo {
    uint32_t startMessageId;
    uint16_t frequency;
    QPointF startPoint;
    QPointF endPoint;
};

struct FkCollectorConfigInfo {
    uint32_t assignId;
    uint8_t canType{FKCOLLECTOR_CAN};
    std::map<uint8_t, FkCollectorConfigModuleInfo> modules;
};

struct FkCollectorModuleInfo {
    QString name;                                                   // Module name, used as the prefix of the message name
                                                                    // (when there is only one message, the message name == module name)
    FkCollectorModuleType type;                                     // Module type
    uint16_t frequency{100};                                        // Module transmission frequency 1-1000
    std::vector<OpenSource::DBCMessage> messages;                   // CAN messages included in the module
};

struct FkCollectorDeviceInfo {
    FkCollectorModuleType type;
    uint8_t channels;
    int moduleId;
};

#endif // FK_COLLECTOR_DEF_H
