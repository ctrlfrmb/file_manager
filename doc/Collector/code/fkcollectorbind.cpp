// fkcollectorbind.cpp
#include "fkcollector/fkcollectorbind.h"
#include "fkcollector/fkcollectordevice.h"
#include "fkcollector/fkcollectorhelper.h"
#include "fkcollector/fkcollectorutils.h"
#include "fkcollector/fkmoduleselectdialog.h"
#include "common/loggermacros.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QMessageBox>
#include <QHeaderView>
#include <QTimer>
#include <QClipboard>
#include <QApplication>
#include <QMap>
#include <set>
#include <vector>
#include <algorithm>

// 构造函数实现
FkCollectorBind::FkCollectorBind(
    std::map<uint32_t, FkCollectorDeviceInfo>& deviceInfos,
    const std::map<int, FkCollectorModuleInfo>& moduleInfos,
    QWidget* parent)
    : QDialog(parent)
    , deviceInfos_(deviceInfos)
    , moduleInfos_(moduleInfos)
    , blinkTimer_(std::make_unique<QTimer>())
{
    setupUi();
    initConnections();
}

// 析构函数实现
FkCollectorBind::~FkCollectorBind() {
    stopBlinkTimer();
}

void FkCollectorBind::setupUi()
{
    setWindowTitle(tr("Module Binding"));
    setMinimumSize(640, 480);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 设备表格
    deviceTable_ = new QTableWidget;
    deviceTable_->setColumnCount(4);
    deviceTable_->setHorizontalHeaderLabels({
        tr("Module"), tr("Serial Number"), tr("Type"), tr("Channels")
    });
    deviceTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    deviceTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    deviceTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    deviceTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    deviceTable_->setAlternatingRowColors(true);
    mainLayout->addWidget(deviceTable_);

    // 按钮区域
    auto buttonLayout = new QHBoxLayout;

    // 添加绑定/解绑按钮
    bindButton_ = new QPushButton(tr("Bind"));
    unbindButton_ = new QPushButton(tr("Unbind"));
    buttonLayout->addWidget(bindButton_);
    buttonLayout->addWidget(unbindButton_);

    // 添加快速绑定/解绑按钮
    buttonLayout->addStretch();
    quickBindButton_ = new QPushButton(tr("Quick Bind All"));
    quickUnbindButton_ = new QPushButton(tr("Quick Unbind All"));
    buttonLayout->addWidget(quickBindButton_);
    buttonLayout->addWidget(quickUnbindButton_);
    buttonLayout->addStretch();

    // 添加工作/配置模式按钮
    workModeButton_ = new QPushButton(tr("Set Work Mode"));
    configModeButton_ = new QPushButton(tr("Set Config Mode"));
    buttonLayout->addWidget(workModeButton_);
    buttonLayout->addWidget(configModeButton_);

    mainLayout->addLayout(buttonLayout);
    updateButtonStates();
}

void FkCollectorBind::initConnections()
{
    connect(bindButton_, &QPushButton::clicked, this, &FkCollectorBind::onBindClicked);
    connect(unbindButton_, &QPushButton::clicked, this, &FkCollectorBind::onUnbindClicked);
    connect(workModeButton_, &QPushButton::clicked, this, &FkCollectorBind::onSetWorkModeClicked);
    connect(configModeButton_, &QPushButton::clicked, this, &FkCollectorBind::onSetConfigModeClicked);
    connect(deviceTable_->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &FkCollectorBind::onSelectionChanged);
    connect(blinkTimer_.get(), &QTimer::timeout, this, &FkCollectorBind::onBlinkTimeout);
    connect(quickBindButton_, &QPushButton::clicked, this, &FkCollectorBind::onQuickBindClicked);
    connect(quickUnbindButton_, &QPushButton::clicked, this, &FkCollectorBind::onQuickUnbindClicked);
    connect(deviceTable_, &QTableWidget::cellDoubleClicked, this, &FkCollectorBind::onTableItemDoubleClicked);
}

