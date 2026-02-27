#include "fkcollector/fkcollectordevice.h"
#include "fkcollector/fkcollectorconfig.h"
#include "fkcollector/fkcollectorhelper.h"
#include "fkvci/fkvcimanager.h"
#include "fkvci/fkvcistatusmanager.h"
#include "fkvci/fkvcidatamanager.h"
#include "common/loggermacros.h"
#include <chrono>
#include <thread>
#include <QMessageBox>

//#define SELF_TEST

#define DEVICE_REQUEST_ID                       0x01
#define DEVICE_DISCOVER_REQUEST_CMD             0x3E
#define DEVICE_DISCOVER_RESPONSE_CMD            0x7E
#define DEVICE_CONFIG_REQUEST_CMD               0x2E
#define DEVICE_CONFIG_RESPONSE_CMD              0x6E
#define DEVICE_CONFIG_MODE_CMD                  0x10
#define DEVICE_CONFIG_MODE_RESPONSE_CMD         0x50
#define DEVICE_CONFIG_MODE_ENTER                0x03
#define DEVICE_CONFIG_MODE_COMPLETE             0x01

#define DEVICE_RESPONSE_COMMAND_INDEX           0
#define DEVICE_RESPONSE_SUB_COMMAND_INDEX       1
#define DEVICE_REQUEST_SUB_COMMAND_INDEX        5

#define DEVICE_CONFIG_TXID_CMD                  0x01
#define DEVICE_CONFIG_FRZ_CMD                   0x02
#define DEVICE_CONFIG_MEASRNG_CMD               0x03
#define DEVICE_CONFIG_OUTVOL_CMD                0x04
#define DEVICE_CONFIG_CANTYPE_CMD               0x05
#define DEVICE_CONFIG_COMPLETED_CMD             0xFF

#define DEVICE_RESPONSE_TIMEOUT                 (-120000)
#define DEVICE_CONFIG_INPUT_INVALID             (-120001)
#define DEVICE_CONFIG_TXID_FAILED               (-120011)
#define DEVICE_CONFIG_FRZ_FAILED                (-120012)
#define DEVICE_CONFIG_RANG_FAILED               (-120013)
#define DEVICE_CONFIG_OUTVOL_FAILED             (-120014)
#define DEVICE_CONFIG_CANTYPE_FAILED            (-120015)
#define DEVICE_CONFIG_COMPLETED_FAILED          (-120016)
#define DEVICE_CONFIG_FINISHED_FAILED           (-120021)
#define DEVICE_CONFIG_ENTER_FAILED              (-120023)

// 首先定义错误提示信息映射
const std::map<int, QString> ERROR_MESSAGES = {
    {DEVICE_RESPONSE_TIMEOUT, QObject::tr("Device not responding. Please check the connection.")},
    {DEVICE_CONFIG_INPUT_INVALID, QObject::tr("Invalid configuration parameters.")},
    {DEVICE_CONFIG_TXID_FAILED, QObject::tr("Failed to configure message ID.")},
    {DEVICE_CONFIG_FRZ_FAILED, QObject::tr("Failed to configure sampling frequency.")},
    {DEVICE_CONFIG_RANG_FAILED, QObject::tr("Failed to configure measurement range.")},
    {DEVICE_CONFIG_OUTVOL_FAILED, QObject::tr("Failed to configure output voltage.")},
    {DEVICE_CONFIG_CANTYPE_FAILED, QObject::tr("Failed to configure CAN type.")},
    {DEVICE_CONFIG_COMPLETED_FAILED, QObject::tr("Failed to complete device configuration.")},
    {DEVICE_CONFIG_FINISHED_FAILED, QObject::tr("Failed to switch to work mode.")},
    {DEVICE_CONFIG_ENTER_FAILED, QObject::tr("Failed to enter configuration mode.")}
};

// 添加显示错误的辅助函数
void showError(int errorCode) {
    auto it = ERROR_MESSAGES.find(errorCode);
    QString message = it != ERROR_MESSAGES.end() ?
                     it->second :
                     QObject::tr("Unknown error occurred.");

    QMessageBox::critical(nullptr, QObject::tr("Error"), message);
}

FkCollectorDevice &FkCollectorDevice::getInstance() {
    static FkCollectorDevice obj;
    return obj;
}

FkCollectorDevice::FkCollectorDevice(QObject* parent) : QObject(parent)
  , m_config(FkCollectorConfig::getInstance())
  , m_responseReceived(false) {}

FkCollectorDevice::~FkCollectorDevice() {}

