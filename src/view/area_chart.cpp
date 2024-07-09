#include "area_chart.h"

static bool isRegistered = [] {
  ChartFactory::registerChartType(AreaChart::staticMetaObject);
  return true;
}();

AreaChart::AreaChart(QString title, Sensor *sensor, QWidget *parent) : Chart(title, sensor, parent), axisX(new QValueAxis()), axisY(new QValueAxis()) {
  connect(sensor, &Sensor::valueChanged, this, &AreaChart::updateChart);

  initChart();
  initSeries();

  QChartView *chartView = new QChartView(chart);
  chartView->setRenderHint(QPainter::Antialiasing);
  chartView->setChart(chart);
  chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  
  QGridLayout *layout = new QGridLayout();
  layout->addWidget(chartView);  
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);  

  updateTheme();
  redrawChart();
}

AreaChart::AreaChart(const AreaChart &other) : AreaChart(other.title, other.sensor->clone(), other.parentWidget()) {
}

AreaChart* AreaChart::clone() const {
  return new AreaChart(*this);
}

AreaChart::~AreaChart() {
  disconnect(sensor, &Sensor::valueChanged, this, &AreaChart::updateChart);
  stopSimulation = true;
  future.waitForFinished();

  qDeleteAll(seriesVector);
  qDeleteAll(upperSeriesVector);  
  qDeleteAll(lowerSeriesVector);

  delete axisX;
  delete axisY;
}

bool AreaChart::initChart() {
  chart->setTitle(this->title);
  chart->setTitleFont(QFont("Roboto", 20));  
  chart->legend()->setVisible(true);
  chart->legend()->setAlignment(Qt::AlignBottom);
  chart->legend()->setShowToolTips(true);
  chart->legend()->setFont(QFont("Roboto", 14));
  chart->setAnimationOptions(QChart::SeriesAnimations);

  axisSetDefault();
  chart->addAxis(axisX, Qt::AlignBottom);
  chart->addAxis(axisY, Qt::AlignLeft);

  return true;
}

bool AreaChart::initSeries() {  
  QVector<QString> categories = sensor->getReadingLables();

  QColor colors[] = {QColor(255, 55, 55), QColor(117, 22, 215)};

  for (int i=0; i < categories.size(); i++) {
    QAreaSeries *series = new QAreaSeries();
    series->setName(categories[i]);
    series->setUpperSeries(new QLineSeries());
    series->setLowerSeries(new QLineSeries());
    
    QColor color = colors[i];
    color.setAlphaF(0.1);
    series->setBrush(color);

    series->setPen(colors[i]);
    series->setBorderColor(colors[i]);

    chart->addSeries(series);
    chart->legend()->markers(series)[0]->setBrush(colors[i]);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    seriesVector.push_back(series);
    upperSeriesVector.push_back(series->upperSeries());
    lowerSeriesVector.push_back(series->lowerSeries());
  }   
  
  return true;
} 
  
void AreaChart::axisSetDefault() {
  axisX->setRange(0, 100);
  axisX->setVisible(false);
  
  std::pair<double, QString> scaleUnitPair = sensor->getScaleUnit(100);

  axisY->setRange(0, 100);
  axisY->setLabelFormat("%.1f " + scaleUnitPair.second);
}

void AreaChart::clearSeries() {
  for (auto& series : upperSeriesVector) {
    series->clear();
  }
  for (auto& series : lowerSeriesVector) {
    series->clear();
  }
  chart->axes(Qt::Horizontal).first()->setRange(0, 100);
}

bool AreaChart::setSensor(Sensor *sensor) {
  if (this->sensor != nullptr) {
    disconnect(this->sensor, &Sensor::valueChanged, this, &AreaChart::updateChart);

    for (auto series : seriesVector) {
      chart->removeSeries(series);
    }
    
    seriesVector.clear();
    upperSeriesVector.clear();
    lowerSeriesVector.clear();
  }
  this->sensor = sensor;  
  initSeries();
  axisSetDefault();

  connect(sensor, &Sensor::valueChanged, this, &AreaChart::updateChart);
  return true;
}

