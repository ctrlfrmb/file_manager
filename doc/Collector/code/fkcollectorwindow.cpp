#include "fkcollectorwindow.h"
#include "ui_fkcollectorwindow.h"
#include "fkcollector/fkcollectorconfig.h"
#include "fkcollector/fkcollectorutils.h"
#include "fkcollector/fkcollectorhelper.h"
#include "fkcollector/fkcollectordevice.h"
#include "fkcollector/fkcollectorauto.h"
#include "fkcollector/fkcollectorbind.h"
#include "fkcollector/fkcollectorconfigdialog.h"
#include "fkcollector/fkcollectorpressorewindow.h"
#include "common/loggermacros.h"
#include "common_ui/commonuiutils.h"
#include "common_ui/debuglogdialog.h"
#include "fkvciversiondialog.h"
#include "fkvciconfigdialog.h"
#include "fkvci/fkvcimanager.h"
#include "fkvci/fkvcistatusmanager.h"

#include <QMessageBox>
#include <QLineEdit>

FkCollectorWindow::FkCollectorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FkCollectorWindow)
{
    ui->setupUi(this);
    initWindow();
}

FkCollectorWindow::~FkCollectorWindow()
{
    delete ui;
}

void FkCollectorWindow::initWindow() {
    CommonUiUtils::setStatusBarVersionInfo(ui->statusBar,
                                           QString("%1 %2 %3").arg(SOFT_VERSION).arg(QString(__DATE__)).arg(QString(__TIME__)));
    ui->menubar->setVisible(false);
    ui->actionOpen->setCheckable(false);

    setModuleID();

    FkCollectorHelper::initTreeModule(ui->treeModule);
    initTableConfig();
    initTableSignalList();
    initTableSignal();

    connect(&FkCollectorDevice::getInstance(), &FkCollectorDevice::devicesCleared, this, &FkCollectorWindow::onDevicesCleared);
    FkVciManager::getInstance().setVciStatusChangedCallback(std::bind(&FkCollectorWindow::processVciStatusChanged, this, std::placeholders::_1, std::placeholders::_2));
}

void FkCollectorWindow::exitWindow() {
    FkVciManager::getInstance().closeDevices();
}

void FkCollectorWindow::closeEvent(QCloseEvent *event) {
    int ret = QMessageBox::question(this, tr("Tips"), tr("Are you sure to close the program?"), QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        exitWindow();
        event->accept();
    } else {
        event->ignore();
    }
}

void FkCollectorWindow::start() {
    on_actionOpen_triggered();
    showModuleConfigDialog();
}

void FkCollectorWindow::processVciStatusChanged(int deviceIndex, uint8_t cmd) {
    Q_UNUSED(deviceIndex);
    if (cmd != VIEW_CAN_OPENED && cmd != VIEW_CAN_CLOSED)
        return;

    bool isConnected = isDeviceOpened_.load(std::memory_order_relaxed);
    if (isConnected && cmd == VIEW_CAN_OPENED)
        return;

    if (!isConnected && cmd == VIEW_CAN_CLOSED)
        return;

    if (isConnected && cmd == VIEW_CAN_CLOSED && FkVciStatusManager::getInstance().hasCanChannelsOpened()) {
        return;
    }

    isConnected = cmd == VIEW_CAN_OPENED;
    isDeviceOpened_.store(isConnected, std::memory_order_release);

    QMetaObject::invokeMethod(this, [this, isConnected]() {
        if (!isConnected) {
            FkCollectorDevice::getInstance().clearDevices();
        }

        ui->actionOpen->setCheckable(isConnected);
        ui->actionOpen->setIcon(QIcon(QString(":/resource/vci/vci_%1.png").arg(isConnected?"close":"open")));
        ui->actionOpen->setToolTip(QString("Quickly %1 FKVCI device").arg(isConnected?"close":"open"));
    }, Qt::QueuedConnection);
}