FkVciDevice* FkCollectorDevice::getDeviceAndReceive() {
    auto* handle = FkVciManager::getInstance().getDeviceHandle(m_config.getDeviceIndex());
    if (!handle) {
        LOG_ERROR("Failed to obtain Device{} handle ", m_config.getDeviceIndex());
        return nullptr;
    }

    handle->setCanReceiveCallback(std::bind(&FkCollectorDevice::receiveCanMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    return handle;
}

int FkCollectorDevice::sendAndCheckResponse(const FkVciCanDataType &request, int timeoutMS) {
#ifdef SELF_TEST
    return 0;
#endif
    auto handle = getDeviceAndReceive();
    if (!handle) return -1;

    if (0 != handle->send(FkVciDataManager::constructCanSendData(m_config.getChannelIndex(), request))) {
        LOG_ERROR("Request failed, id 0x{:X}: {}", request.CanID, QByteArray(reinterpret_cast<const char *>(request.Data), static_cast<int>(request.DLC)).toHex(' ').toStdString().c_str());
    }
    else {
        LOG_INFO("Request id 0x{:X}: {}", request.CanID,  QByteArray(reinterpret_cast<const char *>(request.Data), static_cast<int>(request.DLC)).toHex(' ').toStdString().c_str());
    }
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_cv.wait_for(lock, std::chrono::milliseconds(timeoutMS),
                      [this] { return m_responseReceived; })) {
        m_responseReceived = false;
        return 0;
    }

    return DEVICE_RESPONSE_TIMEOUT;
}

void FkCollectorDevice::receiveCanMessage(uint8_t /*index*/, uint8_t channel, FkVciCanDataType &&can) {
    if (channel != m_config.getChannelIndex() || can.REV2 != FKCAN_FLAG_RX) return;

    handleResponse(can);
    if (m_isLog)
        LOG_INFO("id:0x{:X}, dlc:{}, data: {}", can.CanID, can.DLC, QByteArray(reinterpret_cast<const char *>(can.Data), can.DLC).toHex(' ').toStdString().c_str());
    m_cv.notify_one();
}

void FkCollectorDevice::handleResponse(const FkVciCanDataType &can) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_response.Data[DEVICE_RESPONSE_COMMAND_INDEX] != can.Data[DEVICE_RESPONSE_COMMAND_INDEX]) return;

    switch (can.Data[DEVICE_RESPONSE_COMMAND_INDEX]) {
        case DEVICE_DISCOVER_RESPONSE_CMD:
            if (can.DLC == SEARCH_DEVICE_RESPONSE_DATA_LENGTH && can.Data[1] < FKCOLLECTOR_MODULE_CHANNEL_SIZE) {
                m_deviceInfos[can.CanID] = {static_cast<FkCollectorModuleType>(can.Data[1]), can.Data[2], INVALID_DEFAULT_VALUE};
            }
            break;
        case DEVICE_CONFIG_MODE_RESPONSE_CMD:
            if (can.CanID == m_response.CanID) {
                m_responseReceived = true;
            }
            break;
        case DEVICE_CONFIG_RESPONSE_CMD:
            if (can.CanID == m_response.CanID && m_response.Data[DEVICE_RESPONSE_SUB_COMMAND_INDEX] == can.Data[DEVICE_RESPONSE_SUB_COMMAND_INDEX]) {
                m_responseReceived = true;
            }
            break;
        default:
            // Handle unknown response
            break;
    }
}

void FkCollectorDevice::prepareRequest(FkVciCanDataType &request, uint32_t deviceId, uint8_t command, uint8_t subCommand, uint16_t value) {
    memset(&request, 0, sizeof (FkVciCanDataType));
    request.CanID = DEVICE_REQUEST_ID;
    request.FLAG |= FKCAN_FLAGS_EXTENDED_FRAME;
    request.Data[DEVICE_RESPONSE_COMMAND_INDEX] = command;
    switch (command) {
    case DEVICE_DISCOVER_REQUEST_CMD:
        request.DLC = 5;
        *(uint32_t*)&request.Data[1] = deviceId;
        break;
    case DEVICE_CONFIG_MODE_CMD:
        request.DLC = 6;
        *(uint32_t*)&request.Data[1] = deviceId;
        request.Data[DEVICE_REQUEST_SUB_COMMAND_INDEX] = subCommand;
        break;
    default:
        request.DLC = 8;
        *(uint32_t*)&request.Data[1] = deviceId;
        request.Data[DEVICE_REQUEST_SUB_COMMAND_INDEX] = subCommand;
        if (subCommand == DEVICE_CONFIG_TXID_CMD || subCommand == DEVICE_CONFIG_FRZ_CMD
                || subCommand == DEVICE_CONFIG_CANTYPE_CMD) {
            *(uint16_t*)&request.Data[6] = value;
        }
        else {
            request.Data[6] = static_cast<uint8_t>(value>>8);
            request.Data[7] = static_cast<uint8_t>(value&0xFF);

            LOG_INFO("SD: CH{} VAL:{}", request.Data[6], request.Data[7]);
        }
        break;
    }
}