void FkCollectorBind::refreshDeviceTable()
{
    deviceTable_->setRowCount(0);
    deviceRowMap_.clear();

    // 过滤出当前类型的设备
    std::vector<std::pair<uint32_t, FkCollectorDeviceInfo*>> filteredDevices;
    for (auto& [deviceId, deviceInfo] : deviceInfos_) {
        if (deviceInfo.type == currentModuleType_) {
            filteredDevices.emplace_back(deviceId, &deviceInfo);
        }
    }

    // 设置表格行数
    deviceTable_->setRowCount(static_cast<int>(filteredDevices.size()));

    // 填充表格
    for (size_t i = 0; i < filteredDevices.size(); ++i) {
        const auto& [deviceId, deviceInfo] = filteredDevices[i];

        // 存储设备ID和行号的映射关系
        deviceRowMap_[deviceId] = static_cast<int>(i);

        // 填充当前行
        updateTableRow(static_cast<int>(i), deviceId, *deviceInfo);
    }

    updateButtonStates();
}

void FkCollectorBind::updateTableRow(int row, uint32_t deviceId, const FkCollectorDeviceInfo& deviceInfo)
{
    if (row < 0 || row >= deviceTable_->rowCount()) {
        return;
    }

    // 模块名称（已绑定或未绑定）
    QTableWidgetItem* moduleItem = new QTableWidgetItem();
    if (deviceInfo.moduleId != INVALID_DEFAULT_VALUE) {
        auto it = moduleInfos_.find(deviceInfo.moduleId);
        if (it != moduleInfos_.end()) {
            moduleItem->setText(it->second.name);
        } else {
            moduleItem->setText(tr("Unknown Module"));
        }
    } else {
        moduleItem->setText(tr("Unbound"));
    }
    deviceTable_->setItem(row, 0, moduleItem);

    // 序列号
    deviceTable_->setItem(row, 1, new QTableWidgetItem(
        QString("0x%1").arg(deviceId, 8, 16, QChar('0'))));

    // 设备类型
    deviceTable_->setItem(row, 2, new QTableWidgetItem(
        FkCollectorHelper::getModuleTypeString(deviceInfo.type)));

    // 通道数
    deviceTable_->setItem(row, 3, new QTableWidgetItem(
        QString::number(deviceInfo.channels)));
}

bool FkCollectorBind::getSelectedDevice(uint32_t& deviceId, FkCollectorDeviceInfo*& deviceInfo)
{
    int row = deviceTable_->currentRow();
    if (row < 0) return false;

    // 从映射表中查找对应的设备ID
    for (const auto& [id, rowIndex] : deviceRowMap_) {
        if (rowIndex == row) {
            deviceId = id;
            deviceInfo = &deviceInfos_[deviceId];
            return true;
        }
    }

    return false;
}

QMap<int, QString> FkCollectorBind::getAvailableModules(FkCollectorModuleType type) const
{
    QMap<int, QString> modules;

    // 收集已绑定的模块ID
    std::set<int> boundModules;
    for (const auto& [devId, devInfo] : deviceInfos_) {
        if (devInfo.moduleId != INVALID_DEFAULT_VALUE) {
            boundModules.insert(devInfo.moduleId);
        }
    }

    // 添加所有未绑定的、匹配当前类型的模块
    for (const auto& [moduleId, moduleInfo] : moduleInfos_) {
        if (boundModules.find(moduleId) == boundModules.end() &&
            moduleInfo.type == type) {
            modules.insert(moduleId, moduleInfo.name);
        }
    }

    return modules;
}

