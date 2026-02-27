#include <QApplication>
#include <QMessageBox>

#include "../ui/fkcollector/fkcollectorwindow.h"
#include "common/loggermacros.h"
#include "common/commonconfigmanager.h"
#include <QMetaType>

//########### icon:#3a6ea5 ##################
int main(int argc, char *argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    //qRegisterMetaTyp<OpenSourceey::DBCSignal*>OpenSourceey::DBCSignal*");

    QApplication app(argc, argv);

    {
        const auto& config = CommonConfigManager::getInstance();
        std::string logFile(config.getLogFilePath().toStdString());

        LOG_INIT(logFile, static_cast<LogLevel>(config.getLogLevel()), config.getLogSize(), config.getLogFiles());
        try {
            LOG_SET_OUTPUT(config.getLogToConsole());
            LOG_SOFTWARE_INFO(SOFT_NAME, SOFT_VERSION, "figkey-leiwei", "windows");
        } catch (const std::exception &e) {
            QMessageBox::critical(nullptr, "error", e.what());
        }
    }

    FkCollectorWindow* w = nullptr;
    try {
        w = new FkCollectorWindow;
        w->show();
        w->start();

        return app.exec();
    } catch (const std::exception &e) {
        QMessageBox::critical(nullptr, "异常", e.what());
        return -1;
    } catch (...) {
        QMessageBox::critical(nullptr, "未知异常", "程序发生了未知异常并即将退出。");
        return -2;
    }
}
