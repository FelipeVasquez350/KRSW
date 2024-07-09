#include "add_dialog.h"

AddDialog::AddDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("Add Chart");
  QVBoxLayout *layout = new QVBoxLayout(this); 
  valuesGroupBox = new QGroupBox("Values", this);
  QVBoxLayout *valuesLayout = new QVBoxLayout(valuesGroupBox);

  chartTitleEdit = new QLineEdit(this);
  sensorComboBox = new QComboBox(this);
  chartComboBox = new QComboBox(this);
  warningLabel = new QLabel(this);
  activeCheckBox = new QCheckBox("Active", this);
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Close, this);

  QStringList sensorTypes = SensorFactory::getSensorTypes();
  QStringList chartTypes = ChartFactory::getChartTypes();
  
  sensorComboBox->addItems(sensorTypes);
  sensorComboBox->setCurrentIndex(-1);
  chartComboBox->addItems(chartTypes);
  chartComboBox->setCurrentIndex(-1);
  warningLabel->setStyleSheet("QLabel { color : orange; }");
  warningLabel->setWordWrap(true);
  warningLabel->hide();

  layout->addWidget(new QLabel("Chart Title:"));
  layout->addWidget(chartTitleEdit);
  layout->addWidget(sensorComboBox);
  layout->addWidget(chartComboBox);
  layout->addWidget(warningLabel);
  layout->addWidget(activeCheckBox);

  valuesGroupBox->hide();
  valuesLayout->addWidget(new QLabel("Sensor Values:"));
  tableWidget = new QTableWidget(0, 1, this);
  tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  QPushButton *addRowButton = new QPushButton("Add Row", this);
  QPushButton *removeRowButton = new QPushButton("Remove Selected Row", this);

  valuesLayout->addWidget(tableWidget);
  valuesLayout->addWidget(addRowButton);
  valuesLayout->addWidget(removeRowButton);

  connect(sensorComboBox, &QComboBox::currentTextChanged, this, &AddDialog::update);
  connect(chartComboBox, &QComboBox::currentTextChanged, this, &AddDialog::update);
  connect(activeCheckBox, &QCheckBox::toggled, this, &AddDialog::update);
  connect(addRowButton, &QPushButton::clicked, this, &AddDialog::addRow);
  connect(removeRowButton, &QPushButton::clicked, this, &AddDialog::removeSelectedRow);
  connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &AddDialog::applyChanges);
  connect(buttonBox->button(QDialogButtonBox::Close), &QPushButton::clicked, this, &AddDialog::close);

  layout->addWidget(valuesGroupBox);    
  layout->addWidget(buttonBox);

  setLayout(layout);
  setMinimumSize(QSize(350, 200));
}

AddDialog::~AddDialog() {
  delete chartTitleEdit;
  delete sensorComboBox;
  delete chartComboBox;
  delete warningLabel;
  delete activeCheckBox;
  delete tableWidget;
  delete valuesGroupBox;
}

QString AddDialog::getChartTitle() const {
  return chartTitleEdit->text();
}

QString AddDialog::getSensor() const {
  return sensorComboBox->currentText();
}

QString AddDialog::getChart() const {
  return chartComboBox->currentText();
}

bool AddDialog::isActive() const {
  return activeCheckBox->isChecked();
}

QVector<QVariant> AddDialog::getValues() const {
  QVector<QVariant> values;
  for (int row = 0; row < tableWidget->rowCount(); ++row) {
    QLineEdit *lineEdit1 = qobject_cast<QLineEdit*>(tableWidget->cellWidget(row, 0));
    if (lineEdit1) {
      if (tableWidget->columnCount() == 1) {
        values.append(lineEdit1->text().toDouble());
      } else if (tableWidget->columnCount() == 2) {
        QLineEdit *lineEdit2 = qobject_cast<QLineEdit*>(tableWidget->cellWidget(row, 1));
        if (lineEdit2) {
          double value1 = lineEdit1->text().toDouble();
          double value2 = lineEdit2->text().toDouble();
          values.append(QVariant::fromValue(QPair<double, double>(value1, value2)));
        }
      }
    }
  }
  return values;
}

void AddDialog::update() {
  QString sensor = sensorComboBox->currentText();
  QString chart = chartComboBox->currentText();
  Sensor* tmpSensor = SensorFactory::createSensor(sensorComboBox->currentText());

  if (chart == "PieChart" && !tmpSensor->isReadingRelative()) {
    warningLabel->setText("The selected sensor is not ideally suited for a pie chart, as pie charts are designed to represent proportions of a whole. While it's possible to use this sensor data in a pie chart, the resulting visualization may not be as informative or useful compared to other chart types.");
    warningLabel->show();
  }
  else {
    warningLabel->hide();
  }

  updateTable();
  adjustSize();
}

void AddDialog::addRow() {
  int row = tableWidget->rowCount();

  tableWidget->insertRow(row);
  Sensor* tmpSensor = SensorFactory::createSensor(sensorComboBox->currentText());
  for (int column = 0; column < tableWidget->columnCount(); ++column) {
    QLineEdit *lineEdit = new QLineEdit(this);
    QDoubleValidator *validator = new QDoubleValidator(lineEdit);
    
    if (tmpSensor->isReadingRelative()) {
        validator = new QDoubleValidator(0, 100, 2, lineEdit);
      } else {
        validator = new QDoubleValidator(lineEdit);
    }
    lineEdit->setValidator(validator);
    connect(lineEdit, &QLineEdit::textChanged, this, &AddDialog::checkInput);
    tableWidget->setCellWidget(row, column, lineEdit);
    
  }
  delete tmpSensor;
}

void AddDialog::checkInput(const QString &text) {
  QLineEdit *lineEdit = qobject_cast<QLineEdit*>(sender());
  if (lineEdit) {
    bool ok;
    double value = text.toDouble(&ok);
    Sensor* tmpSensor = SensorFactory::createSensor(sensorComboBox->currentText());
    if ((!ok || value < 0 || value > 100) && (tmpSensor->isReadingRelative())) {
      lineEdit->undo();
    }
    delete tmpSensor;
  }      
}

void AddDialog::removeSelectedRow() {
  QItemSelectionModel *select = tableWidget->selectionModel();
  if (select->hasSelection())
    tableWidget->removeRow(tableWidget->currentRow());
}

void AddDialog::updateTable() {
  QString sensor = sensorComboBox->currentText();
  if (sensor.isEmpty() || activeCheckBox->isChecked()) {
    valuesGroupBox->hide();
    adjustSize();
  } else {
    QPropertyAnimation *animation = new QPropertyAnimation(this, "size");
    animation->setDuration(300);
    animation->setStartValue(size());

    Sensor* tmpSensor = SensorFactory::createSensor(sensor);
    QStringList label = tmpSensor->getReadingLables();
  
    if (tmpSensor->getQVariantType() == QMetaType::Double) {
      tableWidget->setColumnCount(1);
    } else {
      tableWidget->setColumnCount(2);
    }
    tableWidget->setHorizontalHeaderLabels(label);
  
    delete tmpSensor;  
    valuesGroupBox->show();  
    adjustSize();    
    animation->setEndValue(size());
    animation->start(QAbstractAnimation::DeleteWhenStopped);
  }
}

void AddDialog::applyChanges() {

  if (chartTitleEdit->text().isEmpty() || sensorComboBox->currentText().isEmpty() || chartComboBox->currentText().isEmpty()) {
    QMessageBox::warning(this, "Error", "Please fill all the fields");
  } else {
    accept();
  }
}