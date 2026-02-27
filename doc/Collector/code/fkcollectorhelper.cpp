#include "fkcollector/fkcollectorhelper.h"
#include "fkcollector/fkcollectorconfig.h"
#include "can/dbcparser.h"
#include "can/dbcwriter.h"
#include "can/dbcmerge.h"
#include "can/dbchelper.h"
#include "common/loggermacros.h"
#include "common/commonutils.h"
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>

#include <QLineEdit>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QComboBox>

const QString ICON_BOUND = ":/resource/light_on.png";
const QString ICON_UNBOUND = ":/resource/light_off.png";

FkCollectorHelper::FkCollectorHelper() {}

QString FkCollectorHelper::getModuleTypeString(FkCollectorModuleType type)
{
    switch (type) {
        case FKCOLLECTOR_TEMPERATURE: return "Temperature";
        case FKCOLLECTOR_VOLTAGE: return "Voltage";
        case FKCOLLECTOR_CURRENT: return "Current";
        case FKCOLLECTOR_FREQUENCY: return "Frequency";
    }
    return QString();
}

void FkCollectorHelper::addModuleTypeItem(QTreeWidget *treeWidget, FkCollectorModuleType type, const QString &iconPath)
{
    QTreeWidgetItem *moduleItem = new QTreeWidgetItem(treeWidget);
    moduleItem->setText(0, getModuleTypeString(type));
    moduleItem->setData(0, Qt::UserRole, type);
    moduleItem->setIcon(0, QIcon(iconPath));
}

void FkCollectorHelper::initTreeModule(QTreeWidget *treeWidget) {
    treeWidget->clear();
    treeWidget->setHeaderLabel("Modules");
    treeWidget->setRootIsDecorated(false);

    addModuleTypeItem(treeWidget, FKCOLLECTOR_TEMPERATURE, ":/resource/fkcollector/temperature.png");
    addModuleTypeItem(treeWidget, FKCOLLECTOR_VOLTAGE, ":/resource/fkcollector/voltage.png");
    addModuleTypeItem(treeWidget, FKCOLLECTOR_CURRENT, ":/resource/fkcollector/current.png");
    addModuleTypeItem(treeWidget, FKCOLLECTOR_FREQUENCY, ":/resource/fkcollector/frequency.png");

    treeWidget->expandAll();
}

FkCollectorModuleInfo FkCollectorHelper::addModuleInfo(FkCollectorModuleType type) {
    FkCollectorModuleInfo moduleInfo;
    moduleInfo.type = type;

    auto &config = FkCollectorConfig::getInstance();
    moduleInfo.frequency = config.getModuleFrequency(type);

    QString dbcFilePath;
    QString dbcFileEncoding;
    if (!config.getModuleFile(type, dbcFilePath, dbcFileEncoding)) {
        return moduleInfo;
    }

    OpenSource::DbcParser dbcParser;
    if (!dbcParser.loadFile(dbcFilePath, dbcFileEncoding)) {
        return moduleInfo;
    }

    moduleInfo.messages = dbcParser.getDbcData().messages;
    if (moduleInfo.messages.empty()) {
        return moduleInfo;
    }

    for (auto &message : moduleInfo.messages) {
        message.id = config.getModuleMessageId(type);
    }
    return moduleInfo;
}

void FkCollectorHelper::updateModuleMessageName(FkCollectorModuleInfo &moduleInfo) {
    if (moduleInfo.messages.size() == 1) {
        moduleInfo.messages.front().name = moduleInfo.name;
    }
    else {
        int index = 0;
        for (auto &message : moduleInfo.messages) {
            ++index;
            message.name = QString("%1_C%2").arg(moduleInfo.name).arg(index);
        }
    }
}

void FkCollectorHelper::updateModuleSignalInfo(FkCollectorModuleInfo &moduleInfo, std::unordered_set<QString> &signalNames) {
    for (auto &message : moduleInfo.messages) {
        int j=0;
        for (auto &signal : message.signalList) {
            ++j;
            QString newName = signal.name;
            if (signalNames.find(newName) != signalNames.end()) {
                newName = QString("%1_%2").arg(message.name).arg(signal.name);
            }

            if (signalNames.find(newName) != signalNames.end()) {
                int suffix = 1;
                do {
                    newName = QString("%1_Signal%2").arg(message.name).arg(suffix);
                    suffix++;
                } while (signalNames.find(newName) != signalNames.end());
            }

            auto result = signalNames.insert(newName);
            if (!result.second) {
                LOG_WARN("Duplicate signal name found: {}, Message ID: {}", newName.toStdString().c_str(), message.id);
            }

            signal.newName = newName;
        }
    }
}

