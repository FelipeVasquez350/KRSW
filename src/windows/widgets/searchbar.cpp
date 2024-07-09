#include "searchbar.h"

SearchBar::SearchBar(QWidget *parent) : QToolBar(parent) {
  setMovable(true);
  setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
  setFloatable(false);

  QLabel *label = new QLabel("Search: ");
  lineEdit = new QLineEdit;   
    
  QLabel *labelComboBox = new QLabel("Whitelist: ");
  comboBox = new QComboBox;
  QStringList filters = QList<QString>() << " ";
  filters.append(SensorFactory::getSensorTypes());
  filters.append(ChartFactory::getChartTypes());
  comboBox->addItems(filters);
  comboBox->setCurrentIndex(0);

  QWidget *filterWidget = new QWidget();
  filterLayout = new QHBoxLayout(filterWidget);
  
  addWidget(label);
  addWidget(lineEdit);  
  addWidget(filterWidget);
  addWidget(labelComboBox);
  addWidget(comboBox);

  connect(comboBox, QOverload<int>::of(&QComboBox::activated), this, &SearchBar::addFilter);
  connect(lineEdit, &QLineEdit::textChanged, this, &SearchBar::performSearch);

  setMinimumHeight(50);  
}

SearchBar::~SearchBar() {
  delete lineEdit;
  delete comboBox;
  delete filterLayout;
}

void SearchBar::addFilter(int index) {
  if (index == 0) return;

  QString filter = comboBox->itemText(index);

  QToolButton *button = new QToolButton;
  button->setText(filter);
  button->setCheckable(true);
  button->setChecked(true);
  button->setProperty("originalIndex", index); 
  filterLayout->addWidget(button);

  comboBox->removeItem(index);
  comboBox->setCurrentIndex(0);

  performSearch(lineEdit->text());

  connect(button, &QToolButton::toggled, this, [this, button](bool checked) {
    if (!checked) {
      filterLayout->removeWidget(button);
      int originalIndex = button->property("originalIndex").toInt();
      QString itemText = button->text(); itemText.remove("&");
      comboBox->insertItem(originalIndex, itemText);
      button->deleteLater();
      performSearch(lineEdit->text());
    }
  });
}

void SearchBar::performSearch(const QString &text) {
  QStringList filters;
  for (int i = 0; i < filterLayout->count(); ++i) {
    QToolButton *button = qobject_cast<QToolButton *>(filterLayout->itemAt(i)->widget());
    if (button && button->isChecked()) {
      QString itemText = button->text(); itemText.remove("&");
      filters.append(itemText);
    }
  }

  emit search(text, filters);
}