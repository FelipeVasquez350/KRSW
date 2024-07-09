#include "line_chart.h"

static bool isRegistered = [] {
  ChartFactory::registerChartType(LineChart::staticMetaObject);
  return true;
}();

LineChart::LineChart(QString title, Sensor *sensor, QWidget *parent) : Chart(title, sensor,parent), axisX(new QValueAxis()), axisY(new QValueAxis()) {   
  connect(sensor, &Sensor::valueChanged, this, &LineChart::updateChart);
  
  initChart();
  initSeries();

  QChartView *chartView = new QChartView(chart, parent);
  chartView->setRenderHint(QPainter::Antialiasing);
  chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  chartView->setChart(chart);

  QGridLayout *layout = new QGridLayout();
  layout->addWidget(chartView);
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);

  updateTheme();
  redrawChart();
}

LineChart::LineChart(const LineChart &other) : LineChart(other.title, other.sensor->clone(), other.parentWidget()) {
}

LineChart* LineChart::clone() const {
  return new LineChart(*this);
}

LineChart::~LineChart() {
  disconnect(sensor, &Sensor::valueChanged, this, &LineChart::updateChart);
  stopSimulation = true;
  future.waitForFinished();

  qDeleteAll(seriesVector);
  delete axisX;
  delete axisY;
}

bool LineChart::initChart() {
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

bool LineChart::initSeries() {
  QVector<QString> lables = sensor->getReadingLables();

  QColor colors[] = {QColor(241, 222, 38), QColor(157, 158, 163)};

  for (int i=0; i<lables.size(); i++) {
    QLineSeries *series = new QLineSeries();
    series->setName(lables[i]);
    chart->addSeries(series);

    series->attachAxis(axisX);
    series->attachAxis(axisY);
    series->setColor(colors[i]);

    seriesVector.append(series);
  }
  
  std::pair<double, QString> scaleUnitPair = sensor->getScaleUnit(100);
  axisY->setLabelFormat("%.1f " + scaleUnitPair.second);
  return true;
}

void LineChart::axisSetDefault() {
  axisX->setRange(0, 100);
  axisX->setVisible(false);
  
  std::pair<double, QString> scaleUnitPair = sensor->getScaleUnit(100);

  axisY->setRange(0, 100);
  axisY->setLabelFormat("%.1f " + scaleUnitPair.second);
}

void LineChart::clearSeries() {
  for (auto* series : seriesVector) {
    series->clear();
  }
  chart->axes(Qt::Horizontal).first()->setRange(0, 100);
} 

bool LineChart::setSensor(Sensor *sensor) {
  if (this->sensor != nullptr) {
    disconnect(this->sensor, &Sensor::valueChanged, this, &LineChart::updateChart);

    for (auto* series : seriesVector) {
      if (chart->series().contains(series)) {
        chart->removeSeries(series);
      }
    }

    seriesVector.clear();
  }
  this->sensor = sensor;
  initSeries();
  axisSetDefault();

  connect(this->sensor, &Sensor::valueChanged, this, &LineChart::updateChart);
  return true;
}

double LineChart::getMaxYValue() const {
  double maxYValue = std::numeric_limits<double>::lowest();

  for (auto series : seriesVector) {
    int end = series->count();
    int start = std::max(0, end - 100);
    for (int i = start; i < end; i++) {
      auto point = series->at(i);
      maxYValue = std::max(maxYValue, point.y());
    }
  }

  return maxYValue;
}

void LineChart::updateAxis() {
  double max = getMaxYValue();
  if (max == 0 || max == std::numeric_limits<double>::lowest() || axisY->max() == max) return;

  std::pair<double, QString> scaleUnitPair = sensor->getScaleUnit(max * this->scaleFactor); 
  double scaleFactorRatio = this->scaleFactor / scaleUnitPair.first;

  for (auto* series : seriesVector) {
    QVector<QPointF> points = series->points();
    for (auto& point : points) {
      point.setY(point.y() * scaleFactorRatio);
    }
    series->replace(points);
  }
  
  axisY->setLabelFormat("%.1f " + scaleUnitPair.second);
  axisY->setMax(max * scaleFactorRatio);
  
  this->scaleFactor = scaleUnitPair.first;
}

void LineChart::updateRow(int row) {
  QModelIndex index = sensor->index(row);
  QVariant value = sensor->data(index, Qt::DisplayRole);

  int currentRow = seriesVector[0]->count();

  if (value.typeId() == QMetaType::Double || value.typeId() == QMetaType::Int) {
    double yValue = value.toDouble();
    updateSeries(0, currentRow, yValue);
  } else if (value.canConvert<QPair<double, double>>()) {
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

void LineChart::updateSeries(int seriesIndex, int row, double yValue) {  
  seriesVector[seriesIndex]->append(row, yValue/this->scaleFactor);

  std::pair<double, QString> scaleUnitPair  = sensor->getScaleUnit(yValue);
  
  QList<QLegendMarker*> markers = chart->legend()->markers();
  QString label = QString("%1: %2 %3").arg(seriesVector[seriesIndex]->name()).arg(QString::number(yValue/scaleUnitPair.first, 'f', 2)).arg(scaleUnitPair.second);
  markers.at(seriesIndex)->setLabel(label);
}

void LineChart::redrawChart() {
  clearSeries();
  int endRow = sensor->rowCount() - 1;
  for (int row = 0; row <= endRow; ++row) {
    updateRow(row);
  }
}

void LineChart::updateIncrementally() {
  updateRow(sensor->rowCount() - 1);
}

void LineChart::runSimulation() {
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

void LineChart::updateTheme() {
 QPalette palette = this->palette();

  if (this->palette().window().color().lightness() < 128) {
    chart->setTheme(QChart::ChartThemeDark);  
  }

  chart->setBackgroundBrush(QBrush(palette.button().color()));
  QColor colors[] = {QColor(241, 222, 38), QColor(157, 158, 163)};

  for (int i=0; i < seriesVector.size(); i++) {
    seriesVector[i]->setColor(colors[i]);
  }
}
