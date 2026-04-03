#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>

#include "../SWFluentUI/swtoggleswitch.hpp"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QWidget window;
  window.setWindowTitle("SW_Widgets Demo");

  auto *layout = new QVBoxLayout(&window);

  auto *toggle1 = new SWFluentUI::SWToggleSwitch();
  toggle1->setChecked(true);

  auto *toggle2 = new SWFluentUI::SWToggleSwitch();
  toggle2->setEnabled(false);

  layout->addWidget(toggle1);
  layout->addWidget(toggle2);

  window.resize(300, 200);
  window.show();

  return app.exec();
}