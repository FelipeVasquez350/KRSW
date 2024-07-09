#ifndef CHART_H
#define CHART_H

#include <QWidget>
#include <QtCharts/QChart>

#include "sensor.h"
#include "enums.h"

class Chart : public QWidget {
  Q_OBJECT

protected:
  QString title;
  Sensor *sensor;
  QChart *chart;
  double scaleFactor;

  virtual bool initChart() = 0;
  virtual bool initSeries() = 0;
  virtual void updateAxis() = 0;

  virtual void redrawChart() = 0;
  virtual void updateIncrementally() = 0;
  virtual void runSimulation() = 0;

public slots:
  void updateChart(ChartDrawType type = IncrementalUpdate);

public:
  Chart(QString title, Sensor *sensor, QWidget *parent = nullptr);
  Chart(const Chart &other);
  virtual Chart* clone() const = 0;
  virtual ~Chart();

  void setTitle(QString title); 
  QString getTitle() const;

  virtual bool setSensor(Sensor *sensor) = 0;
  virtual Sensor* getSensor() const;  

  virtual void updateTheme() = 0;
};

#endif // CHART_H