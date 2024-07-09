#include "sensor_factory.h"


QMap<QString, QMetaObject> SensorFactory::sensorConstructors;

Sensor* SensorFactory::createSensor(QString sensorType,  QObject *parent) {
  auto it = sensorConstructors.find(sensorType);
  if (it != sensorConstructors.end()) {
    QObject *obj = it.value().newInstance(Q_ARG(QObject*, parent));
    return qobject_cast<Sensor*>(obj);
  } else {
    return nullptr;
  }
}

void SensorFactory::registerSensorType(const QMetaObject &metaObject) {
  sensorConstructors[metaObject.className()] = metaObject;
}

QStringList SensorFactory::getSensorTypes() {
  QStringList types;
  for (auto it = sensorConstructors.begin(); it != sensorConstructors.end(); ++it) {
    types.append(it.key());
  }
  return types;
}
