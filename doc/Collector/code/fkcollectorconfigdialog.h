#ifndef FK_COLLECTOR_CONFIG_DIALOG_H
#define FK_COLLECTOR_CONFIG_DIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QButtonGroup>
#include "fkcollector/fkcollectordef.h"

class FkCollectorConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FkCollectorConfigDialog(QWidget *parent = nullptr);
    ~FkCollectorConfigDialog() = default;

private:
    // UI组件
    QGroupBox *canGroup_;
    QButtonGroup *canTypeButtonGroup_;
    QComboBox *deviceComboBox_;
    QComboBox *channelComboBox_;

    // 模块配置组件
    QComboBox *moduleTypeComboBox_;
    QSpinBox *messageIdSpinBox_;
    QSpinBox *frequencySpinBox_;
    QSpinBox *timeoutSpinBox_;

    // 初始化函数
    void initUI();
    QWidget* createCanConfigGroup();
    QWidget* createModuleConfigGroup();
    void loadCurrentConfig();
    void updateModuleConfig(FkCollectorModuleType type);
    void updateChannelComboBox();
    bool shouldShowCanGroup();
    void adjustDialogSize();

private slots:
    void onModuleTypeChanged(int index);
    void onDeviceChanged(int index);
    void onOkButtonClicked();
    void onCancelButtonClicked();
};

#endif // FK_COLLECTOR_CONFIG_DIALOG_H
