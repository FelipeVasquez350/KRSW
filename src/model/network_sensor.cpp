#include "network_sensor.h"

static bool isRegistered = [] {
  SensorFactory::registerSensorType(NetworkSensor::staticMetaObject);
  return true;
}();

std::string NetworkSensor::getMainNetworkInterface() const {
  struct ifaddrs *ifaddr, *ifa;
  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    return "";
  }

  std::string mainInterface;
  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL) {
      continue;
    }  
    
    if (ifa->ifa_flags & IFF_UP && ifa->ifa_flags & IFF_RUNNING && !(ifa->ifa_flags & IFF_LOOPBACK)) {
      mainInterface = ifa->ifa_name;
      break;
    }
  }

  freeifaddrs(ifaddr);
  return mainInterface;
}

std::pair<double, double> NetworkSensor::getNetworkUsage(const std::string& interface) const {
  unsigned long int receivedBytes1, transmittedBytes1;
  unsigned long int receivedBytes2, transmittedBytes2;
  std::ifstream procNetDev1 = std::ifstream("/proc/net/dev");
  std::string line;

  while(std::getline(procNetDev1, line)) {
    if(line.find(interface) != std::string::npos) {
      std::sscanf(line.c_str(), "%*s %lu %*d %*d %*d %*d %*d %*d %*d %lu", &receivedBytes1, &transmittedBytes1);
      break;
    }
  }
  procNetDev1.close();

  std::this_thread::sleep_for(std::chrono::milliseconds(350));

  std::ifstream procNetDev2 = std::ifstream("/proc/net/dev");
  while(std::getline(procNetDev2, line)) {
    if(line.find(interface) != std::string::npos) {
      std::sscanf(line.c_str(), "%*s %lu %*d %*d %*d %*d %*d %*d %*d %lu", &receivedBytes2, &transmittedBytes2);
      break;
    }
  }
  procNetDev2.close();

  return std::pair<double, double>(static_cast<double>(receivedBytes2 - receivedBytes1)*2 , static_cast<double>(transmittedBytes2 - transmittedBytes1)*2);
} 


NetworkSensor::NetworkSensor(QObject *parent) : Sensor(parent), watcher(QFutureWatcher<std::pair<double, double>>()) {
  connect(&watcher, &QFutureWatcher<std::pair<double, double>>::finished, [this]() {
    std::pair<double, double> value = watcher.future().result();
    addData(QVariant::fromValue(value));
    emit valueChanged(ChartDrawType::IncrementalUpdate);
  });
}

NetworkSensor::NetworkSensor(const NetworkSensor &other) : Sensor(other), watcher(QFutureWatcher<std::pair<double, double>>()) {
  connect(&watcher, &QFutureWatcher<std::pair<double, double>>::finished, [this]() {
    std::pair<double, double> value = watcher.future().result();
    addData(QVariant::fromValue(value));
    emit valueChanged(ChartDrawType::IncrementalUpdate);
  });
}

NetworkSensor* NetworkSensor::clone() const {
  return new NetworkSensor(*this);
}
NetworkSensor::~NetworkSensor() {
  disconnect(&watcher, &QFutureWatcher<std::pair<double, double>>::finished, nullptr, nullptr);
}

void NetworkSensor::retrieveData() {
  std::string interface = getMainNetworkInterface();
  watcher.setFuture(QtConcurrent::run(&NetworkSensor::getNetworkUsage, this, interface));
}

QMetaType::Type NetworkSensor::getQVariantType() const {
  return QMetaType::QVariant;
}

bool NetworkSensor::isReadingRelative() const {
  return false;
}

QVector<QString> NetworkSensor::getReadingLables() const {
  return {"Download", "Upload"};
}

std::pair<double, QString>  NetworkSensor::getScaleUnit(double value) const {
  const char* units[] = {"B/s", "KiB/s", "MiB/s", "GiB/s"};
  int unitIndex = 0;
  double formattedSpeed = value;
  while (formattedSpeed >= 1024 && unitIndex < 3) {
    formattedSpeed /= 1024;
    unitIndex++;
  }
  return {std::pow(1024,unitIndex), units[unitIndex]};
}

QJsonObject NetworkSensor::serialize() const {
  QJsonObject json;
  QJsonArray values;

  int row = rowCount();
  for (int i = 0; i < row; i++) {
    QModelIndex index = this->index(i);
    QVariant dataValue = this->data(index, Qt::DisplayRole);
    QPair<double, double> data = dataValue.value<QPair<double, double>>();
    
    values.append(QJsonObject {
      {"download", data.first},
      {"upload", data.second}
    });
  }

  json["sensor"] = "NetworkSensor";
  json["values"] = values;
  return json;
}

void NetworkSensor::deserialize(const QJsonObject &json) {
  QJsonArray data = json["values"].toArray();
  for (auto value : data) {
     QJsonObject obj = value.toObject();
    QVariant dataValue = QVariant::fromValue(QPair<double, double>(obj["download"].toDouble(), obj["upload"].toDouble()));
    addData(dataValue);
  }
}