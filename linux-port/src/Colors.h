#ifndef COLORS_H
#define COLORS_H

#include <stdarg.h>
#include <string>

// https://en.wikipedia.org/wiki/ANSI_escape_code#graphics

class Colors {
public:
    // Foreground colors
    const static int FG_DEFAULT         = 39;
    const static int FG_BLACK           = 30;
    const static int FG_RED             = 31;
    const static int FG_GREEN           = 32;
    const static int FG_YELLOW          = 33;
    const static int FG_BLUE            = 34;
    const static int FG_MAGENTA         = 35;
    const static int FG_CYAN            = 36;
    const static int FG_LIGHT_GRAY      = 37;
    const static int FG_DARK_GRAY       = 90;
    const static int FG_LIGHT_RED       = 91;
    const static int FG_LIGHT_GREEN     = 92;
    const static int FG_LIGHT_YELLOW    = 93;
    const static int FG_LIGHT_BLUE      = 94;
    const static int FG_LIGHT_MAGENTA   = 95;
    const static int FG_LIGHT_CYAN      = 96;
    const static int FG_WHITE           = 97;

    // Misc
    const static int RESET              = 0;

    const std::wstring setColors(int argCount, ...) const;
    int convertFromConio(int conioColor, bool isBkg) const;

    void textcolor(int colorConio) const;
    void textbackground(int colorConio) const;

private:
    int getFgBgColor(int color, bool isBkg) const;
};

#endif /* COLORS_H */
