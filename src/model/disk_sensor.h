#ifndef DISK_SENSOR_H
#define DISK_SENSOR_H

#include <fstream>
#include <thread>

#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

#include "sensor.h"
#include "utils/sensor_factory.h"

class DiskSensor : public Sensor {
  Q_OBJECT
private:
  QFutureWatcher<std::pair<double, double>> watcher;
  int getSectorSize(const std::string& disk) const;
  std::string getPrimaryDrive() const;
  std::pair<long long, long long> getDiskUsage(const std::string& disk) const;
  std::pair<double, double> getReadWriteSpeeds(const std::string& disk) const;

public:
  Q_INVOKABLE DiskSensor(QObject *parent = nullptr);
  DiskSensor(const DiskSensor &other);
  DiskSensor* clone() const override;
  ~DiskSensor() override;
 
  QMetaType::Type getQVariantType() const override;
  bool isReadingRelative() const override;
  QVector<QString> getReadingLables() const override;
  std::pair<double, QString>  getScaleUnit(double value) const override;

  QJsonObject serialize() const override;
  void deserialize(const QJsonObject &json) override;

private slots:
  void retrieveData() override;
};  

#endif // DISK_SENSOR_H

