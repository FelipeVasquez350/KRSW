#ifndef EDIT_DIALOG_H
#define EDIT_DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QHeaderView>

#include <QSplitter>
#include <QListWidget> 

#include "widgets/chartdock.h"
#include "utils/sensor_factory.h"
#include "utils/chart_factory.h"

class EditDialog : public QDialog {
  Q_OBJECT
private:
  QList<ChartDock*> docksCopy;
  QList<bool> pausedSensors;
  QList<QLineEdit*> lineEditList;
  QList<QPair<QComboBox*, QComboBox*>> comboBoxList;
  QList<QCheckBox*> checkBoxList;
  QList<QTableWidget*> tableList;
  
  void buildUI();
  
public:
  EditDialog(QList<ChartDock*> charts, QList<bool> pausedSensors, QWidget *parent = nullptr);

  QList<ChartDock*> getModifiedDocks() const;

private slots:  
  void addRow(QTableWidget* tableWidget, int index);
  void checkInput(const QString &text, int index);
  void updateTable(int index);
  void removeSelectedRow(QTableWidget* tableWidget);
  void applyChanges();
};

#endif // EDIT_DIALOG_H