void FkCollectorBind::onBindClicked()
{
    uint32_t deviceId;
    FkCollectorDeviceInfo* deviceInfo;
    if (!getSelectedDevice(deviceId, deviceInfo)) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a module first."));
        return;
    }

    // 如果设备已经绑定了模块，提示先解绑
    if (deviceInfo->moduleId != INVALID_DEFAULT_VALUE) {
        QMessageBox::warning(this, tr("Warning"),
            tr("Device is already bound to a module. Please unbind it first."));
        return;
    }

    // 获取可用模块列表
    QMap<int, QString> availableModules = getAvailableModules(currentModuleType_);
    if (availableModules.isEmpty()) {
        QMessageBox::information(this, tr("Information"),
            tr("No available modules to bind with."));
        return;
    }

    // 显示模块选择对话框
    FkModuleSelectDialog dialog(availableModules, this);
    if (dialog.exec() == QDialog::Accepted) {
        int moduleId = dialog.getSelectedModuleId();
        if (moduleId != -1) {
            bindDevice(deviceId, *deviceInfo, moduleId);
        }
    }
}

void FkCollectorBind::onUnbindClicked()
{
    uint32_t deviceId;
    FkCollectorDeviceInfo* deviceInfo;
    if (!getSelectedDevice(deviceId, deviceInfo)) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a device first."));
        return;
    }

    // 如果设备未绑定模块，提示无需解绑
    if (deviceInfo->moduleId == INVALID_DEFAULT_VALUE) {
        QMessageBox::information(this, tr("Information"),
            tr("Device is not bound to any module."));
        return;
    }

    // 询问用户是否确认解绑
    QString moduleName = tr("Unknown");
    auto it = moduleInfos_.find(deviceInfo->moduleId);
    if (it != moduleInfos_.end()) {
        moduleName = it->second.name;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Confirm Unbind"),
        tr("Are you sure you want to unbind this device from module '%1'?").arg(moduleName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        unbindDevice(deviceId, *deviceInfo);
    }
}

bool FkCollectorBind::bindDevice(uint32_t deviceId, FkCollectorDeviceInfo& device, int moduleId)
{
    // 验证模块ID是否有效
    auto it = moduleInfos_.find(moduleId);
    if (it == moduleInfos_.end()) {
        return false;
    }

    // 检查模块是否已被其他设备绑定
    for (const auto& [otherDeviceId, otherDeviceInfo] : deviceInfos_) {
        if (otherDeviceId != deviceId &&
            otherDeviceInfo.moduleId == moduleId) {
            QMessageBox::warning(this, tr("Warning"),
                tr("Module '%1' is already bound to another device.").arg(it->second.name));
            return false;
        }
    }

    // 创建moduleInfo的副本，防止configDevice修改原始数据
    FkCollectorModuleInfo moduleInfoCopy = it->second;

    // 使用副本配置设备
    int ret = FkCollectorDevice::getInstance().configDevice(deviceId, moduleInfoCopy);
    if (ret != 0) {
        QMessageBox::critical(this, tr("Error"),
            tr("Failed to bind device to module '%1', error code: 0x%2")
                .arg(it->second.name)
                .arg(ret, 0, 16));
        return false;
    }

    // 更新设备绑定状态
    device.moduleId = moduleId;

    // 仅更新当前行，不刷新整个表格
    auto rowIt = deviceRowMap_.find(deviceId);
    if (rowIt != deviceRowMap_.end()) {
        updateTableRow(rowIt->second, deviceId, device);
    }

    // 更新按钮状态
    updateButtonStates();

    // 发送信号通知绑定状态改变
    emit sigUpdateModuleIcon(moduleId, true);

    // 显示成功提示
    QMessageBox::information(this, tr("Success"),
        tr("Successfully bound device (SN: 0x%1) to module '%2'")
            .arg(QString::number(deviceId, 16))
            .arg(it->second.name));

    return true;
}

