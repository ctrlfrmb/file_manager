// fkcollectorbind.h
#ifndef FKCOLLECTOR_BIND_H
#define FKCOLLECTOR_BIND_H

#include <QDialog>
#include <map>
#include <memory>
#include "fkcollector/fkcollectordef.h"

class QTableWidget;
class QPushButton;
class QTimer;

class FkCollectorBind : public QDialog {
    Q_OBJECT

public:
    FkCollectorBind(std::map<uint32_t, FkCollectorDeviceInfo>& deviceInfos,
                    const std::map<int, FkCollectorModuleInfo>& moduleInfos,
                    QWidget* parent = nullptr);
    ~FkCollectorBind();

    void setCurrentModuleType(FkCollectorModuleType type);

signals:
    void sigUpdateModuleIcon(int moduleId, bool isBound);

private slots:
    void onBindClicked();
    void onUnbindClicked();
    void onSetWorkModeClicked();
    void onSetConfigModeClicked();
    void onBlinkTimeout();
    void onQuickBindClicked();
    void onQuickUnbindClicked();
    void onTableItemDoubleClicked(int row, int column);
    void onSelectionChanged();

private:
    void setupUi();
    void initConnections();
    void refreshDeviceTable();
    bool bindDevice(uint32_t deviceId, FkCollectorDeviceInfo& device, int moduleId);
    bool unbindDevice(uint32_t deviceId, FkCollectorDeviceInfo& device);
    void updateButtonStates();
    void startBlinkTimer(uint32_t deviceId);
    void stopBlinkTimer();

    // 获取当前选中的设备ID和设备信息
    bool getSelectedDevice(uint32_t& deviceId, FkCollectorDeviceInfo*& deviceInfo);
    // 获取可用于绑定的模块列表
    QMap<int, QString> getAvailableModules(FkCollectorModuleType type) const;
    // 更新单行数据
    void updateTableRow(int row, uint32_t deviceId, const FkCollectorDeviceInfo& deviceInfo);

    std::map<uint32_t, FkCollectorDeviceInfo>& deviceInfos_;
    std::map<int, FkCollectorModuleInfo> moduleInfos_;
    FkCollectorModuleType currentModuleType_;
    uint32_t blinkingDeviceId_{0};

    QTableWidget* deviceTable_{nullptr};
    QPushButton* bindButton_{nullptr};
    QPushButton* unbindButton_{nullptr};
    QPushButton* workModeButton_{nullptr};
    QPushButton* configModeButton_{nullptr};
    QPushButton* quickBindButton_{nullptr};
    QPushButton* quickUnbindButton_{nullptr};
    std::unique_ptr<QTimer> blinkTimer_;

    // 存储设备ID和表格行的映射关系
    std::map<uint32_t, int> deviceRowMap_;
};

#endif // FKCOLLECTOR_BIND_H
