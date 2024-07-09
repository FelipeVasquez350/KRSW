#include "pie_chart.h"

static bool isRegistered = [] {
  ChartFactory::registerChartType(PieChart::staticMetaObject);
  return true;
}();

PieChart::PieChart(QString title, Sensor* sensor, QWidget* parent) : Chart(title, sensor, parent) {
  connect(sensor, &Sensor::valueChanged, this, &PieChart::updateChart);

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

  redrawChart();
}

PieChart::PieChart(const PieChart &other) : PieChart(other.title, other.sensor->clone(), other.parentWidget()) {
}

PieChart* PieChart::clone() const {
  return new PieChart(*this);
}

PieChart::~PieChart() {
  disconnect(sensor, &Sensor::valueChanged, this, &PieChart::updateChart);
  stopSimulation = true;
  future.waitForFinished();
    
  for (auto serie : series) {
    chart->removeSeries(serie);
    delete serie;
  }
}

bool PieChart::initChart() {
  chart->setTitle(this->title);
  chart->setTitleFont(QFont("Roboto", 20));
  chart->legend()->setAlignment(Qt::AlignBottom);
  chart->legend()->setFont(QFont("Roboto", 14));
  chart->setAnimationOptions(QChart::SeriesAnimations);

  return true;
}

bool PieChart::initSeries() {
  QVector<QString> labels = sensor->getReadingLables();
  QString unit = sensor->getScaleUnit(0).second;

  for (int i = 0; i < labels.size(); ++i) {
    series.append(new QPieSeries(chart));
    slices.append(series[i]->append(labels[i], 0));
    slices.append(series[i]->append("", 100));

    series[i]->setHoleSize(0.2 + i * 0.3);
    series[i]->setPieSize(0.5 + i * 0.3);
    chart->addSeries(series[i]);
  }

  QList<QLegendMarker *> marker = chart->legend()->markers();
  for (int i = 0; i < marker.size()/2; i++) {
    marker[i*2]->setLabel(labels[i]+": 0.00 " + unit);
    marker[i*2+1]->setVisible(false); 
  }  
  
  updateTheme();
  return true;
}

void PieChart::clearSeries() {
  slices.clear();
}

bool PieChart::setSensor(Sensor* sensor) {
  if (this->sensor != nullptr) {
    disconnect(this->sensor, &Sensor::valueChanged, this, &PieChart::updateChart);
    for (auto serie : series) {
      chart->removeSeries(serie);
    }

    series.clear();
    slices.clear();
  }
  this->sensor = sensor;
  initSeries();

  connect(sensor, &Sensor::valueChanged, this, &PieChart::updateChart);
  return true;
}


QPair<double, double> PieChart::getMinMaxValues() const {
  double maxValue = std::numeric_limits<double>::lowest();
  double minValue = std::numeric_limits<double>::max();

  int startSliceIndex = std::max(0, static_cast<int>(slices.size()) - 10);

  for (int sliceIndex = startSliceIndex; sliceIndex < slices.size(); ++sliceIndex) {
    QPieSlice* slice = slices[sliceIndex];
    double value = slice->value();
    if (sliceIndex % 2 == 0) {
      maxValue = std::max(maxValue, value);
    } else {
      minValue = std::min(minValue, value);
    }
  }
  return qMakePair(maxValue, minValue);
}

void PieChart::updateAxis() {
  QPair<double, double> minMaxValues = getMinMaxValues();
  double max = std::max(minMaxValues.first, -minMaxValues.second);

  std::pair<double, QString> scaleUnitPair = sensor->getScaleUnit(max * this->scaleFactor); 
  double scaleFactorRatio = this->scaleFactor / scaleUnitPair.first;

  for (int i = 0; i < slices.size(); i++) {
    if(i % 2 == 0) 
      slices[i]->setValue(slices[i]->value() * scaleFactorRatio);
    else
      slices[i]->setValue(100 - slices[i-1]->value());
  }
  for (int i = 0; i < slices.size(); i+=2) {
    slices[i]->setLabel(QString::number(slices[i]->value(), 'f', 2) + scaleUnitPair.second);
  }
  this->scaleFactor = scaleUnitPair.first;
}

