#ifndef AREA_CHART_H
#define AREA_CHART_H

#include <QGridLayout>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLegendMarker>

#include <QEasingCurve>

#include <QtConcurrent/QtConcurrentRun>
#include <QEventLoop>

#include "chart.h"
#include "chart_factory.h"
#include "sensor.h"

class AreaChart : public Chart {
  Q_OBJECT

private:
  QVector<QAreaSeries*> seriesVector;
  QVector<QLineSeries*> upperSeriesVector;
  QVector<QLineSeries*> lowerSeriesVector;
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
  Q_INVOKABLE AreaChart(QString title, Sensor *sensor, QWidget *parent = nullptr);
  AreaChart(const AreaChart &other);
  AreaChart* clone() const override;
  virtual ~AreaChart() override;

  void updateTheme() override;
};

#endif // AREA_CHART_H