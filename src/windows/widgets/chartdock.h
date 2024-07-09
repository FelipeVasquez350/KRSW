#ifndef CHART_DOCK_H
#define CHART_DOCK_H

#include <QDockWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCloseEvent>

#include "chart.h"

class ChartDock : public QDockWidget {
  Q_OBJECT

private:
  Chart* chart;
  Sensor* sensor;
  QLabel *sensorLabel;
  QLabel *statusLabel;

private slots:
  void handleStateChanged(SensorState state);

public:
  ChartDock(Chart* chart, Sensor* sensor, QWidget *parent = nullptr);
  ChartDock(const ChartDock &other); 
  ~ChartDock(); 
  ChartDock* clone() const; 

  void createContainer();
  Chart* getChart() const;
  void setChart(Chart* chart);
  Sensor* getSensor() const;
  void setSensor(Sensor* sensor);
  void updateSensorLabel();

protected:
  void closeEvent(QCloseEvent *event) override;

signals:
  void removeBeforeDeletion(ChartDock *dock);
};
#endif // CHART_DOCK_H