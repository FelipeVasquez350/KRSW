#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication> 
#include <QMainWindow>
#include <QWidget>
#include <QGridLayout>
#include <QMenuBar>
#include <QFileDialog> 
#include <QMessageBox> 
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument> 
#include <QtCharts/QChart>

#include "add_dialog.h"
#include "edit_dialog.h"
#include "info_dialog.h"
#include "widgets/toolbar.h"
#include "widgets/searchbar.h"
#include "widgets/chartdock.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  QMainWindow *innerWindow;
  QList<ChartDock*> chartDocks;
  QMenuBar *menuBar;

  void createMenu();
  void createBars();
  void addDock(ChartDock* dock);
  void removeDock(ChartDock* dock);
  void updateDocks();

private slots:
  void load();
  void save();
  void clear();
  void exit();
  void add();  
  void edit();
  void search(const QString &text, const QStringList &filters);
  void remove(ChartDock *dock);
  void play();
  void stop();
  void info();
};
#endif