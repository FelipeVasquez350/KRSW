#include "edit_dialog.h"

EditDialog::EditDialog(QList<ChartDock*> docks, QList<bool> pausedSensors, QWidget *parent) : QDialog(parent), docksCopy(docks), pausedSensors(pausedSensors){
  setWindowTitle("Edit Charts");
  setMinimumHeight(600);
  setMinimumWidth(600);
  QSplitter *splitter = new QSplitter(this);
  QListWidget *listWidget = new QListWidget(this);
  
  splitter->addWidget(listWidget);

  QWidget *container = new QWidget(this);
  QLabel *titleLabel = new QLabel("Title:");
  QLabel *sensorLabel = new QLabel("Sensor:");
  QLabel *chartLabel = new QLabel("Chart:");
  QLabel *activeLabel = new QLabel("Active:");

  splitter->addWidget(container);  

  splitter->setSizes(QList<int>() << 100 << 0);

  QStringList sensorTypes = SensorFactory::getSensorTypes();
  QStringList chartTypes = ChartFactory::getChartTypes();

  buildUI();

  for (int i=0; i < docks.size(); i++) {
    auto item = new QListWidgetItem(listWidget);
    auto widget = new QWidget();
    auto layout = new QVBoxLayout(widget);
    auto title = new QLabel(docks[i]->getChart()->getTitle());
    layout->addWidget(title);
    widget->setLayout(layout);
    item->setSizeHint(widget->sizeHint());
    listWidget->setItemWidget(item, widget);
  }

  connect(listWidget, &QListWidget::itemSelectionChanged, [this, listWidget, container, titleLabel, sensorLabel, chartLabel, activeLabel, splitter]() {
    if (container->layout() != nullptr) {
      delete container->layout();
    }

    int currentIndex = listWidget->currentRow();
    if (currentIndex >= 0 && currentIndex < docksCopy.size()) {
      QWidget *widget = new QWidget(this);
      QVBoxLayout *layout = new QVBoxLayout(widget);
      QGroupBox *groupBox = new QGroupBox("Data:", this);
      QVBoxLayout *groupBoxLayout = new QVBoxLayout(groupBox);
      QPushButton *addRowButton = new QPushButton("Add Row");
      QPushButton *removeRowButton = new QPushButton("Remove Row");

      layout->addWidget(titleLabel);
      layout->addWidget(lineEditList[currentIndex]);
      layout->addWidget(sensorLabel);
      layout->addWidget(comboBoxList[currentIndex].first);
      layout->addWidget(chartLabel);
      layout->addWidget(comboBoxList[currentIndex].second);
      layout->addWidget(activeLabel);
      layout->addWidget(checkBoxList[currentIndex]);        
      groupBoxLayout->addWidget(tableList[currentIndex]);
      groupBoxLayout->addWidget(addRowButton);
      groupBoxLayout->addWidget(removeRowButton);
      layout->addWidget(groupBox);

      connect(comboBoxList[currentIndex].first, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, currentIndex](int) {
        updateTable(currentIndex);
      });
      connect(comboBoxList[currentIndex].second, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, currentIndex](int) {
        updateTable(currentIndex);
      });
      connect(addRowButton, &QPushButton::clicked, [this, currentIndex]() {
        addRow(tableList.at(currentIndex), currentIndex);
      });
      connect(removeRowButton, &QPushButton::clicked, [this, currentIndex]() {
        removeSelectedRow(tableList.at(currentIndex));
      });

      container->setLayout(layout);
      splitter->setSizes(QList<int>() << 30 << 180);
    }
  });
       
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Close, this);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(splitter);
  mainLayout->addWidget(buttonBox);
  connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &EditDialog::applyChanges);
  connect(buttonBox->button(QDialogButtonBox::Close), &QPushButton::clicked, this, &EditDialog::close);
  setLayout(mainLayout);
}

