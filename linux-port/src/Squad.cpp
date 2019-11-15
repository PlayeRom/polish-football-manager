
#include <algorithm>
#include <iostream>
#include "Squad.h"
#include "ConioColors.h"
#include "BoxDrawingChars.h"
#include "Tactic.h"

/**
 * Sortuj od najmniejszej liczby porzadkowej
 * @param footballer1
 * @param footballer2
 * @return
 */
bool compareByNumber(const SFootballer &footballer1, const SFootballer &footballer2)
{
    return footballer1.data[0] < footballer2.data[0];
}

/**
 * Sortujod najwiejszej wartosci
 *
 * @param footballer1
 * @param footballer2
 * @return
 */
bool compareByGoalkeeperSkills(const SFootballer &footballer1, const SFootballer &footballer2)
{
    return footballer1.data[3] > footballer2.data[3];
}

bool compareByDefenseSkills(const SFootballer &footballer1, const SFootballer &footballer2)
{
    return footballer1.data[4] > footballer2.data[4];
}

bool compareByMiddlefieldSkills(const SFootballer &footballer1, const SFootballer &footballer2)
{
    return footballer1.data[5] > footballer2.data[5];
}

bool compareByAttacksSkills(const SFootballer &footballer1, const SFootballer &footballer2)
{
    return footballer1.data[6] > footballer2.data[6];
}

bool compareByMorale(const SFootballer &footballer1, const SFootballer &footballer2)
{
    return footballer1.data[7] > footballer2.data[7];
}

bool compareByForm(const SFootballer &footballer1, const SFootballer &footballer2)
{
    return footballer1.data[9] > footballer2.data[9];
}

bool compareByCondition(const SFootballer &footballer1, const SFootballer &footballer2)
{
    return footballer1.data[11] > footballer2.data[11];
}

bool compareByGoals(const SFootballer &footballer1, const SFootballer &footballer2)
{
    return footballer1.data[16] > footballer2.data[16];
}

bool compareByContractDaysLeft(const SFootballer &footballer1, const SFootballer &footballer2)
{
    return footballer1.data[18] < footballer2.data[18];
}

Squad::Squad(const Colors *pColors, Language *pLang)
{
    this->pColors = pColors;
    this->pLang = pLang;
}

/**
 * Gdy jest sortowanie, to nie ma stronicowania
 *
 * @param footballers Tablica zawodnikow korzy beda wysietlani
 * @param setting Usatwienie zespolu wyswietlanego
 * @param mode Np. 11 bo trzeba wyswietlic 11 zawodnikow dla taktyki, 16 - rezerwowi, 20 lub 40 dla pełnego składu i treningu
 * @param who -1 - dla treningu, 0 - taktyka moja, > 0 - numer klubu rywala
 * @param sort Sposob sortowanie, 0 - po numerze lp lub jeden z indeksow z SFootballer.dane[...]
 *  0 - ws lp - domyślny,
 *  3 - wg umiejętności B
 *  4 - wg umiejętności O
 *  5 - wg umiejętności P
 *  6 - wg umiejętności N
 *  7 - wg morale
 *  9 - wg formy
 * 11 - wg kondycji
 * 16 - wg goli
 * 18 - wg kolejności wygaśnięcia kontraktu
 */
