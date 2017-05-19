#pragma once

class Theme {
public:
    Theme(uint16_t textColor, uint16_t backgroundColor, uint16_t selectedTextColor, uint16_t foregroundColor): textColor(textColor), backgroundColor(backgroundColor), selectedTextColor(selectedTextColor), foregroundColor(foregroundColor) {}
    uint16_t textColor, backgroundColor, selectedTextColor, foregroundColor;
};

class ThemeLight: public Theme {
public:
    ThemeLight():Theme(0x0000, 0xffff, 0x001f, 0x0000){}
};

class ThemeDark: public Theme {
public:
   ThemeDark():Theme(0xC800, 0x3186, 0xffff, 0xFFDF){}
};