void FkCollectorHelper::addModuleInfoItem(QTreeWidgetItem *parent, const QString &name, int id) {
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, name);
    item->setIcon(0, QIcon(ICON_UNBOUND));
    item->setData(0, Qt::UserRole, id);
    //item->setFlags(item->flags() | Qt::ItemIsEditable);
}

bool FkCollectorHelper::addModuleInfoToTree(QTreeWidget *treeModule, std::map<int, FkCollectorModuleInfo> &moduleInfos
                                            , std::unordered_set<QString> &signalNames) {
    QTreeWidgetItem *currentItem = treeModule->currentItem();
    if (!currentItem) return false;

    if (currentItem->childCount() >= FKCOLLECTOR_MODULE_CHANNEL_SIZE) {
        QMessageBox::critical(nullptr, "Error", QString("Unable to add a new module, the maximum number of modules has exceeded %1.").arg(FKCOLLECTOR_MODULE_CHANNEL_SIZE));
        return false;
    }

    FkCollectorModuleType type = static_cast<FkCollectorModuleType>(currentItem->data(0, Qt::UserRole).toInt());
    QString baseName = getModuleTypeString(type);

    QString newModuleName;
    for (int i = currentItem->childCount() + 1; i > 0; --i) {
        newModuleName = QString("%1_%2").arg(baseName).arg(i);
        bool exists = false;

        for (int j = 0; j < currentItem->childCount(); ++j) {
            if (currentItem->child(j)->text(0) == newModuleName) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            auto info = addModuleInfo(type);
            auto id = static_cast<int>(type)*100+i;
            moduleInfos[id] = info;
            moduleInfos[id].name = newModuleName;
            updateModuleMessageName(moduleInfos[id]);
            updateModuleSignalInfo(moduleInfos[id], signalNames);
            addModuleInfoItem(currentItem, newModuleName, id);
            treeModule->expandItem(currentItem);
            return true;
        }
    }

    return false;
}

bool FkCollectorHelper::isModuleItem(QTreeWidgetItem *item) {
    return item && item->parent() && (0 == item->childCount());
}

void FkCollectorHelper::initModuleIcon(QTreeWidget *treeModule)
{
    // 遍历所有模块节点，将图标设置为 light_off.png
    QTreeWidgetItemIterator it(treeModule);
    while (*it) {
        if (FkCollectorHelper::isModuleItem(*it)) {
            (*it)->setIcon(0, QIcon(ICON_UNBOUND));
        }
        ++it;
    }
}

void FkCollectorHelper::updateModuleIcon(QTreeWidget *treeModule, int moduleId, bool isBound)
{
    if (QTreeWidgetItem* item = findModuleItem(treeModule, moduleId)) {
        item->setIcon(0, QIcon(isBound ? ICON_BOUND : ICON_UNBOUND));
    }
}

QTreeWidgetItem* FkCollectorHelper::findModuleItem(QTreeWidget* treeModule, int moduleId)
{
    if (!treeModule) return nullptr;

    QTreeWidgetItemIterator it(treeModule);
    while (*it) {
        if ((*it)->data(0, Qt::UserRole).toInt() == moduleId) {
            return *it;
        }
        ++it;
    }
    return nullptr;
}

void FkCollectorHelper::updateTableSignalList(QTableWidget *tableSignalList, const std::vector<OpenSource::DBCMessage> &messages) {
    if (messages.empty()) return;

    QSignalBlocker blocker(tableSignalList);
    tableSignalList->setRowCount(0);

    for (const auto &message : messages) {
        for (const auto &signal : message.signalList) {
            int row = tableSignalList->rowCount();
            tableSignalList->insertRow(row);

            QTableWidgetItem *channelItem = new QTableWidgetItem(signal.name);
            channelItem->setIcon(QIcon(":/resource/dbc_signal.png"));
            channelItem->setFlags(channelItem->flags() & ~Qt::ItemIsEditable);
            channelItem->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(&signal)));
            tableSignalList->setItem(row, 0, channelItem);

            QTableWidgetItem *signalNameItem = new QTableWidgetItem(signal.newName);
            signalNameItem->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(&signal)));
            //signalNameItem->setData(Qt::UserRole, QVariant::fromValue(const_cast<OpenSource::DBCSignal*>(&signal)));
            tableSignalList->setItem(row, 1, signalNameItem);
        }
    }
}