void Squad::draw(const vector<SFootballer> &footballers, int setting, int mode, int clubNumber, bool isTrenning /*= false*/, int sort /* = 0*/)
{
    int i = 0, color = 0;

    pColors->textcolor(GREEN);
    wcout << endl << pLang->get(L"No    Footballer       Po.  ");

    pColors->textcolor(LIGHTBLUE);
    wcout << pLang->get(L"G") << L"  ";

    pColors->textcolor(MAGENTA);
    wcout << pLang->get(L"D") << L"  ";

    pColors->textcolor(LIGHTCYAN);
    wcout <<pLang->get( L"M") << L"  ";

    pColors->textcolor(LIGHTGREEN);
    wcout << pLang->get(L"A") << L"  ";

    pColors->textcolor(GREEN);
    if (isTrenning) { // sklad do treningu
        wcout << pLang->get(L"Training");
        color = LIGHTGRAY;
        pColors->textcolor(color);
    }
    else { // sklad normalnie
        wcout << pLang->get(L"Morale  For. Con. Goals");
        if (sort == 18) {
            // sortowanie po wygasnieciu kontraktu
            wcout << pLang->get(L"  Expires in:");
        }
        color = LIGHTBLUE;
        pColors->textcolor(color);
    }

    if (mode == 16) {
        i = 11; //dla pokazania rezerwowych w taktyce (zawsze bez sortowania)
    }
    else if (mode == 40) {
        i = 20; //dla dalej w Składzie (bez sortowania)
    }

    if (sort > 0) {
        color = LIGHTGRAY;
        pColors->textcolor(color);
    }

    // wez zawodnikow tylko ze wskaznego klubu, wiec dzialamy/sortujemy tylko na tymczasowym wektorze
    vector<SFootballer> tmpFootballers;
    for (size_t index = 0; index < footballers.size(); index++) {
        if (clubNumber == footballers[index].data[22]) {
            tmpFootballers.push_back(footballers[index]);
        }
    }

    sortFootballers(sort, tmpFootballers);

    int printPlayerCounter = 0;
    int settingCount = 1;

    for (size_t index = i; index < tmpFootballers.size() && index < mode; index++, settingCount++) {
        const SFootballer &footballer = tmpFootballers[index];

        int newColor = getTextColor(setting, mode, settingCount, isTrenning, sort, color);
        if (newColor != color) {
            color = newColor;
            pColors->textcolor(color);
        }

        wstring strWhatTrain;
        if (isTrenning) {
            // musi byc tutaj aby numer lp. był w odpowiednim kolorze
            strWhatTrain = getTrainingName(footballer.data[1], color);
        }

        wprintf(L"\n\r%2d.", footballer.data[0]); // numer lp.

        drawSpecialEvents(footballer, color);

        // common printf:
        wprintf(
            L"%3ls%-15ls %ls  %2d %2d %2d %2d  %-7ls",
            footballer.name,
            footballer.surname,
            getFootballerPosition(footballer.data[2]).c_str(),
            footballer.data[3],
            footballer.data[4],
            footballer.data[5],
            footballer.data[6],
            (isTrenning ? strWhatTrain : getMorale(footballer.data[7])).c_str()
        );

        if (!isTrenning) {
            wprintf(
                L"  %2d  %3d%%  %2d",
                footballer.data[9],
                footballer.data[11],
                footballer.data[16]
            );

            if (sort == 18) {
                wprintf(pLang->get(L" %6d days").c_str(), footballer.data[18]);
            }

            if (!isTrenning && mode == 20 && sort == 0) {
                printPlayerCounter++;
                switch (printPlayerCounter) {
                    case 1: {
                        pColors->textcolor(WHITE);
                        wcout << L" <- " << pLang->get(L"Playing");
                        break;
                    }
                    case 12: {
                        pColors->textcolor(YELLOW);//BROWN);
                        wcout << L" <- " << pLang->get(L"Reserve");
                        break;
                    }
                    case 17: {
                        pColors->textcolor(LIGHTGRAY);
                        wcout << L" <- " << pLang->get(L"Other");
                        break;
                    }
                }
            }
        }
    }
}

void Squad::sortFootballers(int sort, vector<SFootballer> &footballers)
{
    switch (sort) {
        case 3:
            std::sort(footballers.begin(), footballers.end(), compareByGoalkeeperSkills);
            break;

        case 4:
            std::sort(footballers.begin(), footballers.end(), compareByDefenseSkills);
            break;

        case 5:
            std::sort(footballers.begin(), footballers.end(), compareByMiddlefieldSkills);
            break;

        case 6:
            std::sort(footballers.begin(), footballers.end(), compareByAttacksSkills);
            break;

        case 7:
            std::sort(footballers.begin(), footballers.end(), compareByMorale);
            break;

        case 9:
            std::sort(footballers.begin(), footballers.end(), compareByForm);
            break;

        case 11:
            std::sort(footballers.begin(), footballers.end(), compareByCondition);
            break;

        case 16:
            std::sort(footballers.begin(), footballers.end(), compareByGoals);
            break;

        case 18:
            std::sort(footballers.begin(), footballers.end(), compareByContractDaysLeft);
            break;

        case 0:
        default:
            std::sort(footballers.begin(), footballers.end(), compareByNumber);
            break;
    }
}

