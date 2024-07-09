#include "chartdock.h"

ChartDock::ChartDock(Chart* chart, Sensor* sensor, QWidget *parent): QDockWidget(parent), chart(chart), sensor(sensor)  {
  connect(sensor, &Sensor::stateChanged, this, &ChartDock::handleStateChanged);
  createContainer();
}

ChartDock::ChartDock(const ChartDock &other): ChartDock(other.chart, other.sensor, other.parentWidget()) {}

ChartDock::~ChartDock() {
  delete chart;
  delete sensor;
  delete sensorLabel;
  delete statusLabel;
}

ChartDock* ChartDock::clone() const {
  Chart *chart = this->chart->clone();
  Sensor *sensor = chart->getSensor();
  ChartDock *chartDock = new ChartDock(chart, sensor, this->parentWidget());
  return chartDock;
}

void ChartDock::createContainer() {
  QWidget *container = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout;
  QHBoxLayout *hLayout = new QHBoxLayout;
      

  this->sensorLabel = new QLabel(QString("<b>Sensor Type:</b> ") + chart->getSensor()->metaObject()->className());
  hLayout->addWidget(sensorLabel);

  QLabel *chartLabel = new QLabel(QString("<b>Chart Type:</b> ") + chart->metaObject()->className());
  hLayout->addWidget(chartLabel);

  this->statusLabel = new QLabel();
  handleStateChanged(sensor->getState());
  hLayout->addWidget(this->statusLabel);

  QPushButton *button = new QPushButton("Simulate");
  connect(button, &QPushButton::clicked, [this]() {
    chart->getSensor()->startSimulation();
    statusLabel->setText(QString("<b>Status:</b> ") + "Simulating...");
  });
  hLayout->addWidget(button);

  layout->addLayout(hLayout);
  layout->addWidget(chart);

  container->setLayout(layout);
  container->setMinimumSize(494,394);
  QDockWidget::setWidget(container);  
}

Chart* ChartDock::getChart() const {
  return chart;
}

void ChartDock::setChart(Chart* chart) {
  if (this->chart) {
    delete this->chart;
  }

  this->chart = chart;
  this->chart->setSensor(sensor);      

  createContainer();
}

Sensor* ChartDock::getSensor() const {
  return sensor;
}

void ChartDock::setSensor(Sensor* sensor) {
  this->chart->setSensor(sensor);
  if (this->sensor) {
    delete this->sensor;
  }
  this->sensor = sensor;
  connect(this->sensor, &Sensor::stateChanged, this, &ChartDock::handleStateChanged);
  updateSensorLabel();
}

void ChartDock::updateSensorLabel() {
  sensorLabel->setText(QString("<b>Sensor Type:</b> ") + chart->getSensor()->metaObject()->className());
}

void ChartDock::handleStateChanged(SensorState state) {
  switch (state) {
    case SensorState::Stopped:
      statusLabel->setText("<b>Status:</b> Stopped");
      break;
    case SensorState::Started:
      statusLabel->setText("<b>Status:</b> Started");
      break;
    case SensorState::Simulating:
      statusLabel->setText("<b>Status:</b> Simulating");
      break;
    case SensorState::PausedSimulation:
      statusLabel->setText("<b>Status:</b> Stopped Simulation");
      break;
  }
}

void ChartDock::closeEvent(QCloseEvent *event) {
  emit removeBeforeDeletion(this);
  event->accept();
}
