// vim: noai:ts=2:sw=2
#pragma once

#define BUFFPIXEL 20
#include <GPNBadge.hpp>
#include <UIThemes.h>

#define BLUE    0x001F

class UIElement {
public:
  virtual void draw(TFT_ILI9163C* tft, Theme * theme, uint16_t offsetX, uint16_t offsetY) = 0;
  virtual bool isDirty() = 0;
  virtual void dispatchInput(JoystickState state) {}
  virtual ~UIElement() {}
  UIElement* parent = nullptr;
  virtual bool isValid() {
    return true;
  }
  virtual bool requiresFullScreen() {
    return false;
  }
};

class FullScreenStatus: public UIElement {
public:
  void draw(TFT_ILI9163C* tft, Theme * theme, uint16_t offsetX, uint16_t offsetY);

  bool isDirty() {
    return dirty;
  }

 void setMain(String main) {
    this->main = main;
    this->dirty = true;
  }
  void setSub(String sub) {
    this->sub = sub;
    this->dirty = true;
  }

  bool requiresFullScreen() {
    return true;
  }

private:
  String main = "FULL";
  String sub ="screen status";
  bool dirty = true;
};

class Overlay: public UIElement {
public:  
  virtual uint16_t getOffsetX() = 0;
  virtual uint16_t getOffsetY() = 0;
  bool isValid() {
    return true;
  }
  bool requiresFullScreen() {
    return false;
  }
  virtual void draw(TFT_ILI9163C* tft, Theme * theme, uint16_t offsetX, uint16_t offsetY) = 0;
  virtual bool isDirty() = 0;
};

class StatusOverlay: public Overlay {
public:
  StatusOverlay(uint16_t batCritical, uint16_t batFull): batCritical(batCritical), batFull(batFull), bat(batCritical) {}
  bool isDirty() {
    return dirty;
  }
  void draw(TFT_ILI9163C* tft, Theme * Theme, uint16_t offsetX, uint16_t offsetY);
  bool isValid() {
    return true;
  }
  void updateBat(uint16_t bat) {
    if(this->bat == bat) {
      return;
    }
    this->bat = bat;
    this->dirty = true;
  }
  void updateWiFiState(String wifi) {
    if(this->wifi.equals(wifi)) {
      return;
    }
    this->wifi = wifi;
    this->dirty = true;
  }
  uint16_t getOffsetX();
  uint16_t getOffsetY();
private:
  String wifi;
  uint16_t bat, batCritical, batFull;
  bool dirty = true;
};

class NotificationScreen: public UIElement {
public:
  NotificationScreen(String summary, String location, String description): summary(summary), location(location), description(description){}

  void draw(TFT_ILI9163C* tft, Theme * theme, uint16_t offsetX, uint16_t offsetY);

  bool isDirty() {
    return dirty;
  }

  void dispatchInput(JoystickState state) {
    if(state == JoystickState::BTN_ENTER) {
      valid = false;
    }
  }

  bool isValid() {
    return valid;
  }

  bool requiresFullScreen() {
    return true;
  }
private:
  String summary, location, description;
  bool dirty = true;
  bool valid = true;
};

class BMPRender {
public:
  void bmpDraw(const char *filename, uint8_t x, uint16_t y, TFT_ILI9163C* tft);
};

class FullScreenBMPStatus: public UIElement, BMPRender {
public:
  void draw(TFT_ILI9163C* tft, Theme * theme, uint16_t offsetX, uint16_t offsetY);

  bool isDirty() {
    return dirty;
  }
  void setBmp(char* path, uint16_t x, uint16_t y) {
    this->bmp = path;
    this->bmpx = x;
    this->bmpy = y;
    this->dirty = true;
  }
  void setSub(String sub) {
    this->setSub(sub, 12, 105);
  }

  void setSub(String sub, uint16_t x, uint16_t y) {
    if(sub == this->sub && subx == x && suby == y) {
    	return;
    }
    this->sub = sub;
    this->subx = x;
    this->suby = y;
    this->dirty = true;
  }

  bool requiresFullScreen() {
    return true;
  }

private:
  char* bmp = nullptr;
  String sub ="screen status";
  bool dirty = true;
  uint16_t bmpx = 0;
  uint16_t bmpy = 0;
  uint16_t subx = 12;
  uint16_t suby = 105;
};

class FullScreenBMPDisplay: public UIElement, BMPRender {
public:
  void draw(TFT_ILI9163C* tft, Theme * theme, uint16_t offsetX, uint16_t offsetY);

  bool isDirty() {
    return dirty;
  }
  void setBmp(char* path) {
    this->bmp = path;
    this->dirty = true;
  }

  bool requiresFullScreen() {
    return true;
  }

  void dispatchInput(JoystickState state) {
    if(state == JoystickState::BTN_ENTER) {
      valid = false;
    }
  }

  bool isValid() {
    return valid;
  }

private:
  char* bmp = nullptr;
  bool dirty = true;
  bool valid = true;
};

class SimpleTextDisplay: public UIElement {
public:
  void draw(TFT_ILI9163C* tft, Theme * theme, uint16_t offsetX, uint16_t offsetY);
  bool isDirty() {
    return dirty;
  }
  void setText(String text) {
    this->text = text;
    this->dirty = true;
  }

private:
  String text = "FULL";
  bool dirty = true;
};

