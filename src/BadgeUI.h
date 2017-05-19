// vim: noai:ts=2:sw=2
#pragma once

#define BUFFPIXEL 20
#include <GPNBadge.hpp>

#define UI_LINES_IN_MENU 3

#define BLUE    0x001F

class UIElement {
public:
  virtual void draw(TFT_ILI9163C* tft) = 0;
  virtual bool isDirty() = 0;
  virtual void dispatchInput(JoystickState state) {}
  UIElement* parent = nullptr;
};

class FullScreenStatus: public UIElement {
public:
  void draw(TFT_ILI9163C* tft);

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

private:
  String main = "FULL";
  String sub ="screen status";
  bool dirty = true;
};



class FullScreenBMPStatus: public UIElement {
public:
  void draw(TFT_ILI9163C* tft);

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

private:
  char* bmp = nullptr;
  String sub ="screen status";
  bool dirty = true;
  uint16_t bmpx = 0;
  uint16_t bmpy = 0;
  uint16_t subx = 12;
  uint16_t suby = 105;
  void bmpDraw(const char *filename, uint8_t x, uint16_t y, TFT_ILI9163C* tft);
};

class SimpleTextDisplay: public UIElement {
public:
  void draw(TFT_ILI9163C* tft);
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
    if(forceRedraw || head->isDirty()) {
      head->draw(this->tft);
      this->tft->writeFramebuffer();
      forceRedraw = false;
    }
  }

  void dispatchInput(JoystickState state){
    if(prevState == state) {
      return;
    }
    prevState = state;
    head->dispatchInput(state);
  }
protected:
  TFT_ILI9163C* tft;
private:
  bool forceRedraw = false;
  JoystickState prevState = JoystickState::BTN_NOTHING;
};

class MenuItem: public UIElement {
public:
  friend class Menu;
  MenuItem(String text, std::function<void(void)> trigger): text(text), triggerFunc(trigger) {
  } 

  void draw(TFT_ILI9163C* tft);
  
  bool isDirty() {
    return true;
  }

  void setSelect(bool selected) {
    this->selected = selected;
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
  ~Menu(){
    MenuItem * ite = tail;
    while(ite) {
      MenuItem * pre = ite->prev;
      delete ite;
      ite = pre;
    }
  }

  void draw(TFT_ILI9163C* tft);

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
    if(distanceDown >= UI_LINES_IN_MENU) {
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
};


