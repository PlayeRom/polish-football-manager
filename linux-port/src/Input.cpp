
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <sys/select.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "Input.h"

void Input::clrscr() const
{
    (void) !system("clear");
}

/**
 * Return input from keyboard without printing the sign
 * @return wchar_t
 */
wchar_t Input::getch() const
{
    struct termios oldt, newt;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	wchar_t result = getwchar(); // dla entera getwchar zwraza \n, a getchar \r, grrrr!
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	return result;
}

/**
 * Return input from keyboard with printing the sign, without waiting for enter
 * @return char
 */
wchar_t Input::getche() const
{
    // Set terminal to raw mode
    (void) !system("stty raw");

    wchar_t result = std::wcin.get();

    // Reset terminal to normal "cooked" mode
    (void) !system("stty cooked");

    return result;
}


/**
 * Input dla podania dwóch numerów oddzielonych spacją
 * @param number1
 * @param number2
 */
void Input::getNumbersExchange(int &number1, int &number2) const
{
    std::wstring input;
    std::getline(std::wcin, input);

    // rozdziel liczby odzielone spacja
    std::wistringstream iss(input);
    std::wstring strNumber;
    int count = 0;

    try {
        while (std::getline(iss, strNumber, L' ')) {
            if (count == 0) {
                //number1 = atoi(strNumber.c_str());
                number1 = std::stoi(strNumber);
            }
            else {
                //number2 = atoi(strNumber.c_str());
                number2 = std::stoi(strNumber);
            }
            count++;
        }
    }
    catch (const std::invalid_argument& exc) {
        // ustaw dwie te same liczby, czyli nic nie rób
        number1 = 1;
        number2 = 1;
    }
}

int Input::getNumber() const
{
    std::wstring strInput;
    std::getline(std::wcin, strInput);

    try {
        return std::stoi(strInput);
    }
    catch (const std::invalid_argument &exc) {
        return 0;
    }
}

float Input::getFloat() const
{
    try {
        std::wstring strInput;
        std::getline(std::wcin, strInput);

        // zmian '.' na ','
        int len = strInput.length();
        for (int i = 0; i < len; i++) {
            if (strInput[i] == L'.') {
                strInput[i] = ',';
            }
            else {
                strInput[i] = strInput[i];
            }
        }

        return std::stof(strInput);
    }
    catch (const std::invalid_argument &exc) {
        return 0;
    }
}

/**
 * W sumie tego należy używac przy wskazaniu przez usera akcji we wszelkim menu
 * Obsługuje znaki alfanumeryczne, strzałki, page up/down, home/end, delete, backspance, tab, enter
 * UWAGA 1: Nie zwróci espace.
 * UWAGA 2: F1 - F12 zwóci niespodziewane wyniki
 * @return Wcisniety klawisz
 */
wchar_t Input::getKeyBoardPressed() const
{
    wchar_t key[5];
    int i = 0;
    bool isEscape = false;
    while (1) {
        if (i >= 3) {
            i = 0;
        }

        wchar_t ch = getch();
        if (ch == L'\033') {
            // klawisz niealfa zawsze rozpoczyna się od znaku escape '\033', wiec moge wyzerowac index
            // Jest to potrzebne bo klawisz Escape wysyła tylko ten jeden znak
            isEscape = true;
            i = 0;
        }
        else if (i == 1 && ch != L'[' && ch != L'O' && isEscape) {
            isEscape = false;
            i = 0;
        }
        else if (i == 2 && isEscape) {
            isEscape = false;
        }
        /*if (c == L'\n') { // gdyby getch wymagało entera, trzeba to odkomentowac
            i = 0;
            continue;
        }
        else*/
        wchar_t cToUpper = toupper(ch);
        if (i == 0 &&
            (
                (cToUpper >= 32 && cToUpper <= 126) || // zakres normalnych zanków ascii
                cToUpper == _KEY_BACKSPACE ||
                cToUpper == _KEY_TAB ||
                cToUpper == _KEY_ENTER
            )
        ) {
            // zwróć klawisz alfanumeryczny toupper
            return cToUpper;
        }
        else if (i == 2 && key[0] == L'\033' && key[1] == L'[' &&
            (
                ch == KEY_MAP_UP || ch == KEY_MAP_DOWN || ch == KEY_MAP_RIGHT || ch == KEY_MAP_LEFT ||
                ch == KEY_MAP_PAGE_UP || ch == KEY_MAP_PAGE_DOWN || ch == KEY_MAP_HOME || ch == KEY_MAP_END ||
                ch == KEY_MAP_DELETE
            )
        ) {
            // zwróć klawsz speclany/niealfanumeryczny
            switch (ch) {
                case KEY_MAP_UP:        return _KEY_UP;
                case KEY_MAP_DOWN:      return _KEY_DOWN;
                case KEY_MAP_RIGHT:     return _KEY_RIGHT;
                case KEY_MAP_LEFT:      return _KEY_LEFT;
                case KEY_MAP_PAGE_UP:   return _KEY_PAGE_UP;
                case KEY_MAP_PAGE_DOWN: return _KEY_PAGE_DOWN;
                case KEY_MAP_HOME:      return _KEY_HOME;
                case KEY_MAP_END:       return _KEY_END;
                case KEY_MAP_DELETE:    return _KEY_DELETE;
            }
        }
        /*else if (i == 2 && key[0] == L'\033' && key[1] == L'O' &&
            (
                ch == KEY_MAP_F1 || ch == KEY_MAP_F2 || ch == KEY_MAP_F3 || ch == KEY_MAP_F4 ||
                ch == KEY_MAP_F5 || ch == KEY_MAP_F6 || ch == KEY_MAP_F7 || ch == KEY_MAP_F8 ||
                ch == KEY_MAP_F9 || ch == KEY_MAP_F10 || ch == KEY_MAP_F11 || ch == KEY_MAP_F12
            )
        ) {
            return 1;
        }*/
        else {
            key[i] = ch;
        }
        i++;
    }
}

void Input::getText2Buffer(wchar_t *pBuffer, int maxLength) const
{
    std::wstring strName;
    std::getline(std::wcin, strName);

    int lenght = std::min((int)strName.length(), maxLength - 1);
    for (int i = 0; i < lenght; i++) {
        pBuffer[i] = strName[i];
    }
    pBuffer[lenght + 1] = 0;

    pBuffer[0] = toupper(pBuffer[0]);
}

int Input::kbhit() const
{
    static const int STDIN = 0;
    static bool initialized = false;

    if (! initialized) {
        // Use termios to turn off line buffering
        termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}