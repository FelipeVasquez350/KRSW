#ifndef CHART_FACTORY_H
#define CHART_FACTORY_H

#include <QMap>
#include <QMetaObject>
#include <QString>

#include "chart.h"

class ChartFactory {
public:
  static Chart* createChart(QString chartType, QString title, Sensor *sensor, QWidget *parent = nullptr);

  static void registerChartType(const QMetaObject &metaObject);

  static QStringList getChartTypes();

private:
  ChartFactory() = default;
  ChartFactory(const ChartFactory&) = delete;
  ChartFactory& operator=(const ChartFactory&) = delete;

  static QMap<QString, QMetaObject> chartConstructors;
};

#endif // CHART_FACTORY_H
  