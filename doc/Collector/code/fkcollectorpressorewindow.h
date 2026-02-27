// fkcollectorpressorewindow.h
#ifndef FKCOLLECTOR_PRESSORE_WINDOW_H
#define FKCOLLECTOR_PRESSORE_WINDOW_H

#include <QDialog>
#include <QChartView>
#include <QLineSeries>

#include "fkcollector/fkcollectordef.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QT_CHARTS_USE_NAMESPACE
#endif

class FkCollectorPressoreWindow : public QDialog {
    Q_OBJECT

public:
    explicit FkCollectorPressoreWindow(OpenSource::DBCSignal *signal, FkCollectorModuleType type = FKCOLLECTOR_VOLTAGE, QWidget *parent = nullptr);
    void setPoints(const QPointF& start, const QPointF& end);

private:
    void setupUi();
    void updatePlot();
    void calculateParameters();

    QChartView *m_chartView;
    QPointF m_startPoint;
    QPointF m_endPoint;

    FkCollectorModuleType m_type;
    OpenSource::DBCSignal *m_signal;
    double m_baseFactor{1.0};
};

#endif // FKCOLLECTOR_PRESSORE_WINDOW_H
