#ifndef LINE_CHART_H
#define LINE_CHART_H    

#include <QGridLayout>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLegendMarker>

#include <QtConcurrent/QtConcurrentRun>
#include <QEventLoop>

#include "chart.h"
#include "chart_factory.h"
#include "sensor.h"

class LineChart : public Chart {
  Q_OBJECT

private:
  QVector<QLineSeries*> seriesVector;
  QValueAxis *axisX;
  QValueAxis *axisY;
  bool stopSimulation = false;
  QFuture<void> future;

  double getMaxYValue() const;
  
  bool initChart() override;
  bool initSeries() override;
  void axisSetDefault();

  void clearSeries();
  bool setSensor(Sensor *sensor) override;

  void updateAxis() override;
  void updateRow(int row);
  void updateSeries(int seriesIndex, int row, double yValue);

  void redrawChart() override;
  void updateIncrementally() override;
  void runSimulation() override;

public:
  Q_INVOKABLE LineChart(QString title, Sensor *sensor, QWidget *parent = nullptr);
  LineChart(const LineChart &other);
  LineChart* clone() const override;
  virtual ~LineChart() override;

  void updateTheme() override;
};

#endif // LINE_CHART_H