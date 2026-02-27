// fkmoduleselectdialog.cpp
#include "fkcollector/fkmoduleselectdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>

FkModuleSelectDialog::FkModuleSelectDialog(const QMap<int, QString>& availableModules, QWidget* parent)
    : QDialog(parent)
    , modules_(availableModules)
{
    setupUi();
    setWindowIcon(QIcon(":/resource/module.png"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Select Module"));

    // 添加模块到列表
    for (auto it = modules_.constBegin(); it != modules_.constEnd(); ++it) {
        QListWidgetItem* item = new QListWidgetItem(it.value());
        item->setData(Qt::UserRole, it.key());
        moduleList_->addItem(item);
    }

    // 连接信号和槽
    connect(selectButton_, &QPushButton::clicked, this, [this]() {
        QListWidgetItem* currentItem = moduleList_->currentItem();
        if (currentItem) {
            selectedModuleId_ = currentItem->data(Qt::UserRole).toInt();
            accept();
        }
    });

    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);

    connect(moduleList_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item) {
            selectedModuleId_ = item->data(Qt::UserRole).toInt();
            accept();
        }
    });
}

void FkModuleSelectDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* label = new QLabel(tr("Select a module to bind:"));
    mainLayout->addWidget(label);

    moduleList_ = new QListWidget(this);
    moduleList_->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(moduleList_);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    selectButton_ = new QPushButton(tr("Select"), this);
    selectButton_->setDefault(true);
    cancelButton_ = new QPushButton(tr("Cancel"), this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(selectButton_);
    buttonLayout->addWidget(cancelButton_);

    mainLayout->addLayout(buttonLayout);

    setMinimumSize(300, 250);
}

int FkModuleSelectDialog::getSelectedModuleId() const
{
    return selectedModuleId_;
}