int FkCollectorDevice::sendConfigCommand(uint32_t deviceId, uint8_t command, uint8_t subCommand, uint16_t value, bool isWait) {
    FkVciCanDataType request;
    prepareRequest(request, deviceId, command, subCommand, value);

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_response.CanID = deviceId;
        m_response.Data[DEVICE_RESPONSE_COMMAND_INDEX] = command+0x40;
        m_response.Data[DEVICE_RESPONSE_SUB_COMMAND_INDEX] = subCommand;
        m_responseReceived = !isWait;
    }

    int ret{0};
    if (0 != (ret = sendAndCheckResponse(request))) {
        LOG_ERROR("Module {} configuration failed for command {:X}x, sub command {:X}x", deviceId, command, subCommand);
    }

    return ret;
}

int FkCollectorDevice::setDeviceConfigInfo(uint32_t deviceId,  const FkCollectorModuleInfo &moduleInfo) {
    if (m_deviceInfos.end() == m_deviceInfos.find(deviceId) || moduleInfo.messages.empty()) return DEVICE_CONFIG_INPUT_INVALID;

    int result{0};
    const auto& device = m_deviceInfos[deviceId];

    // 执行配置命令
    if ((result = sendConfigCommand(deviceId, DEVICE_CONFIG_REQUEST_CMD, DEVICE_CONFIG_TXID_CMD, moduleInfo.messages.front().id)) != 0) return DEVICE_CONFIG_TXID_FAILED;
    if ((result = sendConfigCommand(deviceId, DEVICE_CONFIG_REQUEST_CMD, DEVICE_CONFIG_FRZ_CMD, moduleInfo.frequency)) != 0) return DEVICE_CONFIG_FRZ_FAILED;

    // 如果是AD模块，添加额外的配置
    if (device.type == FKCOLLECTOR_VOLTAGE || device.type == FKCOLLECTOR_FREQUENCY) {
        for (const auto &message : moduleInfo.messages) {
            for (const auto &signal : message.signalList) {
                if (0 != sendConfigCommand(deviceId, DEVICE_CONFIG_REQUEST_CMD, DEVICE_CONFIG_MEASRNG_CMD, (signal.channel << 8) | signal.measureRange)) {
                    if (0 == result) result = DEVICE_CONFIG_RANG_FAILED;
                }
                if (0 != sendConfigCommand(deviceId, DEVICE_CONFIG_REQUEST_CMD, DEVICE_CONFIG_OUTVOL_CMD, (signal.channel << 8) | signal.outVoltage)) {
                    if (0 == result) result = DEVICE_CONFIG_OUTVOL_FAILED;
                }
            }
        }
    }

    auto canType = m_config.getCanType();
    if (0 != sendConfigCommand(deviceId, DEVICE_CONFIG_REQUEST_CMD, DEVICE_CONFIG_CANTYPE_CMD, canType)) {
        if (0 == result) result = DEVICE_CONFIG_CANTYPE_FAILED;
    }

    if (0 != sendConfigCommand(deviceId, DEVICE_CONFIG_REQUEST_CMD, DEVICE_CONFIG_COMPLETED_CMD, 0)) {
        if (0 == result) result = DEVICE_CONFIG_COMPLETED_FAILED;
    }
    return result;
}

int FkCollectorDevice::setConfigMode(uint32_t deviceId) {
    bool isWait = (deviceId!=DEVICE_DEFAULT_SERIAL_NUMBER);
    if (0 != sendConfigCommand(deviceId, DEVICE_CONFIG_MODE_CMD, DEVICE_CONFIG_MODE_ENTER, 0, isWait)) {
        LOG_ERROR("Failed to enter configuration mode for device {}", deviceId);
#ifndef SELF_TEST
        return DEVICE_CONFIG_ENTER_FAILED;
#endif
    }
    return 0;
}

int FkCollectorDevice::setWorkMode(uint32_t deviceId) {
    bool isWait = (deviceId!=DEVICE_DEFAULT_SERIAL_NUMBER);
    if (0 != sendConfigCommand(deviceId, DEVICE_CONFIG_MODE_CMD, DEVICE_CONFIG_MODE_COMPLETE, 0, isWait)) {
        LOG_ERROR("Failed to complete configuration mode for device {}", deviceId);
        return DEVICE_CONFIG_FINISHED_FAILED;
    }

    return 0;
}