void EditDialog::buildUI() {
  QLineEdit *lineEdit;
  QComboBox *sensorComboBox;
  QComboBox *chartComboBox;    
  QCheckBox *checkBox;
  QTableWidget *tableWidget;

  for (int i = 0; i < docksCopy.size(); ++i) {
    lineEdit = new QLineEdit(docksCopy.at(i)->getChart()->getTitle());
    lineEditList.append(lineEdit);

    sensorComboBox = new QComboBox();
    sensorComboBox->addItems(SensorFactory::getSensorTypes());
    sensorComboBox->setCurrentText(docksCopy.at(i)->getSensor()->metaObject()->className());

    chartComboBox = new QComboBox();
    chartComboBox->addItems(ChartFactory::getChartTypes());
    chartComboBox->setCurrentText(docksCopy.at(i)->getChart()->metaObject()->className());    
    
    comboBoxList.append(QPair<QComboBox*, QComboBox*>(sensorComboBox, chartComboBox));

    checkBox = new QCheckBox("Active");
    checkBox->setChecked(pausedSensors.at(i));
    checkBoxList.append(checkBox);

    tableWidget = new QTableWidget();
    tableWidget->setColumnCount(2);
    tableWidget->setHorizontalHeaderLabels(docksCopy.at(i)->getSensor()->getReadingLables());
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tableList.append(tableWidget);
    updateTable(i);

    int row = docksCopy.at(i)->getSensor()->rowCount();
    for (int j = 0; j < row; j++) {
      QModelIndex index = docksCopy.at(i)->getSensor()->index(j, 0);
      QVariant data = docksCopy.at(i)->getSensor()->data(index, Qt::DisplayRole);
      tableWidget->insertRow(j);
      if (data.userType() == QMetaType::Double){
        QLineEdit *lineEdit = new QLineEdit(this);
        QDoubleValidator *validator = new QDoubleValidator(lineEdit);
        lineEdit->setValidator(validator);
        lineEdit->setText(data.toString());
        connect(lineEdit, &QLineEdit::textChanged, [this, j](const QString &text) { checkInput(text, j); });
        tableWidget->setCellWidget(j, 0, lineEdit);
     } else {
        QPair<double, double> pair = data.value<QPair<double, double>>();

        QLineEdit *lineEdit1 = new QLineEdit(this);
        QDoubleValidator *validator1 = new QDoubleValidator(lineEdit1);
        lineEdit1->setValidator(validator1);
        lineEdit1->setText(QString::number(pair.first));
        connect(lineEdit1, &QLineEdit::textChanged, [this, j](const QString &text) { checkInput(text, j); });
        tableWidget->setCellWidget(j, 0, lineEdit1);

        QLineEdit *lineEdit2 = new QLineEdit(this);
        QDoubleValidator *validator2 = new QDoubleValidator(lineEdit2);
        lineEdit2->setValidator(validator2);
        lineEdit2->setText(QString::number(pair.second));
        connect(lineEdit2, &QLineEdit::textChanged, [this, j](const QString &text) { checkInput(text, j); });
        tableWidget->setCellWidget(j, 1, lineEdit2);
      }
    }
  }
}

void EditDialog::addRow(QTableWidget* tableWidget, int index) {
  int row = tableWidget->rowCount();
  tableWidget->insertRow(row);

  for (int column = 0; column < tableWidget->columnCount(); ++column) {
    QLineEdit *lineEdit = new QLineEdit(this);
    QDoubleValidator *validator = new QDoubleValidator(lineEdit);
    
    Sensor* tmpSensor = SensorFactory::createSensor(comboBoxList.at(index).first->currentText());
    if (tmpSensor->isReadingRelative()) {
        validator = new QDoubleValidator(0, 100, 2, lineEdit);
      } else {
        validator = new QDoubleValidator(lineEdit);
    }
    lineEdit->setValidator(validator);
    connect(lineEdit, &QLineEdit::textChanged, [this, index](const QString &text) { checkInput(text, index); });    tableWidget->setCellWidget(row, column, lineEdit);
    tableWidget->setCellWidget(row, column, lineEdit);
    delete tmpSensor;
  }
}

