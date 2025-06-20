
#include <iostream>
#include "Tactic.h"
#include "ConioColors.h"
#include "BoxDrawingChars.h"

Tactic::Tactic(const Colors *pColors, Language *pLang)
{
    this->pColors = pColors;
    this->pLang = pLang;
}

/**
 *
 * @param setting Ustawienie zespołu
 * @param isPlayerTeam  true - rysujemy dla zespołu gracza, false - rysujemy dla rywala
 */
void Tactic::drawTeamSetting(int setting, bool isPlayerTeam /*= true*/) const
{
    const wstring *settings = getTeamSettings();

    pColors->textBackground(GREEN);
    pColors->textColor(BLACK); //wypis taktyk
    wprintf(L"\n\r %-15ls", settings[setting - 1].c_str());

    //--------------------linia ataku
    pColors->textColor(LIGHTGREEN);
    if (setting == T4_4_2 || setting == T4_4_2_DEF || setting == T4_4_2_ATT || setting == T4_4_2_DIA ||
        setting == T3_5_2 || setting == T3_5_2_DEF || setting == T3_5_2_ATT || setting == T4_2_4 ||
        setting == T5_3_2 || setting == T5_3_2_DEF || setting == T5_3_2_ATT
    ) {
        wcout << endl << L"     10  11     ";
    }
    else if (setting == T3_4_3 || setting == T4_3_3) {
        wcout << endl << L"     9 10 11    ";
    }
    else { // if (setting == T4_5_1) {
        wcout << endl << L"       11       ";
    }

    if (isPlayerTeam) {
        pColors->textBackground(BLACK);
        pColors->textColor(GREEN);
        wcout << pLang->get(L"     U Change formation         P Swap players");
        pColors->textBackground(GREEN);
        pColors->textColor(LIGHTGREEN);
    }

    //--------------------linia ataku
    if (setting == T4_2_4) { //4 w ataku
        wcout << endl << L"  8           9 ";
    }
    else {
        wcout << endl << L"                ";
    }

    pColors->textBackground(BLACK);
    pColors->textColor(GREEN);
    wcout << pLang->get(L"     I Team instructions        R Show reserve");
    pColors->textBackground(GREEN);

    //--------------------------linia pomocy
    pColors->textColor(LIGHTCYAN);
    if (setting == T4_4_2_DIA || setting == T3_5_2_ATT || setting == T5_3_2_ATT) {
        wcout << endl << L"        9       ";
    }
    else if (setting == T4_4_2_ATT) {
        wcout << endl << L"  6           9 ";
    }
    else {
        wcout << endl << L"                ";
    }

    pColors->textBackground(BLACK);
    pColors->textColor(RED);
    wcout << pLang->get(L"     Q Back");
    pColors->textBackground(GREEN);
    pColors->textColor(LIGHTCYAN);

    //--------------------------linia pomocy - srodkowa
    if (setting == T4_4_2)                                  wcout << endl << L"  6   7   8   9 ";
    else if (setting == T3_4_3 || setting == T3_5_2_ATT)    wcout << endl << L"  5   6   7   8 ";
    else if (setting == T4_4_2_DEF)                         wcout << endl << L"  6           9 ";
    else if (setting == T4_4_2_DIA)                         wcout << endl << L"  6           8 ";
    else if (setting == T5_3_2_DEF)                         wcout << endl << L"      8   9     ";
    else if (setting == T4_4_2_ATT || setting == T5_3_2_ATT)wcout << endl << L"      7   8     ";
    else if (setting == T4_2_4)                             wcout << endl << L"      6   7     ";
    else if (setting == T4_3_3)                             wcout << endl << L"     6  7  8    ";
    else if (setting == T5_3_2)                             wcout << endl << L"     7  8  9    ";
    else if (setting == T3_5_2_DEF)                         wcout << endl << L"  5     6     9 ";
    else if (setting == T3_5_2)                             wcout << endl << L"  5  6  7  8  9 ";
    else if (setting == T4_5_1)                             wcout << endl << L"  6  7  8  9 10 ";
    //--------------------------linia pomocy
    if (setting == T4_4_2_DEF || setting == T3_5_2_DEF)     wcout << endl << L"      7   8     ";
    else if (setting == T4_4_2_DIA || setting == T5_3_2_DEF)wcout << endl << L"        7       ";
    else                                                    wcout << endl << L"                ";

    //-----------------------------linia obrony
    pColors->textColor(MAGENTA);
    if (setting == T5_3_2 || setting == T5_3_2_DEF || setting == T5_3_2_ATT) {
        wcout << endl << L"  5           6 ";
    }
    else {
        wcout << endl << L"                ";
    }
    //-----------------------------linia obrony
    if (setting == T4_4_2 || setting == T4_4_2_DEF || setting == T4_4_2_ATT || setting == T4_4_2_DIA ||
        setting == T4_2_4 || setting == T4_3_3 || setting == T4_5_1
    ) {
        wcout << endl << L"  2   3   4   5 ";
    }
    else {
        wcout << endl << L"     2  3  4    ";
    }

    //-----------------------------biala linia pola bramkowego
    pColors->textColor(WHITE);
    wcout << endl
        << L"   "
        << BOX_LIGHT_DOWN_RIGHT
        << BOX_LIGHT_HORIZONTAL
        << BOX_LIGHT_HORIZONTAL
        << BOX_LIGHT_HORIZONTAL
        << BOX_LIGHT_HORIZONTAL
        << BOX_LIGHT_HORIZONTAL
        << BOX_LIGHT_HORIZONTAL
        << BOX_LIGHT_HORIZONTAL
        << BOX_LIGHT_HORIZONTAL
        << BOX_LIGHT_DOWN_LEFT
        << L"   ";

    pColors->textColor(WHITE);
    wcout << endl << L"   " << BOX_LIGHT_VERTICAL;

    ////----------------------------- bramkarz
    pColors->textColor(LIGHTBLUE);
    wcout << L"    1";

    pColors->textColor(WHITE);
    wcout << L"   " << BOX_LIGHT_VERTICAL << L"   ";
    pColors->textBackground(BLACK);
}

