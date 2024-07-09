#include "main_window.h"

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent), menuBar(new QMenuBar()){

  setWindowIcon(QIcon(":/KRSW.png"));
  setWindowTitle("KRSW");
  resize(1600, 950); 
  
  innerWindow = new QMainWindow(this); 
  innerWindow->setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks);
  innerWindow->setStyleSheet("QMainWindow::separator {background-color: grey; width: 1px; border: none;}");
  createMenu(); 
  createBars();

  QScrollArea *scrollArea = new QScrollArea(this);
  scrollArea->setWidget(innerWindow);
  scrollArea->setWidgetResizable(true);
  setCentralWidget(scrollArea);
}

MainWindow::~MainWindow() {
  for (auto dock : chartDocks) {
    delete dock;
  }
  delete menuBar;
  delete innerWindow;
}

void MainWindow::createMenu() {
  QMenu *fileMenu = new QMenu(tr("&File"), this);
  QAction *loadAction = fileMenu->addAction(tr("&Load"));
  QAction *saveAction = fileMenu->addAction(tr("&Save"));
  QAction *clearAction = fileMenu->addAction(tr("&Clear"));
  QAction *exitAction = fileMenu->addAction(tr("&Exit"));
  loadAction->setShortcut(QKeySequence::Open);
  saveAction->setShortcut(QKeySequence::Save);
  clearAction->setShortcut(QKeySequence("Ctrl+Shift+C"));
  exitAction->setShortcut(QKeySequence::Quit);
  menuBar->addMenu(fileMenu);

  QMenu *viewMenu = new QMenu(tr("&Help"), this);
  QAction *infoAction = viewMenu->addAction(tr("&Info"));
  infoAction->setShortcut(QKeySequence::HelpContents);
  menuBar->addMenu(viewMenu);

  connect(loadAction, &QAction::triggered, this, &MainWindow::load);
  connect(saveAction, &QAction::triggered, this, &MainWindow::save);
  connect(clearAction, &QAction::triggered, this, &MainWindow::clear);
  connect(exitAction, &QAction::triggered, this, &MainWindow::exit);
  connect(infoAction, &QAction::triggered, this, &MainWindow::info);

  setMenuBar(menuBar);
}

void MainWindow::createBars()  {
  ToolBar *toolbar = new ToolBar(this);
  SearchBar *searchbar = new SearchBar(this);

  addToolBar(Qt::LeftToolBarArea, toolbar);
  addToolBar(Qt::TopToolBarArea, searchbar);

  connect(toolbar, &ToolBar::add, this, &MainWindow::add);
  connect(searchbar, &SearchBar::search, this, &MainWindow::search);
  connect(toolbar, &ToolBar::edit, this, &MainWindow::edit);
  connect(toolbar, &ToolBar::play, this, &MainWindow::play);
  connect(toolbar, &ToolBar::pause, this, &MainWindow::stop);
}

void MainWindow::addDock(ChartDock* dock) {    
  if (dock == nullptr) {
    return;
  }
  chartDocks.append(dock);
  connect(dock, &ChartDock::removeBeforeDeletion, this, &MainWindow::remove);
}

void MainWindow::removeDock(ChartDock* dock) {
  int index = chartDocks.indexOf(dock);

  if (index != -1) {
    ChartDock *chartDock = chartDocks.takeAt(index);
    removeDockWidget(chartDock);
    disconnect(chartDock, &ChartDock::removeBeforeDeletion, this, &MainWindow::remove);
    delete chartDock;
  }
}

