#pragma once

class Theme {
public:
    Theme(String name, uint16_t textColor, uint16_t backgroundColor, uint16_t selectedTextColor, uint16_t foregroundColor): name(name), textColor(textColor), backgroundColor(backgroundColor), selectedTextColor(selectedTextColor), foregroundColor(foregroundColor) {}
    uint16_t textColor, backgroundColor, selectedTextColor, foregroundColor;
    String name;
};

class ThemeLight: public Theme {
public:
    ThemeLight():Theme("Light", 0x0000, 0xffff, 0x001f, 0x0000){}
};

class ThemeDark: public Theme {
public:
   ThemeDark():Theme("Dark", 0xC800, 0x3186, 0xffff, 0xFFDF){}
};

