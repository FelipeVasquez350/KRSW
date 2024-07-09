#ifndef SENSOR_H
#define SENSOR_H

#include <QTimer>
#include <QAbstractListModel>

#include <QJsonObject>
#include <QJsonArray>
#include "enums.h"

class Sensor : public QAbstractListModel {
  Q_OBJECT
protected:
  QVector<QVariant> values;
  QTimer* timer;
  SensorState state;

public:
  Sensor(QObject *parent = nullptr);
  Sensor(const Sensor &other);
  virtual Sensor* clone() const = 0;
  virtual ~Sensor() override;

  virtual void start();
  virtual void stop();
  virtual void startSimulation();
  virtual void pauseSimulation();
  virtual SensorState getState() const;
  virtual void setState(SensorState state);

  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  virtual QMetaType::Type getQVariantType() const = 0;

  virtual void addData(const QVariant &value);
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
  virtual bool removeData(const QModelIndex &index);

  virtual bool isReadingRelative() const = 0;
  virtual QVector<QString> getReadingLables() const = 0;
  virtual std::pair<double, QString> getScaleUnit(double value) const = 0;

  virtual QJsonObject serialize() const = 0;
  virtual void deserialize(const QJsonObject &json) = 0;

signals:
  void valueChanged(ChartDrawType type);
  void stateChanged(SensorState state);
   
private slots:
  virtual void retrieveData();
};

#endif // SENSOR_H