void MainWindow::updateDocks() {
  if (chartDocks.empty()) {
    return;  
  }
    QList<QDockWidget*> dockWidgets;
    for (ChartDock* dock : chartDocks) {
        dockWidgets.append(dock);
    }
    for (QDockWidget* dockWidget : dockWidgets) {
        innerWindow->removeDockWidget(dockWidget);
    }
    int totalWidth = innerWindow->width();
    int dockWidth = totalWidth / 2; 
    QList<int> sizes;
    for (int i = 0; i < dockWidgets.size(); ++i) {
        innerWindow->addDockWidget(i % 2 == 0 ? Qt::LeftDockWidgetArea : Qt::RightDockWidgetArea, dockWidgets[i]);
        dockWidgets[i]->show();    
        sizes.append(dockWidth);
    }
    if (dockWidgets.size() > 1) {
        innerWindow->resizeDocks(dockWidgets, sizes, Qt::Horizontal);
    }
}

void MainWindow::load() {
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"));
  if (!fileName.isEmpty()) {
    if (chartDocks.size() != 0 && QMessageBox::warning(this, "Warning", "Opening another file will delete the current charts. Do you want to continue?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
      return;
    }

    clear();

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
      QMessageBox::information(this, "Error", "Unable to open file");
      return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));

    if (doc.isNull()) {
      qWarning("Failed to create JSON doc");
      QMessageBox::information(this, "Read Error", "Failed to read file contents");
      return;
    }
    if (doc.isObject()) {
      QJsonObject obj = doc.object();
      if (obj.isEmpty()) {
        qWarning("JSON object is empty");
        QMessageBox::information(this, "Warning", "JSON object inserted is empty, ignoring...");
        return;
      }

      QJsonArray arr = obj["sensors"].toArray();
      if (arr.isEmpty()) {
        qWarning("JSON array is empty");
        QMessageBox::information(this, "Parsing Error", "Sensor data is empty or not recognized in file, ignoring...");
        return;
      }

      for (const auto& iter : arr) {
        QJsonObject object = iter.toObject();
        if (object.isEmpty()) {
          qWarning("JSON object is empty");
          QMessageBox::information(this, "Parsing Error", "Sensor data is empty or not recognized in file, skipping...");
          continue;
        }

        QString sensorType = object["sensor"].toString();
        if (sensorType.isEmpty()) {
          qWarning("Sensor does not have a type");
          QMessageBox::information(this, "Parsing Error", "Sensor type is not set in file, skipping...");
          continue;
        }

        Sensor* sensor = SensorFactory::createSensor(sensorType, this);
        if (sensor == nullptr) {
          qWarning("Sensor type is not registered");
          QMessageBox::information(this, "Parsing Error", tr("Sensor type %1 is not registered, skipping...").arg(sensorType));
          continue;
        }
        sensor->deserialize(object);

        QString chartType = object["chart"].toString();
        QString chartTitle = object["title"].toString();
        if (chartType.isEmpty()) {
          qWarning("JSON object does not have a chart");
          QMessageBox::information(this, "Parsing Error", "Chart type is not set in file, skipping...");
          continue;
        }
        Chart* chart = ChartFactory::createChart(chartType, chartTitle, sensor, this);
        if (chart == nullptr) {
          qWarning("Chart type is not registered");
          QMessageBox::information(this, "Parsing Error", tr("Chart type %1 is not registered, skipping...").arg(chartType));
          continue;
        }
        ChartDock* dock = new ChartDock(chart, sensor, this);
        addDock(dock);
      }

      updateDocks();
    }
    else {
      qWarning("JSON is not an object");
      QMessageBox::information(this, "Parsing Error", "File is not a JSON Object, ignoring...");
      return;
    }
  }
}

void MainWindow::save() {
  QString fileName = QFileDialog::getSaveFileName(this, "Save File");
  if (!fileName.isEmpty()) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
      QMessageBox::information(this, "I/O Error", "Unable to open file");
      return;
    }

    QJsonArray sensorsArray;
    for (ChartDock* dock : chartDocks) {
      Chart* chart = dock->getChart();
      Sensor* sensor = dock->getSensor();

      QJsonObject sensorObject;

      sensorObject["chart"] = chart->metaObject()->className();
      sensorObject["title"] = chart->getTitle();

      QJsonObject sensorSerialized = sensor->serialize();
      sensorObject["sensor"] = sensorSerialized.value("sensor");
      sensorObject["values"] = sensorSerialized.value("values");

      sensorsArray.append(sensorObject);
    }

    QJsonObject obj;
    obj["sensors"] = sensorsArray;
  
    QJsonDocument doc(obj);
    file.write(doc.toJson());
  }
}

