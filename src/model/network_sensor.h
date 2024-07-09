#ifndef NETWORK_SENSOR_H
#define NETWORK_SENSOR_H

#include <ifaddrs.h>
#include <net/if.h>
#include <fstream>
#include <thread>

#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

#include "sensor.h"
#include "utils/sensor_factory.h"

class NetworkSensor : public Sensor {
  Q_OBJECT

private:
  QFutureWatcher<std::pair<double, double>> watcher;
  std::string getMainNetworkInterface() const;
  std::pair<double, double> getNetworkUsage(const std::string& interface) const;

public:
  Q_INVOKABLE NetworkSensor(QObject *parent = nullptr);
  NetworkSensor(const NetworkSensor &other);
  NetworkSensor* clone() const override;
  ~NetworkSensor() override;

  QMetaType::Type getQVariantType() const override;
  bool isReadingRelative() const override;  
  QVector<QString> getReadingLables() const override;
  std::pair<double, QString>  getScaleUnit(double value) const override;

  QJsonObject serialize() const override;
  void deserialize(const QJsonObject &json) override;

private slots:
  void retrieveData();
};

#endif // NETWORK_SENSOR_H