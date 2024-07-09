#ifndef SENSOR_FACTORY_H
#define SENSOR_FACTORY_H

#include <QMap>
#include <QMetaObject>
#include <QString>

#include "sensor.h"

class SensorFactory {
public:
  static Sensor* createSensor(QString sensorType, QObject *parent = nullptr);

  static void registerSensorType(const QMetaObject &metaObject);

  static QStringList getSensorTypes();

private:
  SensorFactory() = default;
  SensorFactory(const SensorFactory&) = delete;
  SensorFactory& operator=(const SensorFactory&) = delete;

  static QMap<QString, QMetaObject> sensorConstructors;
};

#endif // SENSOR_FACTORY_H