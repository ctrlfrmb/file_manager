#ifndef FK_COLLECTOR_DEVICE_H
#define FK_COLLECTOR_DEVICE_H

#include <QVector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "fkcollector/fkcollectordef.h"
#include "fkvci/fkvcidevice.h"

class FkCollectorConfig;

class FkCollectorDevice : public QObject {
    Q_OBJECT

public:
    static constexpr uint32_t DEVICE_DEFAULT_SERIAL_NUMBER = 0xFFFFFFFF;
    static constexpr uint8_t SEARCH_DEVICE_RESPONSE_DATA_LENGTH = 3;

    static FkCollectorDevice &getInstance();

    bool hasDevice(std::map<uint32_t, FkCollectorDeviceInfo>& deviceInfos);
    std::map<uint32_t, FkCollectorDeviceInfo> getDevices();
    void showDeviceList();
    void clearDevices();

    int setConfigMode(uint32_t deviceId=DEVICE_DEFAULT_SERIAL_NUMBER);
    int setWorkMode(uint32_t deviceId=DEVICE_DEFAULT_SERIAL_NUMBER);
    int configDevice(uint32_t deviceId, const FkCollectorModuleInfo &moduleInfo);
    void sendTurnOn(uint32_t deviceId);

    void receiveCanMessage(uint8_t index, uint8_t channel, FkVciCanDataType &&can);

signals:
    void devicesCleared();

private:
    explicit FkCollectorDevice(QObject* parent = nullptr);
    ~FkCollectorDevice();

    FkCollectorDevice(const FkCollectorDevice&) = delete;
    FkCollectorDevice(FkCollectorDevice&&) = delete;
    FkCollectorDevice& operator=(const FkCollectorDevice&) = delete;
    FkCollectorDevice& operator=(FkCollectorDevice&&) = delete;

    FkVciDevice* getDeviceAndReceive();
    int sendAndCheckResponse(const FkVciCanDataType &request, int timeoutMS = 2000);
    void handleResponse(const FkVciCanDataType &can);
    void prepareRequest(FkVciCanDataType &request, uint32_t deviceId, uint8_t command, uint8_t subCommand, uint16_t value);
    int sendConfigCommand(uint32_t deviceId, uint8_t command, uint8_t subCommand, uint16_t value, bool isWait=true);
    int setDeviceConfigInfo(uint32_t deviceId, const FkCollectorModuleInfo &moduleInfo);

    FkCollectorConfig& m_config;

    std::map<uint32_t, FkCollectorDeviceInfo> m_deviceInfos;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_responseReceived;
    FkVciCanDataType m_response;
    std::atomic_bool m_isLog{false};
};

#endif // FK_COLLECTOR_DEVICE_H