double AreaChart::getMaxYValue() const {
  double maxYValue = std::numeric_limits<double>::lowest();

  for (auto series : upperSeriesVector) {
    int end = series->count();
    int start = std::max(0, end - 100);
    for (int i = start; i < end; i++) {
      auto point = series->at(i);
      maxYValue = std::max(maxYValue, point.y());
    }
  }

  return maxYValue;
}

void AreaChart::updateAxis() {
  double max = getMaxYValue();
  if (max == 0 || max == std::numeric_limits<double>::lowest() || axisY->max() == max) return;
  
  std::pair<double, QString> scaleUnitPair = sensor->getScaleUnit(max * this->scaleFactor); 
  double scaleFactorRatio = this->scaleFactor / scaleUnitPair.first;

  for (auto* series : upperSeriesVector) {
    QVector<QPointF> points = series->points();
    for (auto& point : points) {
      point.setY(point.y() * scaleFactorRatio);
    }
    series->replace(points);
  }
    
  axisY->setLabelFormat("%.1f " + scaleUnitPair.second);
  chart->axes(Qt::Vertical).first()->setRange(0, max * scaleFactorRatio);

  this->scaleFactor = scaleUnitPair.first;
}

void AreaChart::updateRow(int row) {
  QModelIndex index = sensor->index(row);
  QVariant value = sensor->data(index, Qt::DisplayRole);

  int currentRow = upperSeriesVector[0]->count();

  if (value.typeId() == QMetaType::Double || value.typeId() == QMetaType::Int) {
    double yValue = value.toDouble();
    updateSeries(0, currentRow, yValue);
  } 
  else if (value.canConvert<QPair<double, double>>()) {
    QPair<double, double> pair = value.value<QPair<double, double>>();
    updateSeries(0, currentRow, pair.first);
    updateSeries(1, currentRow, pair.second);
  }

  if (currentRow > 100) {
    chart->axes(Qt::Horizontal).first()->setRange(currentRow - 100, currentRow);
  }
    
  if(!sensor->isReadingRelative())
    updateAxis();
}

void AreaChart::updateSeries(int seriesIndex, int row, double yValue) {
  upperSeriesVector[seriesIndex]->append(row, yValue/this->scaleFactor);
  lowerSeriesVector[seriesIndex]->append(row, 0);  

  std::pair<double, QString> scaleUnitPair  = sensor->getScaleUnit(yValue);

  QList<QLegendMarker*> markers = chart->legend()->markers();
  QString label = QString("%1: %2 %3").arg(seriesVector[seriesIndex]->name()).arg(QString::number(yValue/scaleUnitPair.first, 'f', 2)).arg(scaleUnitPair.second);
  markers.at(seriesIndex)->setLabel(label);
}

void AreaChart::redrawChart() {
  clearSeries();
  int endRow = sensor->rowCount() - 1;
  for (int row = 0; row <= endRow; ++row) {
    updateRow(row);
  }
}

void AreaChart::updateIncrementally() {
  updateRow(sensor->rowCount() - 1);
}

void AreaChart::runSimulation() {
  clearSeries();
  QString unit = sensor->getScaleUnit(100).second;
  axisY->setLabelFormat("%.1f " + unit);
  axisY->setMax(100);
  this->scaleFactor = 1;

  future = QtConcurrent::run([this] {
    QThread::msleep(700);
    int row = 0;
    while(row <= sensor->rowCount()) {
      if(stopSimulation) {
        return;
      }
      QThread::msleep(300);
      if (this->getSensor()->getState() == SensorState::Simulating) {
        updateRow(row);
        row++;
      }
    }
    this->getSensor()->setState(SensorState::Stopped);
  });
}

void AreaChart::updateTheme() {
  QPalette palette = this->palette();

  if (this->palette().window().color().lightness() < 128) {
    chart->setTheme(QChart::ChartThemeDark);  
  }

  chart->setBackgroundBrush(QBrush(palette.button().color()));
  
  QColor colors[] = {QColor(255, 55, 55), QColor(117, 22, 215)};

  for (int i=0; i < seriesVector.size(); i++) {
    QColor color = colors[i];
    color.setAlphaF(0.1);
    seriesVector[i]->setBrush(color);

    seriesVector[i]->setPen(colors[i]);
    seriesVector[i]->setBorderColor(colors[i]);
  }
}