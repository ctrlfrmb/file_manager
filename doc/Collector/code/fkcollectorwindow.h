#ifndef FKCOLLECTOR_WINDOW_H
#define FKCOLLECTOR_WINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QTreeWidgetItem>
#include <QTableWidgetItem>
#include <map>
#include <QComboBox>
#include <QPushButton>

#include "fkcollector/fkcollectordef.h"
#include "fkvci/fkvcidef.h"

namespace Ui {
class FkCollectorWindow;
}

class FkCollectorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FkCollectorWindow(QWidget *parent = nullptr);
    ~FkCollectorWindow();

    void start();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onDevicesCleared();

    void onUpdateModuleIcon(int moduleId, bool isBound);

    void onTableConfigItemChanged(int value);

    void openPlotWindow();

    void on_buttonAddModule_clicked();

    void on_buttonDeleteModule_clicked();

    void on_treeModule_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_treeModule_itemChanged(QTreeWidgetItem *item, int column);

    void onTableSignalListCurrentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);

    void onTableSignalListItemChanged(QTableWidgetItem *item);

    void on_buttonExportDBC_clicked();

    void on_buttonMergeDBC_clicked();

    void on_buttonBindModule_clicked();

    void on_actionSet_Work_triggered();

    void on_actionImportConfig_triggered();

    void on_actionExportConfig_triggered();

    void on_actionAssignID_triggered();

    void on_actionSearchModule_triggered();

    void on_actionOpen_triggered();

    void on_actionVersion_triggered();

    void on_actionAbout_triggered();

    void on_actionConfig_Log_triggered();

    void on_actionConfigModule_triggered();

    void on_buttonAddModules_clicked();

    void on_buttonBindModules_clicked();

private:
    Ui::FkCollectorWindow *ui;
    std::atomic_bool isDeviceOpened_{false};

    std::map<int, FkCollectorModuleInfo> moduleInfos;
    std::unordered_set<QString> signalNames;
    int currentModuleID{INVALID_DEFAULT_VALUE};
    int currentMessageIndex{INVALID_DEFAULT_VALUE};

    std::map<uint32_t, FkCollectorDeviceInfo> deviceInfos;

    void initWindow();
    void exitWindow();

    void initTableConfig();
    void initTableSignalList();
    void initTableSignal();

    QLineEdit* createStringEditor(int row);
    QLineEdit* createDoubleEditor(int row);
    QWidget* createPlotButton();
    QComboBox* createChannelComboBox();
    QComboBox* createMeasureRangeComboBox();
    QComboBox* createOutVoltageComboBox();

    void updateLineEditValue(int row, const QString& value);
    void updateSignalEditValue(int row, const QVariant& value);
    void updateSignalComboBoxValue(int row, QComboBox* comboBox);
    void updateTableSignalValue();
    void updateTableSignalEnable();

    void setModuleID(int id=INVALID_DEFAULT_VALUE);
    bool hasModuleID(int id=INVALID_DEFAULT_VALUE);
    FkCollectorModuleType getModuleType(int id=INVALID_DEFAULT_VALUE);
    FkCollectorModuleInfo* getModuleInfo(int id=INVALID_DEFAULT_VALUE);
    OpenSource::DBCMessage* getMessageInfo(int index=INVALID_DEFAULT_VALUE);

    OpenSource::DBCSignal* getSignalInfo();

    void updateTableConfig();

    void processVciStatusChanged(int deviceIndex, uint8_t cmd);
    void showModuleConfigDialog();
};

#endif // FKCOLLECTOR_WINDOW_H
