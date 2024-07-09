#include "cpu_sensor.h"

static bool isRegistered = [] {
  SensorFactory::registerSensorType(CPUSensor::staticMetaObject);
  return true;
}();

void CPUSensor::getCPUTimes(std::vector<size_t>& times) const {
  std::ifstream procStat = std::ifstream("/proc/stat");
  procStat.ignore(5, ' ');
  times.clear();
  for (size_t time, i = 0; i < 4 && procStat >> time; ++i) {
    times.push_back(time);
  }
}

double CPUSensor::calculateCPUUsage() const {
  std::vector<size_t> times1(4), times2(4);
  getCPUTimes(times1);
  std::this_thread::sleep_for(std::chrono::milliseconds(350));
  getCPUTimes(times2);
  
  double overall = std::accumulate(times2.begin(), times2.end(), 0) - std::accumulate(times1.begin(), times1.end(), 0);
  double idleTime = times2[3] - times1[3];
  return 100.0 * (overall - idleTime) / overall;
}

CPUSensor::CPUSensor(QObject *parent) : Sensor(parent), watcher(QFutureWatcher<double>()) {
  connect(&watcher, &QFutureWatcher<double>::finished, [this]() {
    double value = watcher.future().result();
    addData(value);
    emit valueChanged(ChartDrawType::IncrementalUpdate);
  });
}

CPUSensor::CPUSensor(const CPUSensor &other) : Sensor(other), watcher(QFutureWatcher<double>()) {
  connect(&watcher, &QFutureWatcher<double>::finished, [this]() {
    double value = watcher.future().result();
    addData(value);
    emit valueChanged(ChartDrawType::IncrementalUpdate);
  });
}

CPUSensor::~CPUSensor() {
  disconnect(&watcher, &QFutureWatcher<double>::finished, nullptr, nullptr);
}

CPUSensor* CPUSensor::clone() const {
  return new CPUSensor(*this);
}

void CPUSensor::retrieveData() {
  watcher.setFuture(QtConcurrent::run(&CPUSensor::calculateCPUUsage, this));
}

QMetaType::Type CPUSensor::getQVariantType() const {
  return QMetaType::Double;
}

bool CPUSensor::isReadingRelative() const {
  return true;
}

QVector<QString> CPUSensor::getReadingLables() const {
  return {"CPU Usage"};
}

std::pair<double, QString>  CPUSensor::getScaleUnit(double) const {
  return {1, "%"};
}

QJsonObject CPUSensor::serialize() const {
  QJsonObject json;
  QJsonArray values;

  int row = rowCount();
  for (int i = 0; i < row; i++) {
    QModelIndex index = this->index(i);
    values.append(QJsonValue(this->data(index, Qt::DisplayRole).toDouble()));
  }
  json["sensor"] = "CPUSensor";
  json["values"] = values;
  return json;
}

void CPUSensor::deserialize(const QJsonObject &json) {
  QJsonArray data = json["values"].toArray();

  for (auto value : data) {
    addData(value.toDouble());
  }
}