void MainWindow::clear() {
  for (auto dock : chartDocks) {
    removeDock(dock);
  }
  chartDocks.clear();
}

void MainWindow::exit() {
  QApplication::quit();
}

void MainWindow::add() {  
  AddDialog *dialog = new AddDialog(this);
    
  if (dialog->exec() == QDialog::Accepted) {
    QString chartTitle = dialog->getChartTitle();
    QString sensor = dialog->getSensor();
    QString chart = dialog->getChart();
    bool active = dialog->isActive();
    QVector<QVariant> values = dialog->getValues();
    
    Sensor* sensorPtr = SensorFactory::createSensor(sensor, this);
    Chart* chartPtr = ChartFactory::createChart(chart, chartTitle, sensorPtr, this);

    for (auto value : values) {
      sensorPtr->addData(value);
    }
    chartPtr->updateChart(ChartDrawType::FullRedraw);
    if(active)
      sensorPtr->start();

    ChartDock* dock = new ChartDock(chartPtr, sensorPtr, this);
    addDock(dock);
    updateDocks();
  }
}

void MainWindow::edit() {
  QList<bool> pausedSensors;
  QList<ChartDock*> chartDocksCopy;

  for (auto dock : chartDocks) {
    pausedSensors.push_back(dock->getSensor()->getState() == SensorState::Started);
    if (dock->getSensor()->getState() == SensorState::Started)
      dock->getSensor()->stop();
    else if (dock->getSensor()->getState() == SensorState::Simulating)
      dock->getSensor()->pauseSimulation();

    chartDocksCopy.push_back(dock->clone());
  }

  EditDialog *dialog = new EditDialog(chartDocksCopy, pausedSensors, this);

  if (dialog->exec() == QDialog::Accepted) {
    QList<ChartDock*> modifiedDocks = dialog->getModifiedDocks();
    clear();
    for (ChartDock* dock : modifiedDocks) {
      addDock(dock);
    }
    updateDocks();
    return;
  }
    
  for (auto dock : chartDocks) {
    if (dock->getSensor()->getState() == SensorState::PausedSimulation)
      dock->getSensor()->setState(SensorState::Simulating);
    else if (pausedSensors.front()) {
      dock->getSensor()->start();
    } 
    pausedSensors.pop_front();
  }
}

void MainWindow::search(const QString &text, const QStringList &filters) {
  auto simpleFuzzySearch = [](const QString &text, const QString &pattern) {
    if (pattern.isEmpty()) {
      return true;
    }

    int patternIndex = 0;
    for (int i = 0; i < text.length(); ++i) {
      if (text[i].toLower() == pattern[patternIndex].toLower()) {
        ++patternIndex;
        if (patternIndex == pattern.length()) {
          return true;
        }
      }
    }

    return false;
  };

  for (ChartDock* dock : chartDocks) {
    int index = chartDocks.indexOf(dock);
    if (simpleFuzzySearch(dock->getChart()->getTitle(), text) || filters.contains(dock->getSensor()->metaObject()->className()) || filters.contains(dock->getChart()->metaObject()->className())) {
      chartDocks[index]->setVisible(true);
    } else { 
      chartDocks[index]->setVisible(false);
    }
  }
}

void MainWindow::remove(ChartDock *dock) {
  removeDock(dock);
  updateDocks();
}

void MainWindow::play() {
  for (auto dock : chartDocks) {
    if (dock->getSensor()->getState() == SensorState::Stopped)
      dock->getSensor()->start();
  }
}

void MainWindow::stop() {
  for (auto dock : chartDocks) {
    dock->getSensor()->stop();
  }
}

void MainWindow::info() {
  InfoDialog infoDialog;
  infoDialog.exec();
}
