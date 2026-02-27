#ifndef FK_COLLECTOR_HELPER_H
#define FK_COLLECTOR_HELPER_H

#include <QTreeWidget>
#include <map>
#include <QPushButton>
#include <QTableWidget>
#include "fkcollector/fkcollectordef.h"

class FkCollectorHelper : public QObject
{
    Q_OBJECT

public:
    FkCollectorHelper();

    static QString getModuleTypeString(FkCollectorModuleType type);
    static void addModuleTypeItem(QTreeWidget *treeWidget, FkCollectorModuleType type, const QString &iconPath);
    static void initTreeModule(QTreeWidget *treeWidget);
    static QString getModuleDbcFile(FkCollectorModuleType type);
    static FkCollectorModuleInfo addModuleInfo(FkCollectorModuleType type);
    static void updateModuleMessageName(FkCollectorModuleInfo &moduleInfo);
    static void updateModuleSignalInfo(FkCollectorModuleInfo &moduleInfo, std::unordered_set<QString> &signalNames);
    static void addModuleInfoItem(QTreeWidgetItem *parent, const QString &name, int id);
    static bool addModuleInfoToTree(QTreeWidget *treeModule, std::map<int, FkCollectorModuleInfo> &moduleInfos,
                                     std::unordered_set<QString> &signalNames);
    static bool isModuleItem(QTreeWidgetItem *item);
    static void initModuleIcon(QTreeWidget *treeModule);
    static void updateModuleIcon(QTreeWidget *treeModule, int moduleId, bool isBound);
    static QTreeWidgetItem* findModuleItem(QTreeWidget* treeModule, int moduleId);
    static void updateTableSignalList(QTableWidget *tableSignalList, const std::vector<OpenSource::DBCMessage> &messages);
    static bool saveModuleInfoToJson(const std::map<int, FkCollectorModuleInfo> &moduleInfos);
    static bool loadModuleInfoFromJson(QTreeWidget *treeWidget, std::map<int, FkCollectorModuleInfo> &moduleInfos);
    static QTreeWidgetItem *findModuleTypeItem(QTreeWidget *treeWidget, FkCollectorModuleType type);
    static void setButtonStyle(QPushButton *button);

    static bool isModuleBound(int moduleId, const std::map<uint32_t, FkCollectorDeviceInfo>& deviceInfos);
    static bool resetDeviceModuleID(int moduleId, std::map<uint32_t, FkCollectorDeviceInfo>& deviceInfos);

    static bool exportDbcFile(QWidget *parent, const std::map<int, FkCollectorModuleInfo> &moduleInfos);
    static bool mergeDbcFile(QWidget *parent, const std::map<int, FkCollectorModuleInfo> &moduleInfos);
};

#endif // FK_COLLECTOR_HELPER_H