/**
 *
 * @param setting Ustawienie zespolu
 * @param clubId ID klubu ktorego dotyczy wykres
 * @param footballers Tablica z zawodnikami klubu
 * @param isRival Czy rysowany wykres dotyczy przeciwnika
 */
void Tactic::drawChart(int setting, int clubId, const vector<SFootballer> &footballers, bool isRival /*= false*/)
{
    int goalkeeper = 0, defense = 0, midfield = 0, attack = 0;

    for (size_t index = 0; index < footballers.size(); index++) {
        const SFootballer &footballer = footballers[index];
        if (clubId == footballer.data[22]) {
            if (footballer.data[0] == 1) {
                goalkeeper = footballer.data[3];
            }

            if (footballer.data[0] == 2 || footballer.data[0] == 3 || footballer.data[0] == 4) {
                defense += footballer.data[4];
            }

            if (footballer.data[0] == 5) {
                if (setting == T4_4_2 || setting == T4_4_2_DEF || setting == T4_4_2_ATT || setting == T4_4_2_DIA ||
                    setting == T4_2_4 || setting == T4_3_3 || setting == T4_5_1 || setting == T5_3_2 ||
                    setting == T5_3_2_DEF || setting == T5_3_2_ATT
                ) {
                    defense += footballer.data[4];
                }
                else {
                    midfield += footballer.data[5];
                }
            }

            if ((footballer.data[0] == 6)) {
                if (setting == T5_3_2 || setting == T5_3_2_DEF || setting == T5_3_2_ATT) {
                    defense += footballer.data[4];
                }
                else {
                    midfield += footballer.data[5];
                }
            }

            if (footballer.data[0] == 7) {
                midfield += footballer.data[5];
            }

            if ((footballer.data[0] == 8)) {
                if (setting == T4_2_4) {
                    attack += footballer.data[6];
                }
                else {
                    midfield += footballer.data[5];
                }
            }

            if ((footballer.data[0] == 9)) {
                if ((setting == T3_4_3 || setting == T4_3_3 || setting == T4_2_4)) {
                    attack += footballer.data[6];
                }
                else {
                    midfield += footballer.data[5];
                }
            }

            if ((footballer.data[0] == 10)) {
                if (setting == T4_5_1) {
                    midfield += footballer.data[5];
                }
                else {
                    attack += footballer.data[6];
                }
            }

            if (footballer.data[0] == 11) {
                attack += footballer.data[6];
            }
        }
    }

    wcout << endl;
    drawBoxes(LIGHTBLUE, pLang->get(L"G"), goalkeeper, 4);
    drawBoxes(MAGENTA, pLang->get(L"D"), defense, 20);
    drawBoxes(LIGHTCYAN, pLang->get(L"M"), midfield, 20);
    drawBoxes(LIGHTGREEN, pLang->get(L"A"), attack, 16);

    if (isRival) {
        pColors->textColor(LIGHTGRAY);
        wcout << L"<- " << pLang->get(L"Rival");
    }
}

void Tactic::drawBoxes(int color, const wstring& sign, int strength, int max)
{
    pColors->textColor(color);
    wcout << sign << L"-";

    int counter = 0;
    for (int i = 5; i <= strength; i += 5) {
        wcout << BOX_FULL_BLOCK;
        counter++;
    }

    for (int i = counter; i < max; i++) {
        pColors->textColor(DARKGRAY);
        wcout << BOX_FULL_BLOCK;
    }
}