void EditDialog::checkInput(const QString &text, int index) {
  QLineEdit *lineEdit = qobject_cast<QLineEdit*>(sender());
  if (lineEdit) {
    bool ok;
    double value = text.toDouble(&ok);
      Sensor* tmpSensor = SensorFactory::createSensor(comboBoxList.at(index).first->currentText());
      if ((!ok || value < 0 || value > 100) && (tmpSensor->isReadingRelative())) {
        lineEdit->undo();
      delete tmpSensor;
    }
  }
}

void EditDialog::updateTable(int index) {
  QPair<QComboBox*, QComboBox*> comboBoxPair = comboBoxList.at(index);
  QString sensor = comboBoxPair.first->currentText();
  QString chart = comboBoxPair.second->currentText();

  Sensor* tmpSensor = SensorFactory::createSensor(sensor);
  QStringList label = tmpSensor->getReadingLables();
  
  if (tmpSensor->getQVariantType() == QMetaType::Double) {
    tableList.at(index)->setColumnCount(1);
  } else {
    tableList.at(index)->setColumnCount(2);
  }
  tableList.at(index)->setHorizontalHeaderLabels(label);
  
  delete tmpSensor;  
}

void EditDialog::removeSelectedRow(QTableWidget* tableWidget) {
  if (tableWidget && tableWidget->selectionModel()->hasSelection()) {
    tableWidget->removeRow(tableWidget->currentRow());
  }
}

void EditDialog::applyChanges() {
  for (int i = 0; i < docksCopy.size(); ++i) {
    Sensor* sensor = docksCopy.at(i)->getSensor();
    Chart* chart = docksCopy.at(i)->getChart();
    QTableWidget* table = tableList.at(i);

    if(sensor->metaObject()->className() != comboBoxList[i].first->currentText()) {
      sensor = SensorFactory::createSensor(comboBoxList[i].first->currentText());

      docksCopy.at(i)->setSensor(sensor);
    } 
    else {
      int range = std::max(table->rowCount(), sensor->rowCount());
      for (int j = 0; j < range; ++j) {
        QModelIndex index = sensor->index(j, 0);
        if(j < table->rowCount()) {
          if (sensor->getQVariantType() == QMetaType::Double){
            QLineEdit *lineEdit = qobject_cast<QLineEdit*>(table->cellWidget(j, 0)); 
            if (j < sensor->rowCount())
              sensor->setData(index, lineEdit->text().toDouble());
            else
              sensor->addData(lineEdit->text().toDouble());
          } else if (sensor->getQVariantType() == QMetaType::QVariantPair){
            QLineEdit *item0 = qobject_cast<QLineEdit*>(table->cellWidget(j, 0));
            QLineEdit *item1 = qobject_cast<QLineEdit*>(table->cellWidget(j, 1));
            if (item0 && item1) {
              double value0 = item0->text().toDouble();
              double value1 = item1->text().toDouble();
              if (j < sensor->rowCount())
                sensor->setData(index, QVariant::fromValue(QPair<double, double>(value0, value1)));
              else
                sensor->addData(QVariant::fromValue(QPair<double, double>(value0, value1)));
            }
          }
        } else {
          QModelIndex ind = sensor->index(sensor->rowCount()-1,0);
          sensor->removeData(ind);
        }
      }
    }

    if (chart->metaObject()->className() != comboBoxList[i].second->currentText()) {
      chart = ChartFactory::createChart(comboBoxList[i].second->currentText(), lineEditList[i]->text(), sensor);
      docksCopy.at(i)->setChart(chart);
    }

    chart->setTitle(lineEditList[i]->text());
    chart->updateChart(ChartDrawType::FullRedraw);

    if (checkBoxList.at(i)->isChecked() && sensor->getState() == SensorState::Stopped) {
      sensor->start();  
    } 
    else if (!checkBoxList.at(i)->isChecked() && sensor->getState() == SensorState::PausedSimulation) {
      sensor->setState(SensorState::Stopped);
    }
    else {
      sensor->stop();
    }
  }
  accept();
}

QList<ChartDock*> EditDialog::getModifiedDocks() const {
  return docksCopy;
}