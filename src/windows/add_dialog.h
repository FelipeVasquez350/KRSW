#ifndef ADD_DIALOG_H
#define ADD_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTableWidget>
#include <QGroupBox>
#include <QPropertyAnimation>
#include <QHeaderView>

#include "utils/chart_factory.h"
#include "utils/sensor_factory.h"

class AddDialog : public QDialog {
  Q_OBJECT

public:
  AddDialog(QWidget *parent = nullptr);
  ~AddDialog();

  QString getChartTitle() const;
  QString getSensor() const;
  QString getChart() const;
  bool isActive() const;
  QVector<QVariant> getValues() const;

public slots:
  void update();
  void applyChanges();
  void addRow();
  void checkInput(const QString &text);
  void removeSelectedRow();
  void updateTable();

private:
  QLineEdit *chartTitleEdit;
  QComboBox *sensorComboBox;
  QComboBox *chartComboBox;
  QLabel *warningLabel;
  QCheckBox *activeCheckBox;
  QTableWidget *tableWidget;
  QGroupBox *valuesGroupBox;
};

#endif // ADD_DIALOG_H