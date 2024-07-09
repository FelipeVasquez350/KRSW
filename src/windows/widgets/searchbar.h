#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QWidget>
#include <QToolBar>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QSplitter>
#include <QWidgetAction>
#include <QToolButton>

#include "utils/sensor_factory.h"
#include "utils/chart_factory.h"

class SearchBar : public QToolBar {
  Q_OBJECT
private:
  QHBoxLayout *filterLayout;
  QLineEdit *lineEdit;
  QComboBox *comboBox;

private slots:
  void addFilter(int index);
  void performSearch(const QString &text);

public:
  SearchBar(QWidget *parent = nullptr);
  ~SearchBar();

signals:
  void search(const QString &text, const QStringList &filters);
};

#endif // SEARCHBAR_H