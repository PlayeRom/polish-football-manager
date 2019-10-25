
#include <algorithm>
#include <iostream>
#include "TeamComposition.h"
#include "ConioColors.h"
#include "BoxDrawingChars.h"
#include "Tactic.h"

using namespace std;

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

TeamComposition::TeamComposition(const Colors *pColors)
{
    this->pColors = pColors;
}

TeamComposition::TeamComposition(const TeamComposition& orig)
{
}

TeamComposition::~TeamComposition()
{
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
void TeamComposition::draw(const vector<SFootballer> &footballers, int setting, int mode, int clubNumber, bool isTrenning /*= false*/, int sort /* = 0*/)
{
    int i = 0, color = 0;

    pColors->textcolor(GREEN);
    wcout << endl << L"Lp.   Zawodnik         Po.  ";

    pColors->textcolor(LIGHTBLUE);
    wcout << L"B  ";

    pColors->textcolor(MAGENTA);
    wcout << L"O  ";

    pColors->textcolor(LIGHTCYAN);
    wcout << L"P  ";

    pColors->textcolor(LIGHTGREEN);
    wcout << L"N  ";

    pColors->textcolor(GREEN);
    if (isTrenning) { // sklad do treningu
        wcout << L"Trening";
        color = LIGHTGRAY;
        pColors->textcolor(color);
    }
    else { // sklad normalnie
        wcout << L"Morale  For. Kon. Gole";
        if (sort == 18) {
            // sortowanie po wygasnieciu kontraktu
            wcout << L"  Wygasa za:";
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
            strWhatTrain = getTrenningName(footballer.data[1], color);
        }

        wprintf(L"\n\r%2d.", footballer.data[0]); // numer lp.

        drawSpecialEvents(footballer, color);

        // common printf:
        wprintf(
            L"%3ls%-15ls %lc  %2d %2d %2d %2d  %-7ls",
            footballer.name,
            footballer.surname,
            getFootballerPosition(footballer.data[2]),
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
                wprintf(L" %6d dni", footballer.data[18]);
            }

            if (!isTrenning && mode == 20 && sort == 0) {
                printPlayerCounter++;
                switch (printPlayerCounter) {
                    case 1: {
                        pColors->textcolor(WHITE);
                        wcout << L" <- Skład";
                        break;
                    }
                    case 12: {
                        pColors->textcolor(YELLOW);//BROWN);
                        wcout << L" <- Rezerwowi";
                        break;
                    }
                    case 17: {
                        pColors->textcolor(LIGHTGRAY);
                        wcout << L" <- Pozostali";
                        break;
                    }
                }
            }
        }
    }
}

void TeamComposition::sortFootballers(int sort, vector<SFootballer> &footballers)
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

int TeamComposition::getTextColor(int setting, int mode, int settingCount, bool isTrenning, int sort, int color)
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

void TeamComposition::drawSpecialEvents(const SFootballer &footballer, int color)
{
    if (footballer.data[15] > 0 || footballer.data[19]) { // kontuzjowany
        pColors->textbackground(RED);
        pColors->textcolor(LIGHTGRAY);
        wcout << L"Ko";
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
        wcout << L"T ";
        pColors->textcolor(color);
    }
    else {
        wcout << L"  ";
    }
}

wchar_t TeamComposition::getFootballerPosition(int pos)
{
    switch (pos) {
        case PLAYERS_POS_B: return L'B';
        case PLAYERS_POS_O: return L'O';
        case PLAYERS_POS_P: return L'P';
        case PLAYERS_POS_N: return L'N';
        default:            return L' ';
    }
}

wstring TeamComposition::getTrenningName(int watTrain, int &color)
{
    wstring result;
    switch (watTrain) {
        case PLAYERS_TRENNING_NONE: {
            result = L"Nie trenuje";
            color = LIGHTGRAY;
            pColors->textcolor(color);
            break;
        }
        case PLAYERS_TRENNING_B: {
            result = L"Bramkarze";
            color = LIGHTBLUE;
            pColors->textcolor(color);
            break;
        }
        case PLAYERS_TRENNING_O: {
            result = L"Obrona";
            color = MAGENTA;
            pColors->textcolor(color);
            break;
        }
        case PLAYERS_TRENNING_P: {
            result = L"Pomoc";
            color = LIGHTCYAN;
            pColors->textcolor(color);
            break;
        }
        case PLAYERS_TRENNING_N: {
            result = L"Atak";
            color = LIGHTGREEN;
            pColors->textcolor(color);
            break;
        }
    }

    return result;
}

