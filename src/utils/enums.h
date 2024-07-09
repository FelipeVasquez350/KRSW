#ifndef ENUMS_H
#define ENUMS_H

enum SensorState {
  Stopped,
  Started,
  Simulating,
  PausedSimulation
};

enum ChartDrawType {
  FullRedraw,
  IncrementalUpdate,
  SimulationMode
};

#endif // ENUMS_H