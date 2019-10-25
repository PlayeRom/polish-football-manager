
#ifndef INPUT_H
#define INPUT_H

#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <string>

// zmapowane klawisze z ciagu \033[...
#define KEY_MAP_UP      65 // A
#define KEY_MAP_DOWN    66 // B
#define KEY_MAP_RIGHT   67 // C
#define KEY_MAP_LEFT    68 // D

#define KEY_MAP_PAGE_UP     53 // 5
#define KEY_MAP_PAGE_DOWN   54 // 6
#define KEY_MAP_HOME        72 // H
#define KEY_MAP_END         70 // F
#define KEY_MAP_DELETE      51 // 3

#define KEY_MAP_F1          80
#define KEY_MAP_F2          81
#define KEY_MAP_F3          82
#define KEY_MAP_F4          83
#define KEY_MAP_F5          84
#define KEY_MAP_F6          85
#define KEY_MAP_F7          86
#define KEY_MAP_F8          87
//#define KEY_MAP_F9          88
//#define KEY_MAP_F10         89
//#define KEY_MAP_F11         50
//#define KEY_MAP_F12         91

#define _KEY_BACKSPACE  8
#define _KEY_TAB        9
#define _KEY_ENTER      10
// dla klawiszy nieaflanumerycznych ustawiam kody dla małych liter, jako ze uzywam toupper,
// wiec male litery nie sa uzywane
#define _KEY_UP         97
#define _KEY_DOWN       98
#define _KEY_RIGHT      99
#define _KEY_LEFT       100
#define _KEY_PAGE_UP    101
#define _KEY_PAGE_DOWN  102
#define _KEY_HOME       103
#define _KEY_END        104
#define _KEY_DELETE     105

class Input {
public:
    void clrscr() const;
    wchar_t getch() const;
    wchar_t getche() const;
    void getNumbersExchange(int &number1, int &number2) const;
    int getNumber() const;
    float getFloat() const;
    wchar_t getKeyBoardPressed() const;
    void getText2Buffer(wchar_t *pBuffer, int maxLength) const;
};

#endif /* INPUT_H */
