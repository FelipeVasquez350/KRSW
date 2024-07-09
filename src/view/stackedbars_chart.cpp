#include "stackedbars_chart.h"

static bool isRegistered = [] {
  ChartFactory::registerChartType(StackedBarsChart::staticMetaObject);
  return true;
}();

StackedBarsChart::StackedBarsChart(QString title, Sensor* sensor, QWidget* parent) : Chart(title, sensor, parent), series(new QStackedBarSeries), axisX(new QBarCategoryAxis()), axisY(new QValueAxis()) {  
  connect(sensor, &Sensor::valueChanged, this, &StackedBarsChart::updateChart);
  
  initChart();
  initSeries();

  QChartView *chartView = new QChartView(chart, parent);
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

StackedBarsChart::StackedBarsChart(const StackedBarsChart &other) : StackedBarsChart(other.title, other.sensor->clone(), other.parentWidget()) {
}

StackedBarsChart* StackedBarsChart::clone() const {
  return new StackedBarsChart(*this);
}

StackedBarsChart::~StackedBarsChart() {
  disconnect(sensor, &Sensor::valueChanged, this, &StackedBarsChart::updateChart);
  stopSimulation = true;
  future.waitForFinished();

  qDeleteAll(sets); 
  delete series; 
  delete axisX; 
  delete axisY; 
}

bool StackedBarsChart::initChart() {
  chart->setTitle(this->title);
  chart->setTitleFont(QFont("Roboto", 20));
  chart->legend()->setVisible(true);
  chart->legend()->setAlignment(Qt::AlignBottom);
  chart->legend()->setShowToolTips(true);
  chart->legend()->setFont(QFont("Roboto", 14));
  
  return true;
}

bool StackedBarsChart::initSeries() { 
  QVector<QString> categories = sensor->getReadingLables();

  for (int i = 0; i < categories.size(); ++i) {
    sets.append(new QBarSet(categories[i])); 
    sets[i]->setPen(QPen(Qt::transparent)); 
    series->append(sets[i]);
  }
  
  for (int i = 0; i < 100; ++i) {
    axisX->append(QString::number(i));
  }
  axisX->setVisible(false);
  
  series->setBarWidth(1.0);
  chart->addSeries(series);

  if (sensor->getQVariantType() == QMetaType::QVariantPair) {
    axisY->setRange(-100, 100);
  }
  else
    axisY->setRange(0, 100);

  std::pair<double, QString> scaleUnitPair = sensor->getScaleUnit(100);
  axisY->setLabelFormat("%.1f " + scaleUnitPair.second);
  chart->addAxis(axisY, Qt::AlignLeft);  
  series->attachAxis(axisY);

  chart->addAxis(axisX, Qt::AlignBottom);  
  series->attachAxis(axisX);


  return true;
}

void StackedBarsChart::clearSeries() {
  for (auto* barSet : sets) {
    barSet->remove(0, barSet->count());
  }
}

bool StackedBarsChart::setSensor(Sensor* sensor) {
  if (this->sensor != nullptr) {
    disconnect(this->sensor, &Sensor::valueChanged, this, &StackedBarsChart::updateChart);
    
    chart->removeSeries(series);
    chart->removeAxis(axisX);
    chart->removeAxis(axisY);
    
    sets.clear();
    series->clear();
  }
  this->sensor = sensor;
  initSeries();

  connect(sensor, &Sensor::valueChanged, this, &StackedBarsChart::updateChart);
  return true;
}

QPair<double, double> StackedBarsChart::getMinMaxValues() const {
  double maxValue = std::numeric_limits<double>::lowest();
  double minValue = std::numeric_limits<double>::max();

  for (int setIndex = 0; setIndex < sets.size(); ++setIndex) {
    QBarSet* set = sets[setIndex];
    for (int i = 0; i < set->count(); ++i) {
      if (setIndex % 2 == 0) {
        maxValue = std::max(maxValue, set->at(i));
      } else {
        minValue = std::min(minValue, set->at(i));
      }
    }
  }

  return qMakePair(maxValue, minValue);
}

void StackedBarsChart::updateAxis() {
  QPair<double, double> minMaxValues = getMinMaxValues();
  if ((minMaxValues.first == std::numeric_limits<double>::lowest() && minMaxValues.second == std::numeric_limits<double>::max()) 
  || (axisY->max() == minMaxValues.first && axisY->min() == minMaxValues.second)) {
    return;
  }
  double max = std::max(minMaxValues.first, -minMaxValues.second);
  
  std::pair<double, QString> scaleUnitPair = sensor->getScaleUnit(max * this->scaleFactor); 
  double scaleFactorRatio = this->scaleFactor / scaleUnitPair.first;

  for (QBarSet* set : sets) {
    for (int i = 0; i < set->count(); ++i) {
      set->replace(i, set->at(i) * scaleFactorRatio);
    }
  }

  axisY->setLabelFormat("%.1f " + scaleUnitPair.second);
  axisY->setMax(minMaxValues.first * scaleFactorRatio);
  axisY->setMin(minMaxValues.second * scaleFactorRatio);

  this->scaleFactor = scaleUnitPair.first;
}

void StackedBarsChart::updateRow(int row) {
  QModelIndex index = sensor->index(row);
  QVariant value = sensor->data(index, Qt::DisplayRole);

  int currentRow = sets[0]->count();

  if (value.typeId() == QMetaType::Double || value.typeId() == QMetaType::Int) {
    double yValue = value.toDouble();
    updateSeries(0, yValue);
  }
  else if (value.canConvert<QPair<double, double>>()) {
    QPair<double, double> pair = value.value<QPair<double, double>>();
    updateSeries(0, pair.first);
    updateSeries(1, -pair.second);
  }

  if (currentRow > 100) {
    chart->axes(Qt::Horizontal).first()->setRange(currentRow - 100, currentRow);
  }
    
  if(!sensor->isReadingRelative())
    updateAxis();
}

void StackedBarsChart::updateSeries(int seriesIndex, double yValue) {
  if (sets[seriesIndex]->count() == 100) {
    sets[seriesIndex]->remove(0);
  }
  sets[seriesIndex]->append(yValue/this->scaleFactor);

  if (seriesIndex == 1) {
    yValue = -yValue;
  }

  std::pair<double, QString> scaleUnitPair = sensor->getScaleUnit(yValue);

  QList<QLegendMarker*> markers = chart->legend()->markers();
  QString label = QString("%1: %2 %3").arg(sets[seriesIndex]->label()).arg(QString::number(yValue/scaleUnitPair.first, 'f', 2)).arg(scaleUnitPair.second);
  markers.at(seriesIndex)->setLabel(label);
}

void StackedBarsChart::redrawChart() {
  clearSeries();
  int endRow = sensor->rowCount() - 1;
  for (int row = 0; row <= endRow; ++row) {
    updateRow(row);
  }
}

void StackedBarsChart::updateIncrementally() {
  updateRow(sensor->rowCount() - 1);
}

void StackedBarsChart::runSimulation() {
  clearSeries();
  if (sensor->getQVariantType() == QMetaType::QVariantPair) {
    axisY->setRange(-100, 100);
  }
  else
    axisY->setRange(0, 100);
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

void StackedBarsChart::updateTheme() {
  QPalette palette = this->palette();

  if (palette.window().color().lightness() < 128) {
    chart->setTheme(QChart::ChartThemeDark);  
  }
  
  chart->setBackgroundBrush(QBrush(palette.button().color()));

  for (QBarSet* set : sets) {
    set->setBorderColor(Qt::transparent);
  }
}