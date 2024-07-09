#include "memory_sensor.h"

static bool isRegistered = [] {
  SensorFactory::registerSensorType(MemorySensor::staticMetaObject);
  return true;
}();

double MemorySensor::getMemoryUsage() const {
  std::ifstream procMemInfo("/proc/meminfo");
  std::string line;
  int memTotal = -1;
  int memAvailable = -1;

  while(std::getline(procMemInfo, line)) {
    if(line.find("MemTotal:") == 0) {
      std::sscanf(line.c_str(), "MemTotal:        %d kB", &memTotal);
    } else if(line.find("MemAvailable:") == 0) {
      std::sscanf(line.c_str(), "MemAvailable:        %d kB", &memAvailable);
    }
    if(memTotal != -1 && memAvailable != -1) {
      break;
    }
  }

  if(memTotal == -1 || memAvailable == -1) {
    return -1;
  }

  return 100.0 * (memTotal - memAvailable) / memTotal;
}

double MemorySensor::getSwapUsage() const {
  std::ifstream procMemInfo("/proc/meminfo");
  std::string line;
  int swapTotal = -1;
  int swapFree = -1;

  while(std::getline(procMemInfo, line)) {
    if(line.find("SwapTotal:") == 0) {
      std::sscanf(line.c_str(), "SwapTotal:        %d kB", &swapTotal);
    } else if(line.find("SwapFree:") == 0) {
      std::sscanf(line.c_str(), "SwapFree:        %d kB", &swapFree);
    }
    if(swapTotal != -1 && swapFree != -1) {
      break;
    }
  }

  if(swapTotal == -1 || swapFree == -1) {
    return -1;
  }

  return 100.0 * (swapTotal - swapFree) / swapTotal;
}


MemorySensor::MemorySensor(QObject *parent) : Sensor(parent) {
};

MemorySensor::MemorySensor(const MemorySensor &other) : Sensor(other) {
};

MemorySensor* MemorySensor::clone() const {
  return new MemorySensor(*this);
};

MemorySensor::~MemorySensor() {
};

void MemorySensor::retrieveData() {
  addData(QVariant::fromValue(QPair<double, double>(getSwapUsage(),getMemoryUsage())));
  emit valueChanged(ChartDrawType::IncrementalUpdate);
};

QMetaType::Type MemorySensor::getQVariantType() const {
  return QMetaType::QVariantPair;
}

bool MemorySensor::isReadingRelative() const {
  return true;
}

QVector<QString> MemorySensor::getReadingLables() const {
  return {"Swap Usage","Memory Usage"};
}

std::pair<double, QString> MemorySensor::getScaleUnit(double) const {
  return {1, "%"};
}

QJsonObject MemorySensor::serialize() const {
  QJsonObject json;
  QJsonArray values;

  int row = rowCount();
  for (int i = 0; i < row; i++) {
    QModelIndex index = this->index(i);
    QVariant dataValue = this->data(index, Qt::DisplayRole);
    QPair<double, double> data = dataValue.value<QPair<double, double>>();
    
    values.append(QJsonObject {
      {"swap", data.first},
      {"usage", data.second}
    });
  }
  json["sensor"] = "MemorySensor";
  json["values"] = values;
  return json;
}

void MemorySensor::deserialize(const QJsonObject &json) {
  QJsonArray data = json["values"].toArray();
  for (auto value : data) {
    QJsonObject obj = value.toObject();
    QVariant dataValue = QVariant::fromValue(QPair<double, double>(obj["swap"].toDouble(), obj["usage"].toDouble()));
    addData(dataValue);
  }
}