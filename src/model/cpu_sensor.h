#ifndef CPU_SENSOR_H
#define CPU_SENSOR_H

#include <fstream>
#include <thread>

#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

#include "sensor.h"
#include "utils/sensor_factory.h"

class CPUSensor : public Sensor {
  Q_OBJECT
  
private:
  QFutureWatcher<double> watcher;
  void getCPUTimes(std::vector<size_t>& times) const;
  double calculateCPUUsage() const;

public:
  Q_INVOKABLE CPUSensor(QObject *parent = nullptr);
  CPUSensor(const CPUSensor &other);
  ~CPUSensor() override;
  CPUSensor* clone() const override;

  QMetaType::Type getQVariantType() const override;
    
  bool isReadingRelative() const override;  
  QVector<QString> getReadingLables() const override;
  std::pair<double, QString>  getScaleUnit(double value) const override;

  QJsonObject serialize() const override;
  void deserialize(const QJsonObject &json) override;
  
private slots:
  void retrieveData() override;
};

#endif // CPU_SENSOR_H