// fkmoduleselectdialog.h
#ifndef FKMODULESELECTDIALOG_H
#define FKMODULESELECTDIALOG_H

#include <QDialog>
#include <QMap>

class QListWidget;
class QPushButton;

class FkModuleSelectDialog : public QDialog {
    Q_OBJECT

public:
    explicit FkModuleSelectDialog(const QMap<int, QString>& availableModules, QWidget* parent = nullptr);

    // 获取选中的模块ID
    int getSelectedModuleId() const;

private:
    void setupUi();

    QListWidget* moduleList_{nullptr};
    QPushButton* selectButton_{nullptr};
    QPushButton* cancelButton_{nullptr};

    QMap<int, QString> modules_;
    int selectedModuleId_{-1};
};

#endif // FKMODULESELECTDIALOG_H
