#include "info_dialog.h"

InfoDialog::InfoDialog(QWidget *parent): QDialog(parent) {
  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  QVBoxLayout *layout = new QVBoxLayout(mainLayout->parentWidget());
  QLabel *iconLabel = new QLabel(this);
  QPixmap iconPixmap(":/KRSW.png");
  iconLabel->setPixmap(iconPixmap);
  iconLabel->setAlignment(Qt::AlignTop);

  QLabel *titleLabel = new QLabel("KRSW - Kute Resources Sensor Widget");
  QFont titleFont = titleLabel->font();
  titleFont.setPointSize(18);
  titleLabel->setFont(titleFont);

  QLabel *descriptionLabel = new QLabel("KSRW, also known as Karasawa, is a desktop application designed to monitor your computer's resource usage. Developed with C++ and Qt6, it offers a modular user interface, with the capability to introduce new sensors and their associated charts that represent the data currently monitored.\n\nThe toolbar on the left allows you to  start, stop, add or modify existing sensor-chart pairs, while the search bar on top allows you to search and filter them by name. The sensors can also be moved according to your preference, either within the main window dock or as standalone windows by dragging them out. If a chart is no longer needed, it can be removed by clicking on the [x] at the top-right. To remove all charts simultaneously, use the 'clear' option in the menu.\nTo preserve the data accumulated so far, you can use the options in the menu to save and load them via the use of a simple JSON file.\n\nEnjoy using KSRW!");
  descriptionLabel->setWordWrap(true);
  descriptionLabel->setMinimumHeight(280);

  QLabel *versionLabel = new QLabel("Version: 1.0.0");

  QLabel *copyrightLabel = new QLabel("Copyright (c) 2024 ~v350");

  QPushButton *closeButton = new QPushButton("Close");
  connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

  layout->addWidget(titleLabel);
  layout->addWidget(descriptionLabel);
  layout->addWidget(versionLabel);
  layout->addWidget(copyrightLabel);
  layout->addWidget(closeButton);

  mainLayout->addWidget(iconLabel);
  mainLayout->addLayout(layout);
  mainLayout->setAlignment(iconLabel, Qt::AlignCenter);
  resize(850, 500);
}

InfoDialog::~InfoDialog() {}