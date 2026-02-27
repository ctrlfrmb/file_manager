#include "fkcollector/fkcollectorconfigdialog.h"
#include "fkcollector/fkcollectorconfig.h"
#include "fkvci/fkvcistatusmanager.h"
#include "fkvci/fkvcimanager.h"
#include "fkuihelper.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>

FkCollectorConfigDialog::FkCollectorConfigDialog(QWidget *parent)
    : QDialog(parent)
    , canGroup_(nullptr)
    , canTypeButtonGroup_(nullptr)
    , deviceComboBox_(nullptr)
    , channelComboBox_(nullptr)
    , moduleTypeComboBox_(nullptr)
    , messageIdSpinBox_(nullptr)
    , frequencySpinBox_(nullptr)
    , timeoutSpinBox_(nullptr)
{
    initUI();
    loadCurrentConfig();
    adjustDialogSize();
}

void FkCollectorConfigDialog::initUI()
{
    setWindowTitle(tr("Module Configuration"));
    setWindowIcon(QIcon(":/resource/module.png"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(3, 3, 3, 3);
    mainLayout->setSpacing(5);

    // 只有在需要时才创建和添加CAN配置组
    if (shouldShowCanGroup()) {
        canGroup_ = qobject_cast<QGroupBox*>(createCanConfigGroup());
        if (canGroup_) {
            mainLayout->addWidget(canGroup_);
        }
    }

    mainLayout->addWidget(createModuleConfigGroup());
    mainLayout->addStretch();

    // 底部布局
    QGridLayout *bottomLayout = new QGridLayout();
    bottomLayout->setSpacing(5);

    QLabel *timeoutLabel = new QLabel(tr("Discovery Time:"), this);
    timeoutSpinBox_ = new QSpinBox(this);
    timeoutSpinBox_->setRange(0, 60000);

    QPushButton *okButton = new QPushButton(tr("OK"), this);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), this);

    bottomLayout->addWidget(timeoutLabel, 0, 0);
    bottomLayout->addWidget(timeoutSpinBox_, 0, 1);
    bottomLayout->addWidget(okButton, 0, 3);
    bottomLayout->addWidget(cancelButton, 0, 4);
    bottomLayout->setColumnStretch(2, 1);  // 在时间和按钮之间添加弹性空间

    mainLayout->addLayout(bottomLayout);

    connect(okButton, &QPushButton::clicked, this, &FkCollectorConfigDialog::onOkButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &FkCollectorConfigDialog::onCancelButtonClicked);
}

QWidget* FkCollectorConfigDialog::createCanConfigGroup()
{
    QGroupBox *canGroup = new QGroupBox(tr("CAN Configuration"), this);
    QGridLayout *layout = new QGridLayout(canGroup);

    // 设备选择
    QLabel *deviceLabel = new QLabel(tr("Device:"), this);
    deviceComboBox_ = FkUiHelper::newDeviceComboBox(FKVCI_CAN, this);
    layout->addWidget(deviceLabel, 0, 0);
    layout->addWidget(deviceComboBox_, 0, 1);

    // 通道选择
    QLabel *channelLabel = new QLabel(tr("Channel:"), this);
    channelComboBox_ = new QComboBox(this);
    layout->addWidget(channelLabel, 1, 0);
    layout->addWidget(channelComboBox_, 1, 1);

    connect(deviceComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FkCollectorConfigDialog::onDeviceChanged);

    return canGroup;
}

QWidget* FkCollectorConfigDialog::createModuleConfigGroup()
{
    QGroupBox *moduleGroup = new QGroupBox(tr("Module Configuration"), this);
    QGridLayout *layout = new QGridLayout(moduleGroup);

    int row = 0;

    // CAN类型选择
    QLabel *canTypeLabel = new QLabel(tr("CAN Message Type:"), this);
    canTypeButtonGroup_ = new QButtonGroup(this);

    QWidget *radioWidget = new QWidget(this);
    QHBoxLayout *radioLayout = new QHBoxLayout(radioWidget);

    QRadioButton *canRadio = new QRadioButton(tr("CAN"), this);
    QRadioButton *canfdRadio = new QRadioButton(tr("CANFD"), this);

    canTypeButtonGroup_->addButton(canRadio, FKCOLLECTOR_CAN);
    canTypeButtonGroup_->addButton(canfdRadio, FKCOLLECTOR_CANFD);

    radioLayout->addWidget(canRadio);
    radioLayout->addWidget(canfdRadio);

    layout->addWidget(canTypeLabel, row, 0);
    layout->addWidget(radioWidget, row, 1);
    row++;

    // 模块类型选择
    QLabel *typeLabel = new QLabel(tr("Module Type:"), this);
    moduleTypeComboBox_ = new QComboBox(this);

    auto& config = FkCollectorConfig::getInstance();
    for (const auto& [type, _] : config.getConfig().modules) {
        auto moduleType = static_cast<FkCollectorModuleType>(type);
        moduleTypeComboBox_->addItem(config.getModuleTypeName(moduleType), type);
    }

    layout->addWidget(typeLabel, row, 0);
    layout->addWidget(moduleTypeComboBox_, row, 1);
    row++;

    // 消息ID配置
    QLabel *idLabel = new QLabel(tr("Default Start ID:"), this);
    messageIdSpinBox_ = new QSpinBox(this);
    messageIdSpinBox_->setRange(0, OpenSource::DBC_MESSAGE_ID_MAX);
    messageIdSpinBox_->setPrefix("0x");
    messageIdSpinBox_->setDisplayIntegerBase(16);
    layout->addWidget(idLabel, row, 0);
    layout->addWidget(messageIdSpinBox_, row, 1);
    row++;

    // 频率配置
    QLabel *freqLabel = new QLabel(tr("Default Period Time(ms):"), this);
    frequencySpinBox_ = new QSpinBox(this);
    frequencySpinBox_->setRange(1, 1000);
    layout->addWidget(freqLabel, row, 0);
    layout->addWidget(frequencySpinBox_, row, 1);

    connect(moduleTypeComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FkCollectorConfigDialog::onModuleTypeChanged);

    return moduleGroup;
}