bool FkCollectorBind::unbindDevice(uint32_t deviceId, FkCollectorDeviceInfo& device)
{
    if (device.moduleId == INVALID_DEFAULT_VALUE) {
        return true;  // 设备本来就是未绑定状态
    }

    int oldModuleId = device.moduleId;
    QString moduleName = tr("Unknown");
    auto it = moduleInfos_.find(oldModuleId);
    if (it != moduleInfos_.end()) {
        moduleName = it->second.name;
    }

    // 更新设备绑定状态
    device.moduleId = INVALID_DEFAULT_VALUE;

    // 仅更新当前行，不刷新整个表格
    auto rowIt = deviceRowMap_.find(deviceId);
    if (rowIt != deviceRowMap_.end()) {
        updateTableRow(rowIt->second, deviceId, device);
    }

    // 更新按钮状态
    updateButtonStates();

    // 发送信号通知绑定状态改变
    emit sigUpdateModuleIcon(oldModuleId, false);

    // 显示成功提示
    QMessageBox::information(this, tr("Success"),
        tr("Successfully unbound device (SN: 0x%1) from module '%2'")
            .arg(QString::number(deviceId, 16))
            .arg(moduleName));

    return true;
}

void FkCollectorBind::onQuickBindClicked()
{
    // 获取当前类型的所有未绑定设备
    std::vector<std::pair<uint32_t, FkCollectorDeviceInfo*>> unboundDevices;
    for (auto& [deviceId, deviceInfo] : deviceInfos_) {
        if (deviceInfo.type == currentModuleType_ &&
            deviceInfo.moduleId == INVALID_DEFAULT_VALUE) {
            unboundDevices.emplace_back(deviceId, &deviceInfo);
        }
    }

    if (unboundDevices.empty()) {
        QMessageBox::information(this, tr("Information"),
            tr("No unbound devices available."));
        return;
    }

    // 获取当前类型的所有未绑定模块
    QMap<int, QString> availableModulesMap = getAvailableModules(currentModuleType_);
    if (availableModulesMap.isEmpty()) {
        QMessageBox::information(this, tr("Information"),
            tr("No available modules to bind with."));
        return;
    }

    // 执行绑定操作，确保每个模块只绑定一次
    size_t bindCount = 0;
    std::set<int> boundModuleIds; // 跟踪已绑定的模块IDs

    for (auto& [deviceId, deviceInfo] : unboundDevices) {
        // 找到一个未绑定的模块
        int moduleId = -1;
        for (auto it = availableModulesMap.begin(); it != availableModulesMap.end(); ++it) {
            if (boundModuleIds.find(it.key()) == boundModuleIds.end()) {
                moduleId = it.key();
                break;
            }
        }

        if (moduleId != -1) {
            if (bindDevice(deviceId, *deviceInfo, moduleId)) {
                boundModuleIds.insert(moduleId);
                bindCount++;
            }
        } else {
            break; // 没有更多可用的模块
        }
    }

    if (bindCount > 0) {
        QMessageBox::information(this, tr("Success"),
            tr("Successfully bound %1 device(s)").arg(bindCount));
    } else {
        QMessageBox::information(this, tr("Information"),
            tr("No devices were bound."));
    }
}

void FkCollectorBind::onQuickUnbindClicked()
{
    size_t unbindCount = 0;
    for (auto& [deviceId, deviceInfo] : deviceInfos_) {
        if (deviceInfo.type == currentModuleType_ &&
            deviceInfo.moduleId != INVALID_DEFAULT_VALUE) {
            if (unbindDevice(deviceId, deviceInfo)) {
                unbindCount++;
            }
        }
    }

    if (unbindCount > 0) {
        QMessageBox::information(this, tr("Success"),
            tr("Successfully unbound %1 device(s)").arg(unbindCount));
    } else {
        QMessageBox::information(this, tr("Information"),
            tr("No bound devices to unbind"));
    }
}

void FkCollectorBind::onTableItemDoubleClicked(int row, int column)
{
    if (column == 1) {
        QTableWidgetItem* item = deviceTable_->item(row, column);
        if (item) {
            // 获取序列号文本并复制到剪贴板
            QString serialText = item->text();
            QApplication::clipboard()->setText(serialText);

            // 创建一个非模态的信息框
            QMessageBox* msgBox = new QMessageBox(
                QMessageBox::Information,
                tr("Copy"),
                tr("Serial number copied to clipboard"),
                QMessageBox::NoButton,
                this
            );
            msgBox->setModal(false);
            msgBox->show();

            // 自动关闭并删除
            QTimer::singleShot(1500, msgBox, &QMessageBox::close);
            QTimer::singleShot(1500, msgBox, &QMessageBox::deleteLater);
        }
    }
}

