#include <stdlib.h>
#include <sstream>  // for string streams
#include <string>  // for string
#include "Colors.h"

using namespace std;

const std::wstring Colors::setColors(int argCount, ...) const
{
    // declaring output string stream
    std::wostringstream output;
    output << L"\033[";

    va_list ap;
    va_start(ap, argCount);
    for (int i = 0; i < argCount; i++) {
        int value = va_arg(ap, int);
        output << value;
        if (i < argCount - 1) {
            output << L";";
        }
    }
    va_end(ap);

    output << L"m";

    return output.str();
}

int Colors::convertFromConio(int conioColor, bool isBkg) const
{
    switch (conioColor) {
        case 0: // conio.h BLACK
            if (isBkg) {
                // for black bkg set default
                return getFgBgColor(Colors::FG_DEFAULT, isBkg);
            }
            return getFgBgColor(Colors::FG_BLACK, isBkg);
        case 1: // conio.h BLUE
            return getFgBgColor(Colors::FG_BLUE, isBkg);
        case 2: // conio.h GREEN
            return getFgBgColor(Colors::FG_GREEN, isBkg);
        case 3: // conio.h CYAN
            return getFgBgColor(Colors::FG_CYAN, isBkg);
        case 4: // conio.h RED
            return getFgBgColor(Colors::FG_RED, isBkg);
        case 5: // conio.h MAGENTA
            return getFgBgColor(Colors::FG_MAGENTA, isBkg);
        //case 6: // conio.h BROWN
        //    return getFgBgColor(Colors::FG_BLUE, isBkg); // <- nie mamy bronzu
        case 7: // conio.h LIGHTGAY
            return getFgBgColor(Colors::FG_LIGHT_GRAY, isBkg);
        case 8: // conio.h DARKGRAY
            return getFgBgColor(Colors::FG_DARK_GRAY, isBkg);
        case 9: // conio.h LIGHTBLUE
            return getFgBgColor(Colors::FG_LIGHT_BLUE, isBkg);
        case 10: // conio.h LIGHTGREEN
            return getFgBgColor(Colors::FG_LIGHT_GREEN, isBkg);
        case 11: // conio.h LIGHTCYAN
            return getFgBgColor(Colors::FG_LIGHT_CYAN, isBkg);
        case 12: // conio.h LIGHTRED
        case 140:
            return getFgBgColor(Colors::FG_LIGHT_RED, isBkg);
        case 13: // conio.h LIGHTMAGENTA
            return getFgBgColor(Colors::FG_LIGHT_MAGENTA, isBkg);
        case 14: // conio.h YELLOW
            return getFgBgColor(Colors::FG_YELLOW, isBkg);
        case 15: // conio.h WHITE
        case 143:
            return getFgBgColor(Colors::FG_WHITE, isBkg);
        default:
            return getFgBgColor(Colors::FG_DEFAULT, isBkg);
    }
}

int Colors::getFgBgColor(int color, bool isBkg) const
{
    return isBkg ? color + 10 : color;
}

void Colors::textColor(int colorConio) const {
    int color = convertFromConio(colorConio, false);
    wcout << setColors(1, color);
}

void Colors::textBackground(int colorConio) const {
    int color = convertFromConio(colorConio, true);
    wcout << setColors(1, color);
}
