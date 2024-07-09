#include "sensor.h"

Sensor::Sensor(QObject *parent) : QAbstractListModel(parent),  timer(new QTimer(this)), state(Stopped){
  connect(timer, &QTimer::timeout, this, &Sensor::retrieveData);
}

Sensor::Sensor(const Sensor &other) : QAbstractListModel(other.parent()), values(other.values), timer(new QTimer(this)), state(other.state){
  connect(timer, &QTimer::timeout, this, &Sensor::retrieveData);
}

Sensor::~Sensor() {
  disconnect(timer, &QTimer::timeout, this, &Sensor::retrieveData);
  delete timer;
}

void Sensor::start() {
  if (state != Stopped) {
    return;
  }
  timer->start(700);
  setState(Started);
}

void Sensor::stop() {
  if (state != Started) {
    return;
  }
  timer->stop();
  setState(Stopped);
}

void Sensor::startSimulation() {
  if (state == Simulating) {
    return;
  }
  timer->stop();
  emit valueChanged(ChartDrawType::SimulationMode);
  setState(Simulating);
}

void Sensor::pauseSimulation() {
  if (state != Simulating) {
    return;
  }
  setState(PausedSimulation);
}

SensorState Sensor::getState() const {
  return state;
}

void Sensor::setState(SensorState state) {
  this->state = state;
  emit stateChanged(state);
}

int Sensor::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return values.size();
}

QVariant Sensor::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= values.size()) {
    return QVariant();
  }
  if (role == Qt::DisplayRole) {
    return values[index.row()];
  }
  return QVariant();
}

void Sensor::addData(const QVariant &value) {
  beginInsertRows(QModelIndex(), values.size(), values.size());
  values.push_back(value);
  endInsertRows();
}

bool Sensor::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (role != Qt::EditRole) {
    return false;
  }
  if (index.isValid() && index.row() <= values.size()) {
    if (index.row() == values.size()) {
      values.push_back(value);
    } else {
      values[index.row()] = value;
    }
    emit dataChanged(index, index, {role});
    return true;
  }
  return false;
}

bool Sensor::removeData(const QModelIndex &index) {
  if (index.isValid() && index.row() < values.size()) {
    beginRemoveRows(QModelIndex(), index.row(), index.row());
    values.remove(index.row());
    endRemoveRows();
    return true;
  }
  return false;
}

void Sensor::retrieveData() {
  emit valueChanged(ChartDrawType::IncrementalUpdate);
}