void FkCollectorWindow::initTableConfig() {
    // 设置表格行数和列数
    ui->tableConfig->setRowCount(1); // 只需要一行
    ui->tableConfig->setColumnCount(FKCOLLECTOR_CONFIG_TABLE_COLUMN_COUNT);
    ui->tableConfig->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    // 设置列标题
    QStringList headers;
    headers << "CAN ID" << "Period Time(ms)";
    ui->tableConfig->setHorizontalHeaderLabels(headers);

    for (int col = 0; col < ui->tableConfig->columnCount(); ++col) {
        QSpinBox* spinBox = new QSpinBox(ui->tableConfig);
        spinBox->setRange(0, 1000000); // 设置范围
        spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons); // 隐藏上下按钮
        ui->tableConfig->setCellWidget(0, col, spinBox);

        // 连接信号到槽函数
        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &FkCollectorWindow::onTableConfigItemChanged);
    }

    ui->tableConfig->verticalHeader()->setVisible(false);
    ui->tableConfig->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableConfig->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void FkCollectorWindow::initTableSignalList() {
    ui->tableSignalList->setColumnCount(2);
    ui->tableSignalList->setHorizontalHeaderLabels({"Channel", "Signal Name"});
    ui->tableSignalList->verticalHeader()->setDefaultSectionSize(22);
    ui->tableSignalList->horizontalHeader()->setStretchLastSection(true);
    ui->tableSignalList->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableSignalList->verticalHeader()->setVisible(false);

    connect(ui->tableSignalList, &QTableWidget::currentItemChanged,
                this, &FkCollectorWindow::onTableSignalListCurrentItemChanged);
    connect(ui->tableSignalList, &QTableWidget::itemChanged,
                this, &FkCollectorWindow::onTableSignalListItemChanged);
}

void FkCollectorWindow::initTableSignal() {
    ui->tableSignal->setRowCount(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_COUNT);
    ui->tableSignal->setColumnCount(2);
    ui->tableSignal->setHorizontalHeaderLabels({"Property", "Value"});
    ui->tableSignal->verticalHeader()->setVisible(false);

    QStringList properties = {"Value Unit", "Min Value", "Max Value", "Factor", "Offset", "",
                              "Channel", "Sampling Gear", "Output Voltage"};

    for (int i = 0; i < ui->tableSignal->rowCount(); ++i) {
        ui->tableSignal->setItem(i, 0, new QTableWidgetItem(properties[i]));
        ui->tableSignal->item(i, 0)->setFlags(ui->tableSignal->item(i, 0)->flags() & ~Qt::ItemIsEditable);

        QWidget *editor = nullptr;

        switch (i) {
            case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_UNIT:
                editor = createStringEditor(i);
                break;
            case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_MIN:
            case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_MAX:
            case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_FACTOR:
            case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_OFFSET:
                editor = createDoubleEditor(i);
                break;
            case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_PLOT_BUTTON:
                editor = createPlotButton();
                break;
            case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_CAHNNEL:
                editor = createChannelComboBox();
                break;
            case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_MEASURE_RANGE:
                editor = createMeasureRangeComboBox();
                break;
            case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_OUT_VOLTAGE:
                editor = createOutVoltageComboBox();
                break;
        }

        if (editor) {
            ui->tableSignal->setCellWidget(i, 1, editor);
        }
    }

    ui->tableSignal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

QLineEdit *FkCollectorWindow::createStringEditor(int row) {
    QLineEdit *editor = new QLineEdit(ui->tableSignal);
    connect(editor, &QLineEdit::editingFinished, this, [this, row, editor]() {
        updateSignalEditValue(row, editor->text());
    });
    return editor;
}

QLineEdit *FkCollectorWindow::createDoubleEditor(int row) {
    QLineEdit *editor = new QLineEdit(ui->tableSignal);
    QDoubleValidator *validator = new QDoubleValidator(editor);
    editor->setValidator(validator);
    connect(editor, &QLineEdit::editingFinished, this, [this, row, editor]() {
        updateSignalEditValue(row, editor->text().toDouble());
    });
    return editor;
}

QWidget *FkCollectorWindow::createPlotButton() {
    QPushButton* plotButton = new QPushButton("Curve Config");
    FkCollectorHelper::setButtonStyle(plotButton);
    connect(plotButton, &QPushButton::clicked, this, &FkCollectorWindow::openPlotWindow);
    return plotButton;
}

QComboBox *FkCollectorWindow::createChannelComboBox() {
    QComboBox *comboBox = new QComboBox(ui->tableSignal);
    for (int i = 0; i <= FKCOLLECTOR_MODULE_CHANNEL_SIZE; ++i) {
        comboBox->addItem(QString("CH%1").arg(i+1));
    }

    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this, comboBox](int) { updateSignalComboBoxValue(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_CAHNNEL, comboBox); });

    return comboBox;
}