void FkCollectorBind::onSelectionChanged()
{
    uint32_t deviceId;
    FkCollectorDeviceInfo* deviceInfo;
    if (getSelectedDevice(deviceId, deviceInfo)) {
        startBlinkTimer(deviceId);
    } else {
        stopBlinkTimer();
    }

    updateButtonStates();
}

void FkCollectorBind::updateButtonStates()
{
    bool hasSelection = deviceTable_->currentRow() >= 0;
    workModeButton_->setEnabled(hasSelection);
    configModeButton_->setEnabled(hasSelection);

    // 更新绑定/解绑按钮状态
    if (hasSelection) {
        uint32_t deviceId;
        FkCollectorDeviceInfo* deviceInfo;
        if (getSelectedDevice(deviceId, deviceInfo)) {
            bool isBound = deviceInfo->moduleId != INVALID_DEFAULT_VALUE;
            bindButton_->setEnabled(!isBound);
            unbindButton_->setEnabled(isBound);
        }
    } else {
        bindButton_->setEnabled(false);
        unbindButton_->setEnabled(false);
    }

    // 更新快速绑定/解绑按钮状态
    bool hasUnboundDevices = false;
    bool hasBoundDevices = false;
    for (const auto& [deviceId, deviceInfo] : deviceInfos_) {
        if (deviceInfo.type == currentModuleType_) {
            if (deviceInfo.moduleId == INVALID_DEFAULT_VALUE) {
                hasUnboundDevices = true;
            } else {
                hasBoundDevices = true;
            }
        }
    }

    quickBindButton_->setEnabled(hasUnboundDevices);
    quickUnbindButton_->setEnabled(hasBoundDevices);
}

void FkCollectorBind::onSetWorkModeClicked()
{
    uint32_t deviceId;
    FkCollectorDeviceInfo* deviceInfo;
    if (!getSelectedDevice(deviceId, deviceInfo)) return;

    // 设置工作模式
    int ret = FkCollectorDevice::getInstance().setWorkMode(deviceId);
    if (ret != 0) {
        QMessageBox::critical(this, tr("Error"),
            tr("Failed to set work mode, error code: 0x%1").arg(ret, 0, 16));
    } else {
        QMessageBox::information(this, tr("Success"), tr("Work mode set successfully."));
    }
}

void FkCollectorBind::onSetConfigModeClicked()
{
    uint32_t deviceId;
    FkCollectorDeviceInfo* deviceInfo;
    if (!getSelectedDevice(deviceId, deviceInfo)) return;

    // 设置配置模式
    int ret = FkCollectorDevice::getInstance().setConfigMode(deviceId);
    if (ret != 0) {
        QMessageBox::critical(this, tr("Error"),
            tr("Failed to set config mode, error code: 0x%1").arg(ret, 0, 16));
    } else {
        QMessageBox::information(this, tr("Success"), tr("Config mode set successfully."));
    }
}

void FkCollectorBind::setCurrentModuleType(FkCollectorModuleType type)
{
    if (currentModuleType_ != type) {
        currentModuleType_ = type;
        refreshDeviceTable();
    }
}

void FkCollectorBind::onBlinkTimeout()
{
    if (blinkingDeviceId_ != 0) {
        FkCollectorDevice::getInstance().sendTurnOn(blinkingDeviceId_);
    }
}

void FkCollectorBind::startBlinkTimer(uint32_t deviceId)
{
    stopBlinkTimer();
    blinkingDeviceId_ = deviceId;
    blinkTimer_->start(1000);
}

void FkCollectorBind::stopBlinkTimer()
{
    blinkTimer_->stop();
    blinkingDeviceId_ = 0;
}