void PieChart::updateRow(int row) {
  QModelIndex index = sensor->index(row);
  QVariant value = sensor->data(index, Qt::DisplayRole);

  if (value.typeId() == QMetaType::Double || value.typeId() == QMetaType::Int) {
    double yValue = value.toDouble();
    updateSeries(0, yValue);
  } else if (value.canConvert<QPair<double, double>>()) {
    QPair<double, double> pair = value.value<QPair<double, double>>();
    updateSeries(0, pair.first);
    updateSeries(1, pair.second);
  }

  if(!sensor->isReadingRelative())
    updateAxis();
}

void PieChart::updateSeries(int seriesIndex, double value) {
  slices[seriesIndex*2]->setValue(value/this->scaleFactor);
  slices[seriesIndex*2+1]->setValue(100 - value/this->scaleFactor);
  slices[seriesIndex*2]->setLabel(QString::number((value/this->scaleFactor), 'f', 2) + "%");

  std::pair<double, QString> scaleUnitPair = sensor->getScaleUnit(value);
  
  QList<QLegendMarker*> markers = chart->legend()->markers();
  QString label = QString("%1: %2 %3").arg(sensor->getReadingLables()[seriesIndex]).arg(QString::number(value/scaleUnitPair.first, 'f', 2)).arg(scaleUnitPair.second);
  markers.at(seriesIndex*2)->setLabel(label);
}

void PieChart::redrawChart() {
  clearSeries();
  int endRow = sensor->rowCount() - 1;
  for (int row = 0; row <= endRow; ++row) {
    updateRow(row);
  }
}

void PieChart::updateIncrementally() {
  updateRow(sensor->rowCount() - 1);
}

void PieChart::runSimulation() {
  clearSeries();
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

void PieChart::explodeSlice(QPieSlice* slice, bool state) {    
  if (state) {
    qreal startAngle = slice->startAngle();
    qreal endAngle = slice->startAngle() + slice->angleSpan();

    QPieSeries *s = slice->series();
    int seriesIndex = series.indexOf(s);

    for (int i = seriesIndex + 1; i < series.count(); i++) {
      series.at(i)->setPieStartAngle(endAngle);
      series.at(i)->setPieEndAngle(360 + startAngle);
    }
  } else {
    for (int i = 0; i < series.count(); i++) {
      series.at(i)->setPieStartAngle(0);
      series.at(i)->setPieEndAngle(360);
    }
  }
  slice->setExploded(state);
}

void PieChart::updateTheme() {
 QPalette palette = this->palette();

  if (this->palette().window().color().lightness() < 128) {
    chart->setTheme(QChart::ChartThemeDark);  
  }

  chart->setBackgroundBrush(QBrush(palette.button().color()));  
  
  QColor colors[] = {QColor(169, 11, 175), palette.button().color(), QColor(201, 126, 52), QColor(6, 132, 187)};
  for (int i = 0; i < slices.size(); ++i) {
    slices[i]->setColor(colors[i]);
    slices[i]->setBorderColor(colors[i]);
    if (i % 2 == 0) {
      slices[i]->setLabelColor(QColor(255, 255, 255, 255));
    }
  }
  slices[1]->setPen(QPen(palette.button().color()));

  for (int i = 0; i < slices.size(); ++i) {
    if (i != 1) {
      QPieSlice *slice = slices[i];
      QColor originalColor = slice->color();
      connect(slice, &QPieSlice::hovered, [slice, originalColor, this](bool state){
        this->explodeSlice(slice, state);
        slice->setColor(state ? originalColor.lighter() : originalColor);
      });
    }
  }
}