QComboBox* FkCollectorWindow::createMeasureRangeComboBox() {
    QComboBox* comboBox = new QComboBox(ui->tableSignal);

    comboBox->addItem("0~60V", 60.0);
    comboBox->addItem("0~30V", 30.0);
    comboBox->addItem("0~15V", 15.0);
    comboBox->addItem("0~7.5V", 7.5);
    comboBox->addItem("0~3.75V", 3.75);
    comboBox->addItem("0~1.88V", 1.88);
    comboBox->addItem("0~0.94V", 0.94);
    comboBox->addItem("0~0.47V", 0.47);

    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this, comboBox](int) { updateSignalComboBoxValue(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_MEASURE_RANGE, comboBox); });

    return comboBox;
}

QComboBox *FkCollectorWindow::createOutVoltageComboBox() {
    QComboBox *comboBox = new QComboBox(ui->tableSignal);
    comboBox->addItems(QStringList{"NC","5V","12V"});
    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this, comboBox](int) { updateSignalComboBoxValue(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_OUT_VOLTAGE, comboBox); });
    return comboBox;
}

void FkCollectorWindow::updateLineEditValue(int row, const QString& value) {
    if (QLineEdit *edit = qobject_cast<QLineEdit*>(ui->tableSignal->cellWidget(row, 1))) {
        edit->setText(value);
    }
}

void FkCollectorWindow::updateSignalEditValue(int row, const QVariant &value) {
    OpenSource::DBCSignal *currentSignal = getSignalInfo();
    if (!currentSignal) return;

    switch (row) {
        case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_UNIT:
            currentSignal->unit = value.toString();
            break;
        case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_MIN:
            currentSignal->min = value.toDouble();
            break;
        case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_MAX:
            currentSignal->max = value.toDouble();
            break;
        case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_FACTOR:
            currentSignal->factor = value.toDouble();
            break;
        case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_OFFSET:
            currentSignal->offset = value.toDouble();
            break;
    }
}

void FkCollectorWindow::updateSignalComboBoxValue(int row, QComboBox* comboBox) {
    OpenSource::DBCSignal *currentSignal = getSignalInfo();
    if (!comboBox || !currentSignal) return;

    switch (row) {
        case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_CAHNNEL:
            currentSignal->channel = static_cast<uint8_t>(comboBox->currentIndex());
            break;
        case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_MEASURE_RANGE:
            currentSignal->measureRange = static_cast<uint8_t>(comboBox->currentIndex());
//            if (getModuleType() == FKCOLLECTOR_VOLTAGE) {
//                currentSignal->factor = comboBox->currentData().toDouble() / 32767.0; // 对于16位有符号整数，范围是-32768~32767
//                updateLineEditValue(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_FACTOR, QString::number(currentSignal->factor));
//            }
            break;
        case FKCOLLECTOR_SIGNAL_TABLE_COLUMN_OUT_VOLTAGE:
            currentSignal->outVoltage = static_cast<uint8_t>(comboBox->currentIndex());
            break;
    }
}