bool FkCollectorHelper::saveModuleInfoToJson(const std::map<int, FkCollectorModuleInfo>& moduleInfos)
{
    if (moduleInfos.empty()) {
         QMessageBox::critical(nullptr, "Error", "Please add the model before operating.");
         return false;
    }

    QString filePath = QFileDialog::getSaveFileName(nullptr, "Save Module Info", "", "JSON Files (*.json)");
    if (filePath.isEmpty()) {
        return false;
    }

    QJsonObject rootObject;
    QJsonArray modulesArray;

    for (const auto& pair : moduleInfos) {
        QJsonObject moduleObject;
        const FkCollectorModuleInfo& info = pair.second;

        moduleObject["id"] = pair.first;
        moduleObject["name"] = info.name;
        moduleObject["type"] = static_cast<int>(info.type);

        QJsonObject dataObject;
        dataObject["frequency"] = static_cast<int>(info.frequency);
        moduleObject["data"] = dataObject;

        // 使用新接口生成消息字符串列表
        QJsonArray messagesArray;
        for (const auto &message : info.messages) {
            QStringList messageLines = OpenSource::DbcHelper::generateMessagesString({message});
            QJsonArray messageLinesArray;
            for (const QString& line : messageLines) {
                messageLinesArray.append(line);
            }
            messagesArray.append(messageLinesArray);
        }
        moduleObject["messages"] = messagesArray;

        modulesArray.append(moduleObject);
    }

    rootObject["modules"] = modulesArray;

    QJsonDocument jsonDoc(rootObject);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(nullptr, "Error", "Could not open file for writing.");
        return false;
    }

    file.write(jsonDoc.toJson(QJsonDocument::Indented));
    file.close();

    QMessageBox::information(nullptr, "Success", "Module information saved successfully.");
    return true;
}

bool FkCollectorHelper::loadModuleInfoFromJson(QTreeWidget *treeWidget, std::map<int, FkCollectorModuleInfo>& moduleInfos)
{
    QString filePath = QFileDialog::getOpenFileName(nullptr, "Load Module Info", "", "JSON Files (*.json)");
    if (filePath.isEmpty()) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(nullptr, "Error", "Could not open file for reading.");
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull()) {
        QMessageBox::critical(nullptr, "Error", "Invalid JSON format.");
        return false;
    }

    std::map<int, FkCollectorModuleInfo> tempModuleInfos;
    QJsonObject rootObject = jsonDoc.object();
    QJsonArray modulesArray = rootObject["modules"].toArray();

    for (const QJsonValue& moduleValue : modulesArray) {
        QJsonObject moduleObject = moduleValue.toObject();
        int id = moduleObject["id"].toInt();
        FkCollectorModuleInfo info;

        info.name = moduleObject["name"].toString();
        info.type = static_cast<FkCollectorModuleType>(moduleObject["type"].toInt());

        QJsonObject dataObject = moduleObject["data"].toObject();
        info.frequency = dataObject["frequency"].toInt();

        // 使用新接口解析消息字符串列表
        QJsonArray messagesArray = moduleObject["messages"].toArray();
        for (const QJsonValue& messageValue : messagesArray) {
            QJsonArray messageLines = messageValue.toArray();
            QStringList messageLinesString;
            for (const QJsonValue& line : messageLines) {
                messageLinesString.append(line.toString());
            }
            auto messages = OpenSource::DbcHelper::parseMessagesString(messageLinesString);
            info.messages.insert(info.messages.end(), std::make_move_iterator(messages.begin()),
                                 std::make_move_iterator(messages.end()));
        }

        tempModuleInfos[id] = info;
    }

    moduleInfos.swap(tempModuleInfos);

    initTreeModule(treeWidget);

    for (const auto& pair : moduleInfos) {
        const FkCollectorModuleInfo& info = pair.second;
        QTreeWidgetItem* parentItem = findModuleTypeItem(treeWidget, info.type);
        if (parentItem) {
            addModuleInfoItem(parentItem, info.name, pair.first);
        }
    }
    treeWidget->expandAll();

    QMessageBox::information(nullptr, "Success", "Module information loaded successfully.");
    return true;
}

// 辅助函数：查找对应模块类型的树节点
QTreeWidgetItem* FkCollectorHelper::findModuleTypeItem(QTreeWidget* treeWidget, FkCollectorModuleType type)
{
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if (item->data(0, Qt::UserRole).toInt() == static_cast<int>(type)) {
            return item;
        }
    }
    return nullptr;
}

