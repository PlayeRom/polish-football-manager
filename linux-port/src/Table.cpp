
#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <stdexcept>
#include "Table.h"
#include "BoxDrawingChars.h"
#include "ConioColors.h"

using namespace std;

Table::Table(const Colors *pColors, Language *pLang)
{
    this->pColors = pColors;
    this->pLang = pLang;
}

void Table::createTable()
{
    for (int i = 0; i < MAX_CLUBS; i++) {
        table[i].clubId = i + 1;
        table[i].numberMatchesPlayed = 0;
        table[i].wins                = 0;
        table[i].draws               = 0;
        table[i].losses              = 0;
        table[i].goalsScored         = 0;
        table[i].goalsLost           = 0;
        table[i].goalsDiff           = 0;
        table[i].points              = 0;
        table[i].tablePosition       = 0;
    }
}

/**
 * Save the table to file
 */
void Table::save()
{
    FILE *f = fopen(FILE_SAVE_TABLE, "wb");
    fclose(f); // clear old file

    f = fopen(FILE_SAVE_TABLE, "ab");
    for (int i = 0; i < MAX_CLUBS; i++) {
        fwrite(&table[i], sizeof (STable), 1, f);
    }
    fclose(f);
}

/**
 * Load table from file
 */
void Table::load()
{
    FILE *f = fopen(FILE_SAVE_TABLE, "rb");
    if (!f) {
        char message[512];
        snprintf(message, 512, ERROR_OPEN_FILE, FILE_SAVE_TABLE);
        throw std::invalid_argument(message);
    }

    int i = 0;
    while (fread(&table[i], sizeof (STable), 1, f) == 1) {
        i++;
    }
    fclose(f);
}

/**
 * Get club position in the table
 *
 * @param clubNumber Club number
 * @return Position in table
 */
int Table::getPositionInTable(int clubNumber)
{
    for (int i = 0; i < MAX_CLUBS; i++) {
        if (table[i].clubId == clubNumber) {
            return table[i].tablePosition;
        }
    }

    return 0;
}

/**
 * Get club number which is in given position in the table
 *
 * @param tablePosition
 * @return Club number
 */
int Table::getClubNumberInPosition(int tablePosition)
{
    for (int i = 0; i < MAX_CLUBS; i++) {
        if (table[i].tablePosition == tablePosition) {
            return table[i].clubId; // wez klub na pozycji w tabeli
        }
    }

    return 0;
}

/**
 * Draw table
 * @param clubId Player's cloub ID
 * @param clubsNames
 */
void Table::drawTable(int clubId, const PlayerClub *club)
{
    pColors->textColor(LIGHTGRAY);
    wcout << BOX_LIGHT_DOWN_RIGHT;
    for (int i = 0; i < 16; i++) {
        wcout << BOX_LIGHT_HORIZONTAL;
    }
    pColors->textColor(WHITE);
    const int maxLabel = 18;
    const wstring label = pLang->cut(pLang->get(L" TABLE - I LEAGUE "), maxLabel);
    wcout << label;
    pColors->textColor(LIGHTGRAY);
    // add missing horizonal according to translations
    for (int i = label.length(); i < maxLabel; i++) {
        wcout << BOX_LIGHT_HORIZONTAL;
    }

    for (int i = 0; i < 5; i++) {
        wcout << BOX_LIGHT_HORIZONTAL;
    }
    wcout << BOX_LIGHT_DOWN_HORIZONTAL;
    for (int i = 0; i < 8; i++) {
        wcout << BOX_LIGHT_HORIZONTAL;
    }
    wcout << BOX_LIGHT_DOWN_HORIZONTAL;
    for (int i = 0; i < 4; i++) {
        wcout << BOX_LIGHT_HORIZONTAL;
    }
    wcout << BOX_LIGHT_DOWN_LEFT << endl << BOX_LIGHT_VERTICAL;
    pColors->textColor(GREEN);
    wcout << pLang->get(L"No  Club                  M ");

    pColors->textColor(LIGHTGRAY);
    wcout << BOX_LIGHT_VERTICAL;
    pColors->textColor(GREEN);
    wcout << pLang->get(L"  W  D  L ");

    pColors->textColor(LIGHTGRAY);
    wcout << BOX_LIGHT_VERTICAL;
    pColors->textColor(GREEN);
    wcout << pLang->get(L" G+  G- ");

    pColors->textColor(LIGHTGRAY);
    wcout << BOX_LIGHT_VERTICAL;
    pColors->textColor(GREEN);
    wcout << pLang->get(L" Pts");

    pColors->textColor(LIGHTGRAY);
    wcout << BOX_LIGHT_VERTICAL;

    drawTableHorizontalInner();

    for (int i = 0; i < MAX_CLUBS; i++) {
        wcout << endl << BOX_LIGHT_VERTICAL;
        if (table[i].clubId == clubId) {
            pColors->textColor(YELLOW);
        }
        // pozycja w tabeli:
        wcout << std::setfill(L' ') << std::setw(2) << table[i].tablePosition << L". ";
        // nazwa klubu
        wcout << club->getClubName(table[i].clubId - 1);
        int clubNameLength = club->getClubName(table[i].clubId - 1).length();
        int maxNamePlaceholder = 21;
        for (int i = 0; i < maxNamePlaceholder - clubNameLength; i++) {
            wcout << L" ";
        }
        // ilość meczy:
        wcout << std::setfill(L' ') << std::setw(2) << table[i].numberMatchesPlayed << L" " << BOX_LIGHT_VERTICAL << L" ";
        // wygrane mecze:
        wcout << std::setfill(L' ') << std::setw(2) << table[i].wins << L" ";
        // remisy:
        wcout << std::setfill(L' ') << std::setw(2) << table[i].draws << L" ";
        // przegrane:
        wcout << std::setfill(L' ') << std::setw(2) << table[i].losses << L" " << BOX_LIGHT_VERTICAL << L" ";
        // gole zdobyte:
        wcout << std::setfill(L' ') << std::setw(2) << table[i].goalsScored << L"  ";
        // gole stracone:
        wcout << std::setfill(L' ') << std::setw(2) << table[i].goalsLost << L" " << BOX_LIGHT_VERTICAL << L" ";
        // punkty:
        wcout << std::setfill(L' ') << std::setw(2) << table[i].points;

        pColors->textColor(LIGHTGRAY);
        wcout << L" " << BOX_LIGHT_VERTICAL;

        if (i == 0) {
            drawTableHorizontalInner();
        }
        else if (i == 12) {
            drawTableHorizontalInner();
        }
    }
    drawTableHorizontalBottom();
}

