#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

class InfoDialog : public QDialog {
  Q_OBJECT

public:
  InfoDialog(QWidget *parent = nullptr);
  ~InfoDialog();
};