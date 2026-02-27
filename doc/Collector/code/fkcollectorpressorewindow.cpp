// fkcollectorpressorewindow.cpp
#include "fkcollectorpressorewindow.h"
#include "fkcollector/fkcollectorconfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QValueAxis>
#include <QSpacerItem>
#include <QLabel>

FkCollectorPressoreWindow::FkCollectorPressoreWindow(OpenSource::DBCSignal *signal, FkCollectorModuleType type, QWidget *parent)
    : QDialog(parent)
    , m_chartView(nullptr)
    , m_type(type)
    , m_signal(signal)
    , m_baseFactor(signal->factor)
{
    setWindowIcon(QIcon(":/resource/dbc_signal.png"));
    setWindowTitle(QString("Curve Configuration Plot(%1)").arg(signal->newName));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumSize(600, 400);

    setupUi();
}

void FkCollectorPressoreWindow::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    // Input fields in one row
    QHBoxLayout *inputLayout = new QHBoxLayout();

    QFont labelFont("Arial", 9);
    QFont inputFont("Arial", 9);

    auto createInputPair = [&](const QString &label, const QString &defaultValue) {
        QLabel *lbl = new QLabel(label);
        lbl->setFont(labelFont);
        QLineEdit *edit = new QLineEdit(defaultValue);
        edit->setFont(inputFont);
        edit->setFixedWidth(60);
        edit->setStyleSheet("QLineEdit { border: 1px solid #CCCCCC; border-radius: 3px; padding: 1px; }");
        inputLayout->addWidget(lbl);
        inputLayout->addWidget(edit);
        return edit;
    };

    const auto& [startPoint, endPoint]  = FkCollectorConfig::getInstance().getModulePoints(m_type);
    QLineEdit *startXEdit = createInputPair("Start X:", QString::number(startPoint.x()));
    QLineEdit *startYEdit = createInputPair("Y:", QString::number(startPoint.y()));
    QLineEdit *endXEdit = createInputPair("End X:", QString::number(endPoint.x()));
    QLineEdit *endYEdit = createInputPair("Y:", QString::number(endPoint.y()));

    inputLayout->addStretch();

    mainLayout->addLayout(inputLayout);

    // Chart view
    m_chartView = new QChartView();
    m_chartView->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(m_chartView, 1);

    // OK button
    QPushButton *okButton = new QPushButton("OK");
    okButton->setFixedSize(80, 25);
    okButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border-radius: 3px; }");
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    bottomLayout->addWidget(okButton);

    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);

    // Connect text changed signals to update plot
    auto updatePlotLambda = [=]() {
        bool ok;
        QPointF start(startXEdit->text().toDouble(&ok), startYEdit->text().toDouble(&ok));
        QPointF end(endXEdit->text().toDouble(&ok), endYEdit->text().toDouble(&ok));
        if (ok) {
            setPoints(start, end);
        }
    };

    connect(startXEdit, &QLineEdit::textChanged, this, updatePlotLambda);
    connect(startYEdit, &QLineEdit::textChanged, this, updatePlotLambda);
    connect(endXEdit, &QLineEdit::textChanged, this, updatePlotLambda);
    connect(endYEdit, &QLineEdit::textChanged, this, updatePlotLambda);

    // Initial plot update
    updatePlotLambda();
}


void FkCollectorPressoreWindow::setPoints(const QPointF& start, const QPointF& end) {
    m_startPoint = start;
    m_endPoint = end;
    updatePlot();
    calculateParameters();
}

void FkCollectorPressoreWindow::updatePlot() {
    QLineSeries *series = new QLineSeries();
    series->append(m_startPoint);
    series->append(m_endPoint);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->createDefaultAxes();

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
    chart->axisX()->setTitleText("Voltage (mV)");
    chart->axisY()->setTitleText("Pressure (kPa)");
#else
    auto xAxes = chart->axes(Qt::Horizontal);
    auto yAxes = chart->axes(Qt::Vertical);
    if (!xAxes.isEmpty()) {
        if (QValueAxis *valueAxisX = qobject_cast<QValueAxis*>(xAxes.first())) {
            valueAxisX->setTitleText(m_type == FKCOLLECTOR_FREQUENCY ? "Frequency(Hz)" : "Voltage (mV)");
        }
    }
    if (!yAxes.isEmpty()) {
        if (QValueAxis *valueAxisY = qobject_cast<QValueAxis*>(yAxes.first())) {
            valueAxisY->setTitleText(m_signal->unit);
        }
    }
#endif

    m_chartView->setChart(chart);
}


void FkCollectorPressoreWindow::calculateParameters() {
    m_signal->min = qMin(m_startPoint.y(), m_endPoint.y());
    m_signal->max = qMax(m_startPoint.y(), m_endPoint.y());

    // 计算 factor 和 offset
    // x = y * factor + offset
//    if (m_type == FKCOLLECTOR_VOLTAGE) {
//        m_signal->factor = m_baseFactor * ((m_endPoint.x() - m_startPoint.x()) / (m_endPoint.y() - m_startPoint.y())) ;
//    }
//    else {
        m_signal->factor = (m_endPoint.x() - m_startPoint.x()) / (m_endPoint.y() - m_startPoint.y()) ;
//    }
    m_signal->offset = m_startPoint.x() - m_startPoint.y() * m_signal->factor;
}