wstring TeamComposition::getMorale(int value)
{
    switch (value) {
        case PLAYERS_MORALE_FATAL:
            return L"Fatalne";

        case PLAYERS_MORALE_BAD:
            return L"Złe";

        case PLAYERS_MORALE_LOW:
            return L"Niskie";

        case PLAYERS_MORALE_MIDDLE:
            return L"Średnie";

        case PLAYERS_MORALE_GOOD:
            return L"Dobre";

        case PLAYERS_MORALE_VGOOD:
            return L"B.dobre";

        case PLAYERS_MORALE_SUPER:
            return L"Super";

        default:
            return L"Error";
    }
}

/////////////// stare/oryginalne wyswietlanie skladu

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
/*void TeamComposition::draw(vector<SFootballer> &footballers, int setting, int mode, int clubNumber, bool isTrenning, int sort)
{
    int i = 0, color = 0, displayedCaouner = 0;

    pColors->textcolor(GREEN);
    wcout << endl << L"Lp.   Zawodnik         Po.  ";

    pColors->textcolor(LIGHTBLUE);
    wcout << L"B  ";

    pColors->textcolor(MAGENTA);
    wcout << L"O  ";

    pColors->textcolor(LIGHTCYAN);
    wcout << L"P  ";

    pColors->textcolor(LIGHTGREEN);
    wcout << L"N  ";

    pColors->textcolor(GREEN);
    if (isTrenning) { // sklad do treningu
        wcout << L"Trening";
        color = LIGHTGRAY;
        pColors->textcolor(color);
    }
    else { // sklad normalnie
        wcout << L"Morale  For. Kon. Gole";
        if (sort == 18) {
            // sortowanie po wygasnieciu kontraktu
            wcout << L"  Wygasa za:";
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

    switch (sort) {
        case 3:
        case 4:
        case 5:
        case 6: { // sort wg umiejetnosci
            i = 21; // 21, bo 21-1 = 20, czyli max umiejetnosci, -1 nastapi w petli w pierwszej iteracji
            mode = 1; // w sortowaniu dazymy od 20 do 1 (max -> min)
            break;
        }
        case 7: { // sort wg morale
            i = PLAYERS_MORALE_SUPER + 1; // od najwyzszego morale 3 do najmniejszego - 3
            mode = PLAYERS_MORALE_FATAL;
            break;
        }
        case 9: { // wg formy
            i = 11; //od 10 do 1
            mode = 1;
            break;
        }
        case 11: { // sort wg kondycji
            i = 101; // 101 bo 100 - 1 = 100 , czyli max kondycja
            mode = 0; // w sortowaniu dazymy od 100 do 0
            break;
        }
        case 16: { // sort wg goli
            i = 50; // 50 goli powinno wystarczyc
            mode = 0;
            break;
        }
        case 18: { // sort wg wygascienia kontraktu
            i = 1; // od 0 dni, do 1095, czyli przyrostowo
            mode = 1095;
            break;
        }
    }

    if (sort > 0) {
        color = LIGHTGRAY;
        pColors->textcolor(color);
    }

    int printPlayerCounter = 0;
    while (i != mode) {
        if (sort == 0 || sort == 18) { // bez sortowania lub po dniach wygrasniecia kontraktu
            i++;
        }
        else {
            i--;
        }

        if (mode == 16 || i == 12) { // rezerwowi
            color = YELLOW;//BROWN;
            pColors->textcolor(color);
        }

        if (mode == 40 || sort > 0) {
            color = LIGHTGRAY;
            pColors->textcolor(color);
        }

        for (size_t index = 0; index < footballers.size(); index++) {
            SFootballer &footballer = footballers[index];

            if (i == footballer.data[sort] && clubNumber == footballer.data[22]) {
                displayedCaouner++; // licznik wyswietlonych zawodnikow

                wprintf(L"\n\r%2d.", footballer.data[0]); // numer na koszulca, lp

                drawSpecialEvents(footballer, color);

                if (isTrenning) {
                    wstring strWhatTrain = getTrenningName(footballer.data[1], color);
                    wprintf(
                        L"%3ls%-15ls %lc  %2d %2d %2d %2d  %-7ls",
                        footballer.imie,
                        footballer.nazwisko,
                        getFootballerPosition(footballer.data[2]),
                        footballer.data[3],
                        footballer.data[4],
                        footballer.data[5],
                        footballer.data[6],
                        strWhatTrain.c_str()
                    );
                }
                else {
                    wprintf(
                        L"%3ls%-15ls %lc  %2d %2d %2d %2d  %-7ls  %2d  %3d%%  %2d",
                        footballer.imie,
                        footballer.nazwisko,
                        getFootballerPosition(footballer.data[2]),
                        footballer.data[3],
                        footballer.data[4],
                        footballer.data[5],
                        footballer.data[6],
                        getMorale(footballer.data[7]).c_str(),
                        footballer.data[9],
                        footballer.data[11],
                        footballer.data[16]
                    );

                    if (mode == 20) {
                        printPlayerCounter++;
                        switch (printPlayerCounter) {
                            case 1: {
                                pColors->textcolor(WHITE);
                                wcout << L" <- Skład";
                                break;
                            }
                            case 12: {
                                pColors->textcolor(YELLOW);//BROWN);
                                wcout << L" <- Rezerwowi";
                                break;
                            }
                            case 17: {
                                pColors->textcolor(LIGHTGRAY);
                                wcout << L" <- Pozostali";
                                break;
                            }
                        }
                    }
                }

                if (sort == 18) {
                    wprintf(L" %6d dni", footballer.data[18]);
                }

                if (displayedCaouner > 19) {
                    break;
                }
            }
        }

        if (displayedCaouner > 19) {
            break;
        }

        if (!isTrenning && sort == 0) {
            if (i == 1) {
                color = MAGENTA;
                pColors->textcolor(MAGENTA);
            }

            if ((i == 5) &&
                (
                    setting == T4_4_2 || setting == T4_4_2_DEF || setting == T4_4_2_ATT ||
                    setting == T4_4_2_DIA || setting == T4_2_4 || setting == T4_3_3 || setting == T4_5_1
                )
            ) {
                color = LIGHTCYAN;
                pColors->textcolor(color);
            }
            else if ((i == 4) && (setting == T3_4_3 || setting == T3_5_2 || setting == T3_5_2_DEF || setting == T3_5_2_ATT)) {
                color = LIGHTCYAN;
                pColors->textcolor(color);
            }
            else if ((i == 6) && (setting == T5_3_2 || setting == T5_3_2_DEF || setting == T5_3_2_ATT)) {
                color = LIGHTCYAN;
                pColors->textcolor(color);
            }

            if ((i == 7) && (setting == T4_2_4)) {
                color = LIGHTGREEN;
                pColors->textcolor(color);
            }
            else if ((i == 8) && (setting == T4_3_3 || setting == T3_4_3)) {
                color = LIGHTGREEN;
                pColors->textcolor(color);
            }
            else if ((i == 10) && (setting == T4_5_1)) {
                color = LIGHTGREEN;
                pColors->textcolor(color);
            }
            else if ((i == 9) &&
                (
                    setting == T4_4_2 || setting == T4_4_2_DEF || setting == T4_4_2_ATT || setting == T4_4_2_DIA || setting == T3_5_2 ||
                    setting == T3_5_2_DEF || setting == T3_5_2_ATT || setting == T5_3_2 || setting == T5_3_2_DEF || setting == T5_3_2_ATT
                )
            ) {
                color = LIGHTGREEN;
                pColors->textcolor(color);
            }

            if (i == 16) {
                color = LIGHTGRAY;
                pColors->textcolor(color);
            }
        }
        else {
            color = LIGHTGRAY;
            pColors->textcolor(color);
        }
    }
}
*/
