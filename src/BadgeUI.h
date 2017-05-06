#pragma once

#define BUFFPIXEL 20

class UIElement {
public:
  virtual void draw(TFT_ILI9163C* tft) = 0;
  virtual bool isDirty() = 0;
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
protected:
  TFT_ILI9163C* tft;
private:
  bool forceRedraw = false;
};