void FkCollectorWindow::updateTableSignalValue() {
    OpenSource::DBCSignal *currentSignal = getSignalInfo();
    if (!currentSignal) return;

    QTableWidget *table = ui->tableSignal;

    auto updateComboBox = [table](int row, int value) {
        if (QComboBox *comboBox = qobject_cast<QComboBox*>(table->cellWidget(row, 1))) {
            comboBox->setCurrentIndex(value);
        }
    };

    updateLineEditValue(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_UNIT, currentSignal->unit);
    updateLineEditValue(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_MIN, QString::number(currentSignal->min));
    updateLineEditValue(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_MAX, QString::number(currentSignal->max));
    updateLineEditValue(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_FACTOR, QString::number(currentSignal->factor));
    updateLineEditValue(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_OFFSET, QString::number(currentSignal->offset));

    updateComboBox(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_CAHNNEL, currentSignal->channel);
    updateComboBox(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_MEASURE_RANGE, currentSignal->measureRange);
    updateComboBox(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_OUT_VOLTAGE, currentSignal->outVoltage);
}

void FkCollectorWindow::openPlotWindow() {
    OpenSource::DBCSignal *currentSignal = getSignalInfo();
    if (!currentSignal) return;

    FkCollectorPressoreWindow plotWindow(currentSignal, getModuleType(), this);
    if (plotWindow.exec() == QDialog::Accepted) {
        updateTableSignalValue();
    }
}

void FkCollectorWindow::updateTableSignalEnable() {
    auto moduleType = getModuleType();
    bool isEnable = (moduleType == FKCOLLECTOR_VOLTAGE || moduleType == FKCOLLECTOR_FREQUENCY);

    QPushButton *plotButton = qobject_cast<QPushButton*>(ui->tableSignal->cellWidget(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_PLOT_BUTTON, 1));
    if (!plotButton) return;
    plotButton->setEnabled(isEnable);

    auto enbleComboBox = [this, isEnable](int row) {
        if (QComboBox *comboBox = qobject_cast<QComboBox*>(ui->tableSignal->cellWidget(row, 1))) {
            comboBox->setEnabled(isEnable);
        }
    };
    enbleComboBox(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_CAHNNEL);
    enbleComboBox(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_MEASURE_RANGE);
    enbleComboBox(FKCOLLECTOR_SIGNAL_TABLE_COLUMN_OUT_VOLTAGE);
}

void FkCollectorWindow::setModuleID(int id) {
    currentModuleID = id;
    currentMessageIndex = INVALID_DEFAULT_VALUE;
    ui->groupConfig->setEnabled(id != INVALID_DEFAULT_VALUE);
}

bool FkCollectorWindow::hasModuleID(int id) {
    if (id != INVALID_DEFAULT_VALUE)
        return (moduleInfos.find(id) != moduleInfos.end());
    return (moduleInfos.find(currentModuleID) != moduleInfos.end());
}

FkCollectorModuleType FkCollectorWindow::getModuleType(int id) {
    if (FkCollectorModuleInfo* moduleInfo = getModuleInfo(id)) {
        return moduleInfo->type;
    }
    return FKCOLLECTOR_TEMPERATURE;
}

FkCollectorModuleInfo* FkCollectorWindow::getModuleInfo(int id) {
    if (id == INVALID_DEFAULT_VALUE) {
        id = currentModuleID;
    }

    if (moduleInfos.find(id) == moduleInfos.end())
        return nullptr;

    return &moduleInfos[id];
}

OpenSource::DBCMessage *FkCollectorWindow::getMessageInfo(int index) {
    if (moduleInfos.find(currentModuleID) == moduleInfos.end())
        return nullptr;

    if (index == INVALID_DEFAULT_VALUE) {
        index = currentMessageIndex;
    }

    if (index < 0 || index >= static_cast<int>(moduleInfos[currentModuleID].messages.size()))
        return nullptr;

    return &moduleInfos[currentModuleID].messages.front();
}

OpenSource::DBCSignal* FkCollectorWindow::getSignalInfo()
{
    QTableWidgetItem *item = ui->tableSignalList->currentItem();
    if (!item)
        return nullptr;

    return reinterpret_cast<OpenSource::DBCSignal*>(item->data(Qt::UserRole).value<quintptr>());
    //return item->data(Qt::UserRole).value<OpenSource::DBCSignal*>();
}

