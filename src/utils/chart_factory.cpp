#include "chart_factory.h"

QMap<QString, QMetaObject> ChartFactory::chartConstructors;

Chart* ChartFactory::createChart(QString chartType, QString title, Sensor *sensor, QWidget *parent) {
  auto it = chartConstructors.find(chartType);
  if (it != chartConstructors.end()) {
    QObject *obj = it.value().newInstance(Q_ARG(QString, title), Q_ARG(Sensor*, sensor), Q_ARG(QWidget*, parent));
    return qobject_cast<Chart*>(obj);
  } else {
    return nullptr;
  }
}

void ChartFactory::registerChartType(const QMetaObject &metaObject) {
  chartConstructors[metaObject.className()] = metaObject;
}

QStringList ChartFactory::getChartTypes() {
  QStringList types;
  for (auto it = chartConstructors.begin(); it != chartConstructors.end(); ++it) {
    types.append(it.key());
  }
  return types;
}
