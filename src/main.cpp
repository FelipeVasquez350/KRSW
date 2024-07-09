#include <QApplication> 
#include "windows/main_window.h"

int main(int argc, char *argv[]) {     
  QCoreApplication::setOrganizationName("V350");
  QCoreApplication::setApplicationName("KRSW");
  QApplication app(argc, argv);

  MainWindow mainWindow;
  mainWindow.show();

  return QApplication::exec();
}