void FkCollectorWindow::updateTableConfig() {
    const FkCollectorModuleInfo *moduleInfo = getModuleInfo();
    if (!moduleInfo) return;

    QSpinBox *canIDSpinBox = qobject_cast<QSpinBox*>(ui->tableConfig->cellWidget(0, FKCOLLECTOR_CONFIG_TABLE_COLUMN_CANID));
    QSpinBox *frequencySpinBox = qobject_cast<QSpinBox*>(ui->tableConfig->cellWidget(0, FKCOLLECTOR_CONFIG_TABLE_COLUMN_PERIOD));

    if (canIDSpinBox) {
        OpenSource::DBCMessage *message = getMessageInfo();
        if (message) {
            QSignalBlocker blocker(canIDSpinBox);
            canIDSpinBox->setValue(message->id);
            canIDSpinBox->setToolTip("0x"+QString::number(message->id, 16));
        }
    }
    if (frequencySpinBox) {
        QSignalBlocker blocker(frequencySpinBox);
        frequencySpinBox->setValue(moduleInfo->frequency);
    }
}

void FkCollectorWindow::onDevicesCleared()
{
    FkCollectorHelper::initModuleIcon(ui->treeModule);
}

void FkCollectorWindow::onUpdateModuleIcon(int moduleId, bool isBound) {
    FkCollectorHelper::updateModuleIcon(ui->treeModule, moduleId, isBound);
}

void FkCollectorWindow::onTableConfigItemChanged(int value) {
    FkCollectorModuleInfo *moduleInfo = getModuleInfo();
    if (!moduleInfo) return;

    QSpinBox* senderSpinBox = qobject_cast<QSpinBox*>(sender());
    if (!senderSpinBox) {
        return;
    }

    int col = ui->tableConfig->indexAt(senderSpinBox->pos()).column();
    switch (col) {
        case FKCOLLECTOR_CONFIG_TABLE_COLUMN_CANID: // CAN ID
            if (!moduleInfo->messages.empty() && (moduleInfo->messages.begin()->id != static_cast<uint32_t>(value))) {
                bool isDuplicate = false;
                for (const auto& pair : moduleInfos) {
                    const FkCollectorModuleInfo& info = pair.second;
                    for (const OpenSource::DBCMessage& msg : info.messages) {
                        if (msg.id == static_cast<uint32_t>(value)) {
                            isDuplicate = true;
                            break;
                        }
                    }
                    if (isDuplicate) break;
                }

                if (isDuplicate) {
                    QMessageBox::warning(this, "Duplicate CAN ID",
                        "The entered CAN ID already exists. Please enter a unique ID.");

                    QSignalBlocker blocker(senderSpinBox);  // 阻塞信号以防止递归调用
                    senderSpinBox->setValue(moduleInfo->messages.begin()->id);
                } else {
                    for (OpenSource::DBCMessage &msg : moduleInfo->messages) {
                        msg.id = value;
                        ++value;
                    }
                }
            }
            break;
        case FKCOLLECTOR_CONFIG_TABLE_COLUMN_PERIOD: // period
            moduleInfo->frequency = value;
            break;
        default:
            break;
    }
}

void FkCollectorWindow::showModuleConfigDialog()
{
    FkCollectorConfigDialog dialog(this);
    dialog.exec();
}

void FkCollectorWindow::on_buttonAddModule_clicked()
{
    QTreeWidgetItem* currentItem = ui->treeModule->currentItem();
    if (currentItem && !currentItem->parent()) {
        FkCollectorHelper::addModuleInfoToTree(ui->treeModule, moduleInfos, signalNames);
    } else {
        QMessageBox::warning(this, "Add Module", "Please select a module type to add a new module instance.");
    }
}

void FkCollectorWindow::on_buttonAddModules_clicked()
{
    auto autoCreator = new FkCollectorAuto(deviceInfos, moduleInfos, signalNames, this);
    autoCreator->autoCreateModules(ui->treeModule);
}