void FkCollectorConfigDialog::adjustDialogSize()
{
    // 根据是否显示CAN配置组来设置窗口大小
    if (canGroup_) {
        setMinimumSize(360, 240);  // CAN配置组显示时的大小
    } else {
        setMinimumSize(360, 180);  // 不显示CAN配置组时的大小
    }
}

bool FkCollectorConfigDialog::shouldShowCanGroup()
{
    const auto &vciSM = FkVciStatusManager::getInstance();
    auto& config = FkCollectorConfig::getInstance();
    int totalOpenChannels = 0;

    // 统计打开的设备和通道数
    for (int i = 0; i < FKVCI_DEVICE_EQUIPMENT_QUANTITY; ++i) {
        for (int j = 0; j < FKVCI_DEVICE_CHANNEL_NUM; ++j) {
            if (vciSM.canChannelIsOpened(i, j)) {
                config.setDeviceIndex(i);
                config.setChannelIndex(j);
                totalOpenChannels++;
            }
        }
    }

    // 如果只有一个通道打开，则不显示CAN配置组
    return totalOpenChannels > 1;
}

void FkCollectorConfigDialog::updateChannelComboBox()
{
    if (!channelComboBox_) return;

    channelComboBox_->clear();
    int currentDevice = deviceComboBox_->currentData().toInt();
    const auto &config = FkVciStatusManager::getInstance();

    for (int i = FKVCI_DEVICE_CHANNEL_BEGIN; i < FKVCI_DEVICE_CHANNEL_NUM; ++i) {
        if (config.canChannelIsOpened(currentDevice, i)) {
            channelComboBox_->addItem(tr("CAN%1").arg(i+1), i);
        }
    }
}

void FkCollectorConfigDialog::loadCurrentConfig()
{
    auto& config = FkCollectorConfig::getInstance();

    // 加载CAN配置
    if (canGroup_) {
        // 设置当前设备
        int deviceComboIndex = deviceComboBox_->findData(config.getDeviceIndex());
        if (deviceComboIndex >= 0) {
            deviceComboBox_->setCurrentIndex(deviceComboIndex);
        }

        // 更新并设置当前通道
        updateChannelComboBox();
        int channelComboIndex = channelComboBox_->findData(config.getChannelIndex());
        if (channelComboIndex >= 0) {
            channelComboBox_->setCurrentIndex(channelComboIndex);
        }
    }

    // 设置CAN类型单选按钮
    QAbstractButton *button = canTypeButtonGroup_->button(config.getCanType());
    if (button) {
        button->setChecked(true);
    }

    // 加载当前选中模块类型的配置
    if (moduleTypeComboBox_->count() > 0) {
        onModuleTypeChanged(moduleTypeComboBox_->currentIndex());
    }

    timeoutSpinBox_->setValue(config.getDiscoveryTimeout());
}

void FkCollectorConfigDialog::updateModuleConfig(FkCollectorModuleType type)
{
    auto& config = FkCollectorConfig::getInstance();
    messageIdSpinBox_->setValue(config.getModuleMessageDefaultId(type));
    frequencySpinBox_->setValue(config.getModuleFrequency(type));
}

void FkCollectorConfigDialog::onModuleTypeChanged(int index)
{
    if (index >= 0) {
        auto type = static_cast<FkCollectorModuleType>(moduleTypeComboBox_->currentData().toUInt());
        updateModuleConfig(type);
    }
}

void FkCollectorConfigDialog::onDeviceChanged(int /*index*/)
{
    updateChannelComboBox();
}

void FkCollectorConfigDialog::onOkButtonClicked()
{
    auto& config = FkCollectorConfig::getInstance();

    // 保存CAN配置
    if (canGroup_) {
        config.setDeviceIndex(deviceComboBox_->currentData().toUInt());
        config.setChannelIndex(channelComboBox_->currentData().toUInt());
    }

    // 保存模块配置
    config.setCanType(canTypeButtonGroup_->checkedId());

    auto type = static_cast<FkCollectorModuleType>(moduleTypeComboBox_->currentData().toUInt());
    config.setModuleStartMessageId(type, messageIdSpinBox_->value());
    config.setModuleFrequency(type, frequencySpinBox_->value());
    config.setDiscoveryTimeout(timeoutSpinBox_->value());

    accept();
}

void FkCollectorConfigDialog::onCancelButtonClicked()
{
    reject();
}
