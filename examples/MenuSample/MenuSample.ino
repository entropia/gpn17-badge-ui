#include<GPNBadge.hpp>
#include<BadgeUI.h>

Badge badge;
WindowSystem* ui = new WindowSystem(&tft);
Menu * menu = new Menu();

void setup() {
  badge.init();
  badge.setBacklight(true);
  ui->open(menu);
  menu->addMenuItem(new MenuItem("Vibrate", [](){ badge.setVibrator(true); delay(200); badge.setVibrator(false); }));
  menu->addMenuItem(new MenuItem("Left", [](){ pixels.setPixelColor(1, pixels.Color(0, 0, 100)); pixels.show(); }));
  menu->addMenuItem(new MenuItem("Down blue", [](){ pixels.setPixelColor(3, pixels.Color(0, 0, 100)); pixels.show(); }));
  menu->addMenuItem(new MenuItem("Right", [](){ pixels.setPixelColor(2, pixels.Color(200, 0, 100)); pixels.show(); }));
  menu->addMenuItem(new MenuItem("Up", [](){ pixels.setPixelColor(0, pixels.Color(200, 0, 100)); pixels.show(); }));
  ui->draw();
}

void loop() {
  ui->dispatchInput(badge.getJoystickState());
  ui->draw();
}