void FkCollectorWindow::on_buttonDeleteModule_clicked()
{
    if (FkCollectorHelper::isModuleItem(ui->treeModule->currentItem())) {
        if (QMessageBox::question(this, "Delete Module", "Are you sure you want to delete this module?") == QMessageBox::Yes) {
            int moduleId = ui->treeModule->currentItem()->data(0, Qt::UserRole).toInt();

            if (hasModuleID(moduleId)) {
                FkCollectorModuleInfo &moduleInfo = moduleInfos[moduleId]; // 找到对应的moduleInfo，这里需要根据moduleId从moduleInfos中获取
                for (auto &message : moduleInfo.messages) {
                    for (auto &signal : message.signalList) {
                        signalNames.erase(signal.newName);
                    }
                }

                moduleInfos.erase(moduleId);
            }

            FkCollectorHelper::resetDeviceModuleID(moduleId, deviceInfos);

            delete ui->treeModule->currentItem();
            currentModuleID = INVALID_DEFAULT_VALUE;
            currentMessageIndex = INVALID_DEFAULT_VALUE;
        }
    } else {
        QMessageBox::warning(this, "Delete Module", "Please select a module to delete.");
    }
}

void FkCollectorWindow::on_treeModule_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem */*previous*/)
{
    int moduleId = INVALID_DEFAULT_VALUE;
    if (FkCollectorHelper::isModuleItem(current)) {
        moduleId = current->data(0, Qt::UserRole).toInt();
    }

    setModuleID(moduleId);

    if (hasModuleID(moduleId)) {
        if (!moduleInfos[moduleId].messages.empty()) {
            currentMessageIndex = 0;
        }
        updateTableConfig();
        FkCollectorHelper::updateTableSignalList(ui->tableSignalList, moduleInfos[moduleId].messages);
        return;
    }
}

void FkCollectorWindow::on_treeModule_itemChanged(QTreeWidgetItem *item, int column)
{
    if (FkCollectorHelper::isModuleItem(item) && column == 0) {
        FkCollectorModuleInfo *moduleInfo = getModuleInfo();
        if (moduleInfo && moduleInfo->name != item->text(column)) {
            moduleInfo->name = item->text(column);
            FkCollectorHelper::updateModuleMessageName(*moduleInfo);
        }
    }
}

void FkCollectorWindow::onTableSignalListCurrentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    if (previous && current) {
        if (current->row() == previous->row())
            return;
    }

    bool isEnable = (current != nullptr);
    ui->tableSignal->setEnabled(isEnable);

    if (isEnable) {
        updateTableSignalValue();
        updateTableSignalEnable();
    }
}

