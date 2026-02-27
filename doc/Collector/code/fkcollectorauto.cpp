// fkcollectorauto.cpp
#include "fkcollector/fkcollectorauto.h"
#include "fkcollector/fkcollectordevice.h"
#include "fkcollector/fkcollectorhelper.h"
#include <QMessageBox>

FkCollectorAuto::FkCollectorAuto(
    std::map<uint32_t, FkCollectorDeviceInfo>& deviceInfos,
    std::map<int, FkCollectorModuleInfo>& moduleInfos,
    std::unordered_set<QString>& signalNames,
    QObject* parent)
    : QObject(parent)
    , deviceInfos_(deviceInfos)
    , moduleInfos_(moduleInfos)
    , signalNames_(signalNames)
{
}

FkCollectorAuto::~FkCollectorAuto()
{
}

void FkCollectorAuto::clearAllModules(QTreeWidget* treeModule)
{
    // 清除所有模块相关的信号名
    for (const auto& [id, moduleInfo] : moduleInfos_) {
        for (const auto& message : moduleInfo.messages) {
            for (const auto& signal : message.signalList) {
                signalNames_.erase(signal.newName);
            }
        }
    }

    // 重置所有设备的模块ID
    for (auto& [deviceId, deviceInfo] : deviceInfos_) {
        deviceInfo.moduleId = INVALID_DEFAULT_VALUE;
    }

    // 清除所有模块信息
    deviceInfos_.clear();

    // 清除树形控件中的所有模块节点
    if (treeModule) {
        for (int i = 0; i < treeModule->topLevelItemCount(); ++i) {
            auto typeItem = treeModule->topLevelItem(i);
            while (typeItem->childCount() > 0) {
                delete typeItem->takeChild(0);
            }
        }
    }
}

bool FkCollectorAuto::autoCreateModules(QTreeWidget* treeModule)
{
    if (!treeModule) {
        QMessageBox::warning(nullptr, tr("Error"), tr("Tree widget not set."));
        return false;
    }

    // 检查是否有现有模块
    bool hasModules = false;
    for (int i = 0; i < treeModule->topLevelItemCount(); ++i) {
        if (treeModule->topLevelItem(i)->childCount() > 0) {
            hasModules = true;
            break;
        }
    }

    if (hasModules) {
        auto ret = QMessageBox::question(nullptr, tr("Warning"),
            tr("Existing modules will be cleared. Continue?"),
            QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) {
            return false;
        }
        clearAllModules(treeModule);
    }

    // 搜索设备
    deviceInfos_ = FkCollectorDevice::getInstance().getDevices();
    if (deviceInfos_.empty()) {
        QMessageBox::warning(nullptr, tr("Warning"), tr("No devices found."));
        return false;
    }

    // 统计每种类型的设备数量
    std::map<FkCollectorModuleType, int> deviceCounts;
    for (const auto& [deviceId, deviceInfo] : deviceInfos_) {
        deviceCounts[deviceInfo.type]++;
    }

    // 为每种类型创建对应数量的模块
    int totalModules = 0;
    for (const auto& [type, count] : deviceCounts) {
        if (count == 0) continue;

        // 获取类型节点
        QTreeWidgetItem* typeItem = nullptr;
        for (int i = 0; i < treeModule->topLevelItemCount(); ++i) {
            auto item = treeModule->topLevelItem(i);
            if (item->data(0, Qt::UserRole).toInt() == type) {
                typeItem = item;
                break;
            }
        }
        if (!typeItem) continue;

        // 设置当前选中项并创建模块
        treeModule->setCurrentItem(typeItem);
        for (int i = 0; i < count && i < FKCOLLECTOR_MODULE_CHANNEL_SIZE; ++i) {
            if (FkCollectorHelper::addModuleInfoToTree(treeModule, moduleInfos_, signalNames_)) {
                totalModules++;
            }
        }
    }

    // 展开所有节点
    treeModule->expandAll();

    if (totalModules > 0) {
        QMessageBox::information(nullptr, tr("Success"),
            tr("Successfully created %1 modules.").arg(totalModules));
        return true;
    } else {
        QMessageBox::warning(nullptr, tr("Warning"),
            tr("No modules were created."));
        return false;
    }
}

bool FkCollectorAuto::autoBindModules()
{
    int bindCount = 0;

    // 按类型进行绑定
    for (const auto& [deviceId, deviceInfo] : deviceInfos_) {
        // 查找对应类型的未绑定模块
        for (const auto& [moduleId, moduleInfo] : moduleInfos_) {
            if (moduleInfo.type == deviceInfo.type &&
                !FkCollectorHelper::isModuleBound(moduleId, deviceInfos_)) {

                // 配置设备
                int ret = FkCollectorDevice::getInstance().configDevice(deviceId, moduleInfo);
                if (ret == 0) {
                    // 更新绑定状态
                    auto& device = deviceInfos_[deviceId];
                    device.moduleId = moduleId;

                    // 更新模块图标
                    emit sigUpdateModuleIcon(moduleId, true);

                    bindCount++;
                    break;
                }
            }
        }
    }

    if (bindCount > 0) {
        if (QMessageBox::Yes == QMessageBox::question(
                    nullptr,
                    tr("Enter Work Mode"),
                    tr("Successfully bound %1 device(s) to modules. Do you want to enter work mode?").arg(bindCount),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::Yes
                )) return true;
    } else {
        QMessageBox::warning(nullptr, tr("Warning"),
            tr("No devices were bound to modules."));
    }
    return false;
}
