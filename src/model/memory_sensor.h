 #ifndef MEMORY_SENSOR_H
#define MEMORY_SENSOR_H

#include <fstream>
#include <sys/sysinfo.h>

#include "sensor.h"
#include "utils/sensor_factory.h"

class MemorySensor : public Sensor {
  Q_OBJECT
private:
  double getMemoryUsage() const;
  double getSwapUsage() const;

public:
  Q_INVOKABLE MemorySensor(QObject *parent = nullptr);
  MemorySensor(const MemorySensor &other);
  MemorySensor* clone() const override;
  ~MemorySensor() override;

  QMetaType::Type getQVariantType() const override;
  bool isReadingRelative() const override;  
  QVector<QString> getReadingLables() const override;
  std::pair<double, QString> getScaleUnit(double value) const override;

  QJsonObject serialize() const override;
  void deserialize(const QJsonObject &json) override;

private slots:
  void retrieveData();
};

#endif // MEMORY_SENSOR_H