void FkCollectorWindow::onTableSignalListItemChanged(QTableWidgetItem *item)
{
    if (!item || item->column() != 1)  return;

    OpenSource::DBCSignal *currentSignal = getSignalInfo();
    if (!currentSignal) return;

    QString newName = item->text();
    // 检查信号名称是否以字母或下划线开头
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QRegularExpression validStartChar("^[A-Za-z_]");
    QRegularExpressionMatch match = validStartChar.match(newName.left(1));
    if (!match.hasMatch()) {
#else
     QRegExp validStartChar("^[A-Za-z_]");
    if (!validStartChar.exactMatch(newName.left(1))) {
#endif
        item->setText(currentSignal->newName);
        QMessageBox::warning(this, tr("Invalid Signal Name"),
                             tr("The signal name must start with a letter or underscore."));
        return;
    }

    if (signalNames.find(newName) != signalNames.end()) {
        item->setText(currentSignal->newName);
        QMessageBox::warning(this, tr("Duplicate Signal Name"),
                             tr("The signal name '%1' already exists. Please choose a unique name.").arg(newName));
    } else {
        signalNames.erase(currentSignal->newName);
        signalNames.insert(newName);
        currentSignal->newName = newName;
    }
}

void FkCollectorWindow::on_buttonExportDBC_clicked()
{
    FkCollectorHelper::exportDbcFile(this, moduleInfos);
}

void FkCollectorWindow::on_buttonMergeDBC_clicked()
{
    FkCollectorHelper::mergeDbcFile(this, moduleInfos);
}

void FkCollectorWindow::on_buttonBindModule_clicked()
{
    FkCollectorModuleType moduleType;
    if (!FkCollectorUtils::validateModuleType(ui->treeModule->currentItem(), moduleInfos, moduleType)) return;

    if (!FkCollectorDevice::getInstance().hasDevice(deviceInfos)) return;

    if (!FkCollectorUtils::validateDevicesForType(deviceInfos, moduleType)) return;

    FkCollectorBind bindDialog(deviceInfos, moduleInfos, this);
    bindDialog.setCurrentModuleType(moduleType);

    // 连接信号以更新树节点图标
    connect(&bindDialog, &FkCollectorBind::sigUpdateModuleIcon, this, &FkCollectorWindow::onUpdateModuleIcon);

    bindDialog.exec();
}

void FkCollectorWindow::on_buttonBindModules_clicked()
{
    if (!FkCollectorUtils::validateModuleInfos(moduleInfos)) return;

    if (!FkCollectorDevice::getInstance().hasDevice(deviceInfos)) return;

    auto autoCreator = new FkCollectorAuto(deviceInfos, moduleInfos, signalNames, this);
    connect(autoCreator, &FkCollectorAuto::sigUpdateModuleIcon, this, &FkCollectorWindow::onUpdateModuleIcon);

    // 绑定成功后，询问是否进入工作模式
    if (autoCreator->autoBindModules()) {
        on_actionSet_Work_triggered();
    }
}

void FkCollectorWindow::on_actionSet_Work_triggered()
{
    if (deviceInfos.empty()) {
        QMessageBox::warning(this, "Warning", "Please search the device first.");
        return;
    }
    FkCollectorDevice::getInstance().setWorkMode();
}

void FkCollectorWindow::on_actionImportConfig_triggered()
{
    FkCollectorHelper::loadModuleInfoFromJson(ui->treeModule, moduleInfos);
}

void FkCollectorWindow::on_actionExportConfig_triggered()
{
    FkCollectorHelper::saveModuleInfoToJson(moduleInfos);
}

void FkCollectorWindow::on_actionAssignID_triggered()
{
    uint32_t currentID = FkCollectorConfig::getInstance().getAssignID();  // Starting ID

    // Create a vector of pairs to sort modules by type
    std::vector<std::pair<int, FkCollectorModuleInfo*>> sortedModules;
    for (auto& pair : moduleInfos) {
        sortedModules.push_back({pair.first, &pair.second});
    }

    // Sort modules by type to ensure consistent ordering
    std::sort(sortedModules.begin(), sortedModules.end(),
              [](const auto& a, const auto& b) {
                  return a.second->type < b.second->type;
              });

    // Assign new IDs
    for (auto& pair : sortedModules) {
        FkCollectorModuleInfo& module = *pair.second;
        // Assign the new ID
        for(auto &message : module.messages) {
            message.id = currentID;
            ++currentID;
        }
    }
}

void FkCollectorWindow::on_actionSearchModule_triggered()
{
    deviceInfos = FkCollectorDevice::getInstance().getDevices();
    if (deviceInfos.empty()) {
        QMessageBox::critical(this, "Error", "No device found.");
    } else {
        FkCollectorDevice::getInstance().showDeviceList();
    }
}

void FkCollectorWindow::on_actionOpen_triggered()
{
    ui->actionOpen->setEnabled(false);
    if (ui->actionOpen->isCheckable()) {
        if (QMessageBox::question(this, tr("Warnning"), tr("Are you sure to close all devices and corresponding channels?"),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            FkVciManager::getInstance().closeDevices();
            ui->actionOpen->setCheckable(false);
        }
    }
    else {
        FkVciConfigDialog configDialog(this);
        configDialog.setNoneLIN();
        configDialog.exec();
    }
    ui->actionOpen->setEnabled(true);
}

void FkCollectorWindow::on_actionVersion_triggered()
{
    FkVciVersionDialog dialog;
    dialog.exec();
}

void FkCollectorWindow::on_actionAbout_triggered()
{

}

void FkCollectorWindow::on_actionConfig_Log_triggered()
{
    DebugLogDialog logSettings;
    logSettings.exec();
}

void FkCollectorWindow::on_actionConfigModule_triggered()
{
    showModuleConfigDialog();
}
