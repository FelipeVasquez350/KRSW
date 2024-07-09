#include "chart.h"

Chart::Chart(QString title, Sensor *sensor, QWidget *parent) : QWidget(parent), title(title), sensor(sensor), chart(new QChart()), scaleFactor(1) {
}

Chart::Chart(const Chart &other) : QWidget(other.parentWidget()), title(other.title), sensor(other.sensor->clone()), chart(new QChart()), scaleFactor(other.scaleFactor) {
}

Chart::~Chart() {
  delete chart;
}

void Chart::updateChart(ChartDrawType type) {
  switch (type) {
    case FullRedraw:
      redrawChart();
      break;
    case IncrementalUpdate:
      updateIncrementally();
      break;
    case SimulationMode:
      runSimulation();
      break;
  }
}

QString Chart::getTitle() const {
  return chart->title();
}

void Chart::setTitle(QString title) {
  chart->setTitle(title);
}

Sensor* Chart::getSensor() const {
  return sensor;
}