bool FkCollectorDevice::hasDevice(std::map<uint32_t, FkCollectorDeviceInfo>& deviceInfos) {
    if (!deviceInfos.empty()) return true;

    // 如果还没有搜索设备，自动搜索
    deviceInfos = getDevices();
    if (deviceInfos.empty()) {
        QMessageBox::critical(nullptr, tr("Error"), tr("No device found."));
        return false;
    }

    return true;
}

std::map<uint32_t, FkCollectorDeviceInfo> FkCollectorDevice::getDevices() {
#ifdef SELF_TEST
    m_deviceInfos[0x10000001] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_TEMPERATURE), 16, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x10000002] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_TEMPERATURE), 16, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x10000003] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_TEMPERATURE), 16, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x20000001] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_VOLTAGE), 8, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x20000002] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_VOLTAGE), 8, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x20000003] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_VOLTAGE), 8, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x20000004] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_VOLTAGE), 8, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x20000005] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_VOLTAGE), 8, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x20000006] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_VOLTAGE), 8, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x20000007] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_VOLTAGE), 8, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x20000008] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_VOLTAGE), 8, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x20000009] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_VOLTAGE), 8, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x200000a0] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_VOLTAGE), 8, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x200000a1] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_VOLTAGE), 8, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x200000a2] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_VOLTAGE), 8, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x30000001] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_CURRENT), 16, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x40000001] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_FREQUENCY), 8, INVALID_DEFAULT_VALUE};
    m_deviceInfos[0x40000002] = {static_cast<FkCollectorModuleType>(FKCOLLECTOR_FREQUENCY), 8, INVALID_DEFAULT_VALUE};
    return m_deviceInfos;
#endif
    auto handle = getDeviceAndReceive();
    if (!handle) return {};

    clearDevices();

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_response.Data[DEVICE_RESPONSE_COMMAND_INDEX] = DEVICE_DISCOVER_RESPONSE_CMD;
    }

    // 准备广播数据
    FkVciCanDataType configRequest, discoverRequest;
    prepareRequest(configRequest, DEVICE_DEFAULT_SERIAL_NUMBER, DEVICE_CONFIG_MODE_CMD, DEVICE_CONFIG_MODE_ENTER, 0);
    prepareRequest(discoverRequest, DEVICE_DEFAULT_SERIAL_NUMBER, DEVICE_DISCOVER_REQUEST_CMD, 0, 0);

    m_isLog = true;

    auto discoverTimeout = m_config.getDiscoveryTimeout()/3;
    for (int i = 0; i < 3; ++i) {
        handle->send(FkVciDataManager::constructCanSendData(m_config.getChannelIndex(), configRequest));
        std::this_thread::sleep_for(std::chrono::milliseconds(discoverTimeout/5));

        handle->send(FkVciDataManager::constructCanSendData(m_config.getChannelIndex(), discoverRequest));
        std::this_thread::sleep_for(std::chrono::milliseconds(discoverTimeout));
    }

    m_isLog = false;
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_deviceInfos;
}

void FkCollectorDevice::showDeviceList() {
    QString deviceList;
    deviceList = QObject::tr("Found %1 devices:\n\n").arg(m_deviceInfos.size());

    QString separator = QString(70, '-') + "\n";
    deviceList += separator;

    for (const auto& [deviceId, deviceInfo] : m_deviceInfos) {
        deviceList += QString("Serial Number: 0x%1    Type: %2    Channels: %3\n")
            .arg(QString::number(deviceId, 16), 8, QLatin1Char('0'))
            .arg(FkCollectorHelper::getModuleTypeString(deviceInfo.type))
            .arg(QString::number(deviceInfo.channels));

        deviceList += separator;
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle(QObject::tr("Device Information"));
    msgBox.setText(deviceList);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::NoIcon);
    msgBox.exec();
}

void FkCollectorDevice::clearDevices() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_deviceInfos.clear();
    }
    emit devicesCleared();
}

int FkCollectorDevice::configDevice(uint32_t deviceId, const FkCollectorModuleInfo &moduleInfo) {
    int result = setConfigMode(deviceId);
    if (0 == result)  {
        result = setDeviceConfigInfo(deviceId, moduleInfo);
        if (result != 0) {
            showError(result);
        }
    }
    return result;
}

void FkCollectorDevice::sendTurnOn(uint32_t deviceId) {
    if (0 != sendConfigCommand(deviceId, DEVICE_DISCOVER_REQUEST_CMD, 0, 0, false)) {
        LOG_ERROR("Failed to turn on for device {}", deviceId);
    }
}
