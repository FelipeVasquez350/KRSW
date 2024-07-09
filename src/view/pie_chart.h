#ifndef PIE_CHART_H
#define PIE_CHART_H

#include <QGridLayout>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QLegendMarker>

#include <QtConcurrent/QtConcurrentRun>
#include <QEventLoop>

#include "chart.h"
#include "chart_factory.h"
#include "sensor.h"


class PieChart : public Chart {
  Q_OBJECT

private:
  QVector<QPieSeries*> series;
  QVector<QPieSlice*> slices;
  bool stopSimulation = false;
  QFuture<void> future;
  
  QPair<double, double> getMinMaxValues() const;
  void explodeSlice(QPieSlice* slice,bool state);

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
  Q_INVOKABLE PieChart(QString title, Sensor* sensor, QWidget* parent = nullptr);
  PieChart(const PieChart& other);
  PieChart* clone() const override;
  virtual ~PieChart() override;

  void updateTheme() override;
};

#endif // PIE_CHART_H