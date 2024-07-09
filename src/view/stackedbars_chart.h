#ifndef NETOWRK_CHART_H
#define NETOWRK_CHART_H

#include <QGridLayout>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QStackedBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLegendMarker>
#include <QtConcurrent/QtConcurrentRun>
#include <QEventLoop>

#include "chart.h"
#include "chart_factory.h"
#include "sensor.h"

class StackedBarsChart : public Chart {
  Q_OBJECT
  
private:
  QStackedBarSeries* series;
  QVector<QBarSet*> sets;
  QBarCategoryAxis *axisX;
  QValueAxis* axisY;
  bool stopSimulation = false;
  QFuture<void> future;

  QPair<double, double> getMinMaxValues() const;

  bool initChart() override;
  bool initSeries() override; 

  void clearSeries();
  bool setSensor(Sensor *sensor) override;

  void updateAxis() override;
  void updateRow(int row);
  void updateSeries(int seriesIndex, double yValue);

  void redrawChart() override;
  void updateIncrementally() override;
  void runSimulation() override;

public:
  Q_INVOKABLE StackedBarsChart(QString title, Sensor* sensor, QWidget* parent = nullptr);
  StackedBarsChart(const StackedBarsChart& other);
  StackedBarsChart* clone() const override;
  virtual ~StackedBarsChart() override;

  void updateTheme() override;
};

#endif // NETOWRK_CHART_H