void Table::drawTableHorizontalInner()
{
    wcout << endl << BOX_LIGHT_VERTICAL_RIGHT;
    for (int i = 0; i < 28; i++) {
        wcout << BOX_LIGHT_HORIZONTAL;
    }
    wcout << BOX_LIGHT_CROSS;
    for (int i = 0; i < 10; i++) {
        wcout << BOX_LIGHT_HORIZONTAL;
    }
    wcout << BOX_LIGHT_CROSS;
    for (int i = 0; i < 8; i++) {
        wcout << BOX_LIGHT_HORIZONTAL;
    }
    wcout << BOX_LIGHT_CROSS;
    for (int i = 0; i < 4; i++) {
        wcout << BOX_LIGHT_HORIZONTAL;
    }
    wcout << BOX_LIGHT_VERTICAL_LEFT;
}

void Table::drawTableHorizontalBottom()
{
    wcout << endl;
    wcout << BOX_LIGHT_UP_RIGHT;
    for (int i = 0; i < 28; i++) {
        wcout << BOX_LIGHT_HORIZONTAL;
    }
    wcout << BOX_LIGHT_UP_HORIZONTAL;
    for (int i = 0; i < 10; i++) {
        wcout << BOX_LIGHT_HORIZONTAL;
    }
    wcout << BOX_LIGHT_UP_HORIZONTAL;
    for (int i = 0; i < 8; i++) {
        wcout << BOX_LIGHT_HORIZONTAL;
    }
    wcout << BOX_LIGHT_UP_HORIZONTAL;
    for (int i = 0; i < 4; i++) {
        wcout << BOX_LIGHT_HORIZONTAL;
    }
    wcout << BOX_LIGHT_UP_LEFT;
}

void Table::updateAfterMatch(const SRound &round, int roundIndex, SClub &clubRef)
{
    for (int i = 0; i < MAX_CLUBS; i++) { // petla po numerach klubow w kolejce
        for (int j = 0; j < MAX_CLUBS; j++) { // petla po klubach w tabeli
            STable &tableRef = table[j];

            if (round.clubNumbers[i] == tableRef.clubId) {
                tableRef.numberMatchesPlayed++;
                if (i % 2 == 0) {
                    tableRef.goalsScored += clubRef.goalsLeague[roundIndex - 16 + i];
                    tableRef.goalsLost += clubRef.goalsLeague[roundIndex - 16 + i + 1];
                    if (clubRef.goalsLeague[roundIndex - 16 + i] > clubRef.goalsLeague[roundIndex - 16 + i + 1]) {
                        tableRef.wins++; // win
                        tableRef.points += 3; // points +3
                    }
                    else if (clubRef.goalsLeague[roundIndex - 16 + i] == clubRef.goalsLeague[roundIndex - 16 + i + 1]) {
                        tableRef.draws++; // draw
                        tableRef.points++; // points +1
                    }
                    else {
                        tableRef.losses++; // defeat
                    }
                }
                else { //nieparzysta
                    tableRef.goalsScored += clubRef.goalsLeague[roundIndex - 16 + i];
                    tableRef.goalsLost += clubRef.goalsLeague[roundIndex - 16 + i - 1];
                    if (clubRef.goalsLeague[roundIndex - 16 + i] > clubRef.goalsLeague[roundIndex - 16 + i - 1]) {
                        tableRef.wins++; // win
                        tableRef.points += 3; // points +3
                    }
                    else if (clubRef.goalsLeague[roundIndex - 16 + i] == clubRef.goalsLeague[roundIndex - 16 + i - 1]) {
                        tableRef.draws++; // draw
                        tableRef.points++; // points +1
                    }
                    else {
                        tableRef.losses++; // defeat
                    }
                }
                tableRef.goalsDiff = tableRef.goalsScored - tableRef.goalsLost; // goal difference

                break;
            }
        }
    }
}

void Table::overrideTable(vector<STable> newTable)
{
    if (newTable.size() != MAX_CLUBS) {
        throw std::range_error("overrideTable: the new table must has 16 elements");
    }

    for (int i = 0; i < MAX_CLUBS; i++) {
        table[i] = newTable[i];
    }
}
