#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

#define BUFFPIXEL 20

class UIElement {
public:
  virtual void draw(TFT_ILI9163C* tft) = 0;
  virtual bool isDirty() = 0;
  UIElement* parent = nullptr;
};

class FullScreenStatus: public UIElement {
public:
  void draw(TFT_ILI9163C* tft) {
    tft->fillScreen(WHITE);
    tft->setTextColor(BLACK);
    tft->setFont(&FreeSans24pt7b);
    tft->setTextSize(1);
    tft->setCursor(5, 75);
    tft->print(this->main);
    tft->setFont(&FreeSans9pt7b);
    tft->setCursor(12, 105);
    tft->print(this->sub);
    this->dirty = false;
  }
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


//Stolen form https://github.com/Jan--Henrik/TFT_ILI9163C/blob/master/examples/SD_example/SD_example.ino
uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

//Stolen form https://github.com/Jan--Henrik/TFT_ILI9163C/blob/master/examples/SD_example/SD_example.ino
uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}


class FullScreenBMPStatus: public UIElement {
public:
  void draw(TFT_ILI9163C* tft) {
    tft->fillScreen(WHITE);
    tft->setTextColor(BLACK);
    if(bmp) {
    	bmpDraw(this->bmp, bmpx, bmpy, tft);
    }
    tft->setFont(&FreeSans9pt7b);
    tft->setCursor(subx, suby);
    tft->print(this->sub);
    this->dirty = false;
  }
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
  // Stolen from https://github.com/Jan--Henrik/TFT_ILI9163C/blob/master/examples/SD_example/SD_example.ino
  void bmpDraw(const char *filename, uint8_t x, uint16_t y, TFT_ILI9163C* tft) {
    File     bmpFile;
    uint16_t bmpWidth, bmpHeight;   // W+H in pixels
    uint8_t  bmpDepth;              // Bit depth (currently must be 24)
    uint32_t bmpImageoffset;        // Start of image data in file
    uint32_t rowSize;               // Not always = bmpWidth; may have padding
    uint8_t  sdbufferLen = BUFFPIXEL * 3;
    uint8_t  sdbuffer[sdbufferLen]; // pixel buffer (R+G+B per pixel)
    uint8_t  buffidx = sdbufferLen; // Current position in sdbuffer
    boolean  goodBmp = false;       // Set to true on valid header parse
    boolean  flip    = true;        // BMP is stored bottom-to-top
    uint16_t w, h, row, col;
    uint8_t  r, g, b;
    uint32_t pos = 0;

    if((x >= tft->width()) || (y >= tft->height())) return;

    // Open requested file on SD card
    if ((bmpFile = SPIFFS.open(filename,"r")) == NULL) {
      tft->setCursor(20,20);
      tft->print("file not found!");
      return;
    }

    // Parse BMP header
    if(read16(bmpFile) == 0x4D42) { // BMP signature
      read32(bmpFile);
      (void)read32(bmpFile); // Read & ignore creator bytes
      bmpImageoffset = read32(bmpFile); // Start of image data
      // Read DIB header
      read32(bmpFile);
      bmpWidth  = read32(bmpFile);
      bmpHeight = read32(bmpFile);
      if(read16(bmpFile) == 1) { // # planes -- must be '1'
	bmpDepth = read16(bmpFile); // bits per pixel
	if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed
	  goodBmp = true; // Supported BMP format -- proceed!
	  rowSize = (bmpWidth * 3 + 3) & ~3;// BMP rows are padded (if needed) to 4-byte boundary
	  if (bmpHeight < 0) {
	    bmpHeight = -bmpHeight;
	    flip      = false;
	  }
	  // Crop area to be loaded
	  w = bmpWidth;
	  h = bmpHeight;
	  if((x+w-1) >= tft->width())  w = tft->width()  - x;
	  if((y+h-1) >= tft->height()) h = tft->height() - y;
	  //tft->startPushData(x, y, x+w-1, y+h-1);
	  for (row=0; row<h; row++) { // For each scanline...
	    if (flip){ // Bitmap is stored bottom-to-top order (normal BMP)
	      pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
	    } 
	    else {     // Bitmap is stored top-to-bottom
	      pos = bmpImageoffset + row * rowSize;
	    }
	    if (bmpFile.position() != pos) { // Need seek?
	      bmpFile.seek(pos, SeekSet);
	      buffidx = sdbufferLen; // Force buffer reload
	    }
	    for (col=0; col<w; col++) { // For each pixel...
	      // Time to read more pixel data?
	      if (buffidx >= sdbufferLen) { // Indeed
	        bmpFile.read(sdbuffer, sdbufferLen);
	        buffidx = 0; // Set index to beginning
	      }
	      // Convert pixel from BMP to TFT format, push to display
	      b = sdbuffer[buffidx++];
	      g = sdbuffer[buffidx++];
	      r = sdbuffer[buffidx++];
	      tft->drawPixel(x+col, y+row, tft->Color565(r,g,b));
	    } // end pixel
	  } // end scanline
	  //tft->endPushData();
	} // end goodBmp
      }
    }

    bmpFile.close();
    if(!goodBmp) {
      tft->setCursor(20,20);
      tft->print("file unrecognized!");
    }
   }

};

class SimpleTextDisplay: public UIElement {
public:
  void draw(TFT_ILI9163C* tft) {
    tft->fillScreen(WHITE);
    tft->setTextColor(BLACK);
    tft->setFont(&FreeSans9pt7b);
    tft->setCursor(2, 18);
    tft->print(this->text);
    this->dirty = false;
  }
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




