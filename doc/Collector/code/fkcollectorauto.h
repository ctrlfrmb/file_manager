// fkcollectorauto.h
#ifndef FK_COLLECTOR_AUTO_H
#define FK_COLLECTOR_AUTO_H

#include <QObject>
#include <QTreeWidget>
#include <map>
#include <unordered_set>
#include "fkcollector/fkcollectordef.h"

class FkCollectorAuto : public QObject
{
    Q_OBJECT

public:
    FkCollectorAuto(std::map<uint32_t, FkCollectorDeviceInfo>& deviceInfos,
                    std::map<int, FkCollectorModuleInfo>& moduleInfos,
                    std::unordered_set<QString>& signalNames,
                    QObject* parent = nullptr);
    ~FkCollectorAuto();

    // 一键生成模块
    bool autoCreateModules(QTreeWidget* treeModule);

    // 一键绑定模块
    bool autoBindModules();

signals:
    void sigUpdateModuleIcon(int moduleId, bool isBound);

private:
    // 清除所有模块
    void clearAllModules(QTreeWidget* treeModule);

    std::map<uint32_t, FkCollectorDeviceInfo>& deviceInfos_;
    std::map<int, FkCollectorModuleInfo>& moduleInfos_;
    std::unordered_set<QString>& signalNames_;
};

#endif // FK_COLLECTOR_AUTO_H