void FkCollectorHelper::setButtonStyle(QPushButton *button) {
    if (button) {
        button->setStyleSheet(
            "QPushButton {"
            "    background-color: #3a6ea5;"
            "    color: white;"
            "    border: none;"
            "    padding: 5px 10px;"
            "    text-align: center;"
            "    font-size: 12px;"
            "    border-radius: 3px;"
            "}"
            "QPushButton:hover {"
            "    background-color: #2980b9;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #2573a7;"
            "}"
        );
    }
}

bool FkCollectorHelper::isModuleBound(int moduleId, const std::map<uint32_t, FkCollectorDeviceInfo>& deviceInfos)
{
    for (const auto& [deviceId, deviceInfo] : deviceInfos) {
        if (deviceInfo.moduleId == moduleId) {
            return true;
        }
    }
    return false;
}

bool FkCollectorHelper::resetDeviceModuleID(int moduleId, std::map<uint32_t, FkCollectorDeviceInfo>& deviceInfos) {
    for (auto& [deviceId, deviceInfo] : deviceInfos) {
        if (deviceInfo.moduleId == moduleId) {
            deviceInfo.moduleId = INVALID_DEFAULT_VALUE;
            return true;
        }
    }

    return false;
}

bool FkCollectorHelper::exportDbcFile(QWidget *parent, const std::map<int, FkCollectorModuleInfo> &moduleInfos)
{
    if (moduleInfos.empty())
        return false;

    // 第二行：新 DBC
    QString fileName = QFileDialog::getSaveFileName(parent, tr("Export DBC File"), "", tr("DBC Files (*.dbc)"));
    if (fileName.isEmpty()) {
        return false;
    }

    const auto& config = FkCollectorConfig::getInstance();
    FkCollectorModuleType firstModuleType = moduleInfos.begin()->second.type;
    QString dbcFilePath;
    QString dbcFileEncoding;
    if (!config.getModuleFile(firstModuleType, dbcFilePath, dbcFileEncoding)) {
        QMessageBox::critical(parent, tr("Error"), QString("Failed to get module DBC file."));
        return false;
    }

    OpenSource::DbcParser dbcParser;
    if (!dbcParser.loadFile(dbcFilePath, dbcFileEncoding)) {
        QMessageBox::critical(parent, tr("Error"), QString("Failed to load DBC file %1.").arg(dbcFilePath));
        return false;
    }

    auto dbcData = dbcParser.getDbcData();

    std::vector<OpenSource::DBCMessage> moduleMessages;
    for (const auto &moduleEntry : moduleInfos) {
        std::vector<OpenSource::DBCMessage> messages = moduleEntry.second.messages;
        for (auto &message : messages) {
            message.id |= OpenSource::DBC_EXTENDED_FRAME_FLAG;
            for (auto &signal : message.signalList) {
                signal.name = signal.newName;
            }
            moduleMessages.push_back(std::move(message));
        }
    }

    QString err;
    if (!OpenSource::DbcHelper::validateMessages(moduleMessages, err) || !err.trimmed().isEmpty()) {
        QMessageBox::critical(parent, tr("Error"), err);
        return false;
    }

    dbcData.messages.swap(moduleMessages);
    //QString busType = FkCollectorConfig::getInstance().getCanType() == FKCOLLECTOR_CANFD?"CAN FD":"CAN";
    //OpenSource::DbcHelper::SetAttributeValue(dbcData, OpenSource::ATTR_BUSTYPE, OpenSource::ATTR_TYPE_STRING, busType);
    OpenSource::DbcWriter dbcWriter(dbcData);
    if (!dbcWriter.saveToFile(fileName, dbcFileEncoding)) {
        QMessageBox::critical(parent, tr("Error"), QString("Failed to export DBC file %1.").arg(fileName));
        return false;
    }

    QMessageBox::information(parent, tr("Information"), QString("DBC file exported successfully."));
    return true;
}

