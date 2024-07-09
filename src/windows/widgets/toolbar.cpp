#include "toolbar.h"

#include <QStyle>

ToolBar::ToolBar(QWidget *parent) : QToolBar(parent) {
  setMovable(true);
  setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea | Qt::LeftToolBarArea | Qt::RightToolBarArea);
  setFloatable(false);

  QString theme;
  if (this->palette().window().color().lightness() < 128) {
    theme = "dark";
  } else {
    theme = "light";
  }

  QAction *add = addAction(QIcon(":/"+theme+"/add.svg"), "Add");
  QAction *edit = addAction(QIcon(":/"+theme+"/edit.svg"), "Edit");
  QAction *play = addAction(QIcon(":/"+theme+"/play.svg"), "Play");
  QAction *pause = addAction(QIcon(":/"+theme+"/pause.svg"), "Pause");

  connect(add, &QAction::triggered, this, &ToolBar::add);
  connect(edit, &QAction::triggered, this, &ToolBar::edit);
  connect(play, &QAction::triggered, this, &ToolBar::play);
  connect(pause, &QAction::triggered, this, &ToolBar::pause);
  
  setIconSize(QSize(32, 32));
  setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
}

