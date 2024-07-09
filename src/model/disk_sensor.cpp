#include "disk_sensor.h"

static bool isRegistered = [] {
  SensorFactory::registerSensorType(DiskSensor::staticMetaObject);
  return true;
}();

std::string DiskSensor::getPrimaryDrive() const {
  std::string command = "lsblk -no pkname $(df --output=source / | tail -1)";

  std::array<char, 128> buffer;
  std::string result;

  auto pcloseDeleter = [](FILE* f) { pclose(f); };
  std::unique_ptr<FILE, decltype(pcloseDeleter)> pipe(popen(command.c_str(), "r"), pcloseDeleter);  
  
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }

  result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());

  if (result.substr(0, 5) == "/dev/") {
    result = result.substr(5);
  }

  return result;
}

std::pair<long long, long long> DiskSensor::getDiskUsage(const std::string& drive) const {
  std::ifstream file = std::ifstream("/proc/diskstats");
  if (!file) {
    throw std::runtime_error("Failed to open /proc/diskstats");
  }
  std::string line;

  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string field;
    std::vector<std::string> fields;

    while (iss >> field) {
      fields.push_back(field);
    }

    if (fields[2] == drive) {
      long long readSectors = std::stoll(fields[5]);
      long long writeSectors = std::stoll(fields[9]);
      return {readSectors, writeSectors};
    }
  }
  return {0, 0}; 
}

int DiskSensor::getSectorSize(const std::string& disk) const {
  std::string command = "lsblk -o PHY-SEC /dev/" + disk + " | grep -v PHY-SEC";
  std::array<char, 128> buffer;
  std::string result;
  
  auto pcloseDeleter = [](FILE* f) { pclose(f); };
  std::unique_ptr<FILE, decltype(pcloseDeleter)> pipe(popen(command.c_str(), "r"), pcloseDeleter);
    
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return std::stoi(result);
}

std::pair<double, double> DiskSensor::getReadWriteSpeeds(const std::string& disk) const {
  int sectorSize = getSectorSize(disk);

  if (sectorSize < 0) {
    sectorSize = 512;
  }

  std::pair<long long, long long> startStats = getDiskUsage(disk);
  std::this_thread::sleep_for(std::chrono::milliseconds(450));
  std::pair<long long, long long> endStats = getDiskUsage(disk);   

  long long readSpeed = (endStats.first - startStats.first)*2 * sectorSize; 
  long long writeSpeed = (endStats.second - startStats.second)*2 * sectorSize;

  return {readSpeed, writeSpeed};
}

DiskSensor::DiskSensor(QObject *parent) : Sensor(parent), watcher(QFutureWatcher<std::pair<double, double>>()) {
  connect(&watcher, &QFutureWatcher<std::pair<double, double>>::finished, [this]() {
  std::pair<double, double> value = watcher.future().result();
    addData(QVariant::fromValue(value));
    emit valueChanged(ChartDrawType::IncrementalUpdate);
  });
}

DiskSensor::DiskSensor(const DiskSensor &other) : Sensor(other), watcher(QFutureWatcher<std::pair<double, double>>()) {
  connect(&watcher, &QFutureWatcher<std::pair<double, double>>::finished, [this]() {
    std::pair<double, double> value = watcher.future().result();
    addData(QVariant::fromValue(value));
    emit valueChanged(ChartDrawType::IncrementalUpdate);
  });
}

DiskSensor* DiskSensor::clone() const {
  return new DiskSensor(*this);
}

DiskSensor::~DiskSensor() {
  disconnect(&watcher, &QFutureWatcher<std::pair<double, double>>::finished, nullptr, nullptr);
}

void DiskSensor::retrieveData() {
  std::string drive = getPrimaryDrive();
  watcher.setFuture(QtConcurrent::run(&DiskSensor::getReadWriteSpeeds, this, drive));
}

QMetaType::Type DiskSensor::getQVariantType() const {
  return QMetaType::QVariantPair;
}

bool DiskSensor::isReadingRelative() const {
  return false;
}

QVector<QString> DiskSensor::getReadingLables() const {
  return {"Read", "Write"};
}

std::pair<double, QString>  DiskSensor::getScaleUnit(double value) const {
  const char* units[] = {"B/s", "KiB/s", "MiB/s", "GiB/s"};
  int unitIndex = 0;
  double formattedSpeed = value;
  while (formattedSpeed >= 1024 && unitIndex < 3) {
    formattedSpeed /= 1024;
    unitIndex++;
  }  
  return {std::pow(1024,unitIndex), units[unitIndex]};
}

QJsonObject DiskSensor::serialize() const {
  QJsonObject json;
  QJsonArray values;

  int row = rowCount();
  for (int i = 0; i < row; i++) {
    QModelIndex index = this->index(i);
    QVariant dataValue = this->data(index, Qt::DisplayRole);
    QPair<double, double> data = dataValue.value<QPair<double, double>>();
    
    values.append(QJsonObject {
      {"read", data.first},
      {"write", data.second}
    });
  }
  json["sensor"] = "DiskSensor";
  json["values"] = values;
  return json;
}

void DiskSensor::deserialize(const QJsonObject &json) {
  QJsonArray data = json["values"].toArray();
  for (auto value : data) {
    QJsonObject obj = value.toObject();
    QVariant dataValue = QVariant::fromValue(QPair<double, double>(obj["read"].toDouble(), obj["write"].toDouble()));
    addData(dataValue);
  }
}