bool FkCollectorHelper::mergeDbcFile(QWidget *parent, const std::map<int, FkCollectorModuleInfo> &moduleInfos)
{
    if (moduleInfos.empty())
        return false;

    QDialog dialog(parent);
    dialog.setWindowTitle(tr("Merge DBC Files"));
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    dialog.setMinimumWidth(400);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // 第一行：合并 DBC
    QHBoxLayout *mergeLayout = new QHBoxLayout();
    QLabel *mergeLabel = new QLabel(tr("Merge DBC File:"), &dialog);
    QLineEdit *mergeFilePathEdit = new QLineEdit(&dialog);
    mergeFilePathEdit->setReadOnly(true);
    QPushButton *mergeBrowseButton = new QPushButton(tr("Browse"), &dialog);

    mergeLayout->addWidget(mergeLabel);
    mergeLayout->addWidget(mergeFilePathEdit);
    mergeLayout->addWidget(mergeBrowseButton);
    layout->addLayout(mergeLayout);

    // 第二行：新 DBC
    QHBoxLayout *saveLayout = new QHBoxLayout();
    QLabel *saveLabel = new QLabel(tr("Save DBC File:"), &dialog);
    QLineEdit *saveFilePathEdit = new QLineEdit(&dialog);
    saveFilePathEdit->setReadOnly(true);
    QPushButton *saveBrowseButton = new QPushButton(tr("Save As"), &dialog);

    saveLayout->addWidget(saveLabel);
    saveLayout->addWidget(saveFilePathEdit);
    saveLayout->addWidget(saveBrowseButton);
    layout->addLayout(saveLayout);

    // 第三行：文件编码和按钮框
    QHBoxLayout *encodingLayout = new QHBoxLayout();
    QLabel *encodingLabel = new QLabel(tr("DBC Encoding:"), &dialog);
    QComboBox *encodingComboBox = new QComboBox(&dialog);
    encodingComboBox->addItems(CommonUtils::getSupportFileEncodings());
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    encodingLayout->addWidget(encodingLabel);
    encodingLayout->addWidget(encodingComboBox);
    encodingLayout->addStretch();
    encodingLayout->addWidget(buttonBox);
    layout->addLayout(encodingLayout);

    // 连接信号和槽
    connect(mergeBrowseButton, &QPushButton::clicked, [&]() {
        QString fileName = QFileDialog::getOpenFileName(&dialog, tr("Merge DBC File"), "", tr("DBC Files (*.dbc)"));
        if (!fileName.isEmpty()) {
            mergeFilePathEdit->setText(fileName);

            // 自动填充新 DBC 文件路径
            QFileInfo fileInfo(fileName);
            QString newFileName = fileInfo.path() + "/merge_" + fileInfo.fileName();
            saveFilePathEdit->setText(newFileName);
        }
    });

    connect(saveBrowseButton, &QPushButton::clicked, [&]() {
        QString fileName = QFileDialog::getSaveFileName(&dialog, tr("Save DBC File"), "", tr("DBC Files (*.dbc)"));
        if (!fileName.isEmpty()) {
            saveFilePathEdit->setText(fileName);
        }
    });

    connect(buttonBox, &QDialogButtonBox::accepted, [&]() {
        QString mergeFileName = mergeFilePathEdit->text();
        QString saveFileName = saveFilePathEdit->text();
        QString selectedEncoding = encodingComboBox->currentText();

        if (mergeFileName.isEmpty() || saveFileName.isEmpty()) {
            QMessageBox::critical(parent, tr("Error"), QString("Please enter the %1 dbc file path.").arg(mergeFileName.isEmpty()?"merge":"save"));
            return;
        }

        OpenSource::DbcParser dbcParser;
        if (!dbcParser.loadFile(mergeFileName, selectedEncoding)) {
            QMessageBox::critical(parent, tr("Errpr"), QString("Failed to load DBC file %1.").arg(mergeFileName));
            return;
        }

        auto allMessages = dbcParser.getDbcData().messages;
        std::vector<OpenSource::DBCMessage> moduleMessages;
        for (const auto &moduleEntry : moduleInfos) {
            std::vector<OpenSource::DBCMessage> messages = moduleEntry.second.messages;
            for (auto &message : messages) {
                message.id |= OpenSource::DBC_EXTENDED_FRAME_FLAG;
                for (auto &signal : message.signalList) {
                    signal.name = signal.newName;
                }
                allMessages.push_back(message);
                moduleMessages.push_back(std::move(message));
            }
        }

        QString err;
        if (!OpenSource::DbcHelper::validateMessages(allMessages, err)) {
            QMessageBox::critical(parent, tr("Error"), err);
            return;
        }

        OpenSource::DbcMerge merge;
        if (!merge.merge(mergeFileName, saveFileName, std::move(moduleMessages), selectedEncoding)) {
            QMessageBox::critical(parent, tr("Error"), QString("Failed to merge DBC file"));
            return;
        }
        QMessageBox::information(parent, tr("Information"), QString("DBC file merge successfully."));
        dialog.accept();
    });

    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    return dialog.exec() == QDialog::Accepted;
}