int Squad::getTextColor(int setting, int mode, int settingCount, bool isTrenning, int sort, int color)
{
    if (!isTrenning && sort == 0) {
        if (settingCount == 1) { // bramkarz
            color = LIGHTBLUE;
        }

        if (settingCount == 2) {
            color = MAGENTA;
        }

        if (settingCount == 6 &&
            (
                setting == T4_4_2 || setting == T4_4_2_DEF || setting == T4_4_2_ATT ||
                setting == T4_4_2_DIA || setting == T4_2_4 || setting == T4_3_3 || setting == T4_5_1
            )
        ) {
            color = LIGHTCYAN;
        }
        else if (settingCount == 5 &&
            (setting == T3_4_3 || setting == T3_5_2 || setting == T3_5_2_DEF || setting == T3_5_2_ATT)
        ) {
            color = LIGHTCYAN;
        }
        else if (settingCount == 7 && (setting == T5_3_2 || setting == T5_3_2_DEF || setting == T5_3_2_ATT)) {
            color = LIGHTCYAN;
        }

        if (settingCount == 8 && (setting == T4_2_4)) {
            color = LIGHTGREEN;
        }
        else if (settingCount == 9 && (setting == T4_3_3 || setting == T3_4_3)) {
            color = LIGHTGREEN;
        }
        else if (settingCount == 11 && (setting == T4_5_1)) {
            color = LIGHTGREEN;
        }
        else if (settingCount == 10 &&
            (
                setting == T4_4_2 || setting == T4_4_2_DEF || setting == T4_4_2_ATT || setting == T4_4_2_DIA ||
                setting == T3_5_2 || setting == T3_5_2_DEF || setting == T3_5_2_ATT || setting == T5_3_2 ||
                setting == T5_3_2_DEF || setting == T5_3_2_ATT
            )
        ) {
            color = LIGHTGREEN;
        }

        if (settingCount == 17) {
            color = LIGHTGRAY;
        }

        if (mode == 16 || settingCount == 12) { // rezerwowi
            color = YELLOW;//BROWN;
        }
    }

    if (mode == 40 || sort > 0) {
        color = LIGHTGRAY;
    }

    return color;
}

void Squad::drawSpecialEvents(const SFootballer &footballer, int color)
{
    if (footballer.data[15] > 0 || footballer.data[19]) { // kontuzjowany
        pColors->textbackground(RED);
        pColors->textcolor(LIGHTGRAY);
        wcout << pLang->get(L"In"); // injury
        pColors->textbackground(BLACK);
        pColors->textcolor(color);
    }
    else if (footballer.data[14] == 1) { // czerwona kartka
        pColors->textcolor(RED);
        wcout << BOX_FULL_BLOCK << L" ";
        pColors->textcolor(color);
    }
    else if (footballer.data[13] == 1) { // zolta kartka
        pColors->textcolor(YELLOW);
        wcout << BOX_FULL_BLOCK << L" ";
        pColors->textcolor(color);
    }
    else if (footballer.data[13] == 2) { // dwie zolte kartki
        pColors->textcolor(YELLOW);
        wcout << BOX_FULL_BLOCK << BOX_FULL_BLOCK;
        pColors->textcolor(color);
    }
    else if (footballer.data[17] == 1) { // wystawiony na transfer
        pColors->textcolor(LIGHTMAGENTA);
        wcout << pLang->get(L"T") << L" ";
        pColors->textcolor(color);
    }
    else {
        wcout << L"  ";
    }
}

const wstring Squad::getFootballerPosition(int pos)
{
    switch (pos) {
        case PLAYERS_POS_B: return pLang->get(L"G");
        case PLAYERS_POS_O: return pLang->get(L"D");
        case PLAYERS_POS_P: return pLang->get(L"M");
        case PLAYERS_POS_N: return pLang->get(L"A");
        default:            return pLang->get(L" ");
    }
}

const wstring Squad::getTrainingName(int watTrain, int &color)
{
    wstring result;
    switch (watTrain) {
        case PLAYERS_TRAINING_NONE: {
            result = pLang->get(L"Not training");
            color = LIGHTGRAY;
            pColors->textcolor(color);
            break;
        }
        case PLAYERS_TRAINING_B: {
            result = pLang->get(L"Goalkeepers");
            color = LIGHTBLUE;
            pColors->textcolor(color);
            break;
        }
        case PLAYERS_TRAINING_O: {
            result = pLang->get(L"Defense");
            color = MAGENTA;
            pColors->textcolor(color);
            break;
        }
        case PLAYERS_TRAINING_P: {
            result = pLang->get(L"Midfield");
            color = LIGHTCYAN;
            pColors->textcolor(color);
            break;
        }
        case PLAYERS_TRAINING_N: {
            result = pLang->get(L"Attack");
            color = LIGHTGREEN;
            pColors->textcolor(color);
            break;
        }
    }

    return result;
}

const wstring Squad::getMorale(int value)
{
    switch (value) {
        case PLAYERS_MORALE_FATAL:
            return pLang->get(L"Fatal");

        case PLAYERS_MORALE_BAD:
            return pLang->get(L"Bad");

        case PLAYERS_MORALE_LOW:
            return pLang->get(L"Low");

        case PLAYERS_MORALE_MIDDLE:
            return pLang->get(L"Middle");

        case PLAYERS_MORALE_GOOD:
            return pLang->get(L"Good");

        case PLAYERS_MORALE_VGOOD:
            return pLang->get(L"V.good");

        case PLAYERS_MORALE_SUPER:
            return pLang->get(L"Super");

        default:
            return pLang->get(L"Error");
    }
}