class ClosableTextDisplay: public SimpleTextDisplay {
public:

  void dispatchInput(JoystickState state) {
    if(state == JoystickState::BTN_ENTER) {
      valid = false;
      onClose();
    }
  }

  bool isValid() {
    return valid;
  }

  void setOnClose(std::function<void()> onClose) {
    this->onClose = onClose;
  }


private:
  bool valid = true;
  std::function<void()> onClose;
};

class WindowSystem {
public:
  FullScreenBMPStatus* root;
  UIElement* head;
  WindowSystem(TFT_ILI9163C* tft): tft(tft) {
    head = root = new FullScreenBMPStatus();
  }

  void open(UIElement* element) {
    element->parent = head;
    head = element;
  }

  void closeCurrent() {
    UIElement* old = head;
    if(!old->parent) {
      return;
    }
    head = old->parent;
    forceRedraw = true;
    delete old;
  }

  void draw() {
    if(!head->isValid()) {
      closeCurrent();
      return;
    }
    if(forceRedraw || head->isDirty()) {
      if(overlay && !head->requiresFullScreen()) {
        head->draw(this->tft, theme, overlay->getOffsetX(), overlay->getOffsetY());
      } else { 
        head->draw(this->tft, theme,0 ,0);
      }
      this->tft->writeFramebuffer();
      forceRedraw = false;
      if(overlay && !head->requiresFullScreen()) {
        overlay->draw(this->tft, theme, 0, 0);
      } 
    }
    if(overlay && !head->requiresFullScreen() && overlay->isDirty()) {
      overlay->draw(this->tft, theme, 0, 0);
    }
  }

  void dispatchInput(JoystickState state){
    if(prevState == state) {
      return;
    }
    prevState = state;
    head->dispatchInput(state);
  }

  void setTheme(Theme * theme) {
    delete this->theme;
    this->theme = theme;
    forceRedraw = true;
  }

  void setOverlay(Overlay * overlay) {
    this->overlay = overlay;
    forceRedraw = true;
  }

  Theme * getTheme() {
    return theme;
  }
protected:
  TFT_ILI9163C* tft;
private:
  Overlay * overlay = nullptr;
  bool forceRedraw = false;
  JoystickState prevState = JoystickState::BTN_NOTHING;
  Theme * theme = new ThemeLight();
};

class MenuItem: public UIElement {
public:
  friend class Menu;
  MenuItem(String text, std::function<void(void)> trigger): text(text), triggerFunc(trigger) {
  } 

  void draw(TFT_ILI9163C* tft, Theme * theme, uint16_t offsetX, uint16_t offsetY);
  
  bool isDirty() {
    return true;
  }

  void setSelect(bool selected) {
    this->selected = selected;
  }

  void setText(String text) {
    this->text = text;
  }

  void setTrigger(std::function<void()> triggr) {
    this->triggerFunc = triggr;
  }
private:
  String text;
  std::function<void(void)> triggerFunc;
  bool selected = false;
protected:
  MenuItem* prev = nullptr;
  MenuItem* next = nullptr;
  void trigger() {
    triggerFunc();
  }
};

class Menu: public UIElement {
public:
  Menu(int itemsPerPage): itemsPerPage(itemsPerPage) {}
  Menu():Menu(3) {}
  ~Menu(){
    MenuItem * ite = tail;
    while(ite) {
      MenuItem * pre = ite->prev;
      delete ite;
      ite = pre;
    }
  }

  void draw(TFT_ILI9163C* tft, Theme * theme, uint16_t offsetX, uint16_t offsetY);

  void dispatchInput(JoystickState state) {
    switch(state) {
      case JoystickState::BTN_UP:
        if(!focus->prev) {
          return;
        }
        focus->setSelect(false);
        focus = focus->prev;
        focus->setSelect(true);
        break;
      case JoystickState::BTN_DOWN:
        if(!focus->next) {
          return;
        }
        focus->setSelect(false);
        focus = focus->next;
        focus->setSelect(true);
        break;
      case JoystickState::BTN_ENTER:
        focus->trigger();
        break;
      default:
        return;
    }
    int distanceDown = 0;
    MenuItem * temp = firstVisible;
    while(temp != focus) {
      if(!temp->next) {
        distanceDown = 0;
        break;
      }
      temp = temp->next;
      distanceDown++;

    }
    if(distanceDown >= itemsPerPage) {
      firstVisible = firstVisible->next;
    }
    if(focus->next == firstVisible) {
      firstVisible = firstVisible->prev;
    }
    dirty = true;
  }

  bool isDirty() {
    return dirty;
  }

  void addMenuItem(MenuItem * item) {
    item->prev = tail;
    item->next = nullptr;
    if(tail) {
      tail->next = item;
    } else {
      firstVisible = item;
      focus = item;
      item->setSelect(true);
    }
    tail = item;
    dirty = true;
  }
private:
  bool dirty = true;
  MenuItem * tail = nullptr;
  MenuItem * firstVisible = nullptr;
  MenuItem * focus = nullptr;
  int itemsPerPage;
};


