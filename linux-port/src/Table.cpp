
#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <stdexcept>
#include "Table.h"
#include "BoxDrawingChars.h"
#include "ConioColors.h"

using namespace std;

Table::Table(const Colors *pColors)
{
    this->pColors = pColors;
}

Table::Table(const Table& orig)
{
}

Table::~Table()
{
}

void Table::createTable()
{
    for (int i = 0; i < MAX_CLUBS; i++) {
        table[i].num = i + 1;
        memset(table[i].data, 0, 9 * sizeof(int));
    }
}

/**
 * Save the table to file
 */
void Table::save()
{
    FILE *f = fopen(FILE_SAVE_TABLE, "wb");
    fclose(f); // wyczyść stary plik

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
        if (table[i].num == clubNumber) {
            return table[i].data[8];
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
        if (table[i].data[8] == tablePosition) {
            return table[i].num; // wez klub na pozycji lata
        }
    }

    return 0;
}

/**
 * Draw table
 * @param clubId Id klubu gracza
 * @param clubsNames
 */
void Table::drawTable(int clubId, const PlayerClub *club)
{
    pColors->textcolor(LIGHTGRAY);
    wcout << BOX_LIGHT_DOWN_RIGHT;
    for (int i = 0; i < 16; i++) {
        wcout << BOX_LIGHT_HORIZONTAL;
    }
    pColors->textcolor(WHITE);
    wcout << L" TABELA - I LIGA ";
    pColors->textcolor(LIGHTGRAY);
    for (int i = 0; i < 6; i++) {
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
    pColors->textcolor(GREEN);
    wcout << L"Lp. Klub                  M ";

    pColors->textcolor(LIGHTGRAY);
    wcout << BOX_LIGHT_VERTICAL;
    pColors->textcolor(GREEN);
    wcout << L"  W  R  P ";

    pColors->textcolor(LIGHTGRAY);
    wcout << BOX_LIGHT_VERTICAL;
    pColors->textcolor(GREEN);
    wcout << L" Gz  Gs ";

    pColors->textcolor(LIGHTGRAY);
    wcout << BOX_LIGHT_VERTICAL;
    pColors->textcolor(GREEN);
    wcout << L" Pkt";

    pColors->textcolor(LIGHTGRAY);
    wcout << BOX_LIGHT_VERTICAL;

    drawTableHorizontalInner();

    for (int i = 0; i < MAX_CLUBS; i++) {
        wcout << endl << BOX_LIGHT_VERTICAL;
        if (table[i].num == clubId) {
            pColors->textcolor(YELLOW);
        }
        // pozycja w tabeli:
        wcout << std::setfill(L' ') << std::setw(2) << table[i].data[8] << L". ";
        // nazwa klubu
        wcout << club->getClubName(table[i].num - 1);
        int clubNameLength = club->getClubName(table[i].num - 1).length();
        int maxNamePlaceholder = 21;
        for (int i = 0; i < maxNamePlaceholder - clubNameLength; i++) {
            wcout << L" ";
        }
        // ilość meczy:
        wcout << std::setfill(L' ') << std::setw(2) << table[i].data[0] << L" " << BOX_LIGHT_VERTICAL << L" ";
        // wygrane mecze:
        wcout << std::setfill(L' ') << std::setw(2) << table[i].data[1] << L" ";
        // remisy:
        wcout << std::setfill(L' ') << std::setw(2) << table[i].data[2] << L" ";
        // przegrane:
        wcout << std::setfill(L' ') << std::setw(2) << table[i].data[3] << L" " << BOX_LIGHT_VERTICAL << L" ";
        // gole zdobyte:
        wcout << std::setfill(L' ') << std::setw(2) << table[i].data[4] << L"  ";
        // gole stracone:
        wcout << std::setfill(L' ') << std::setw(2) << table[i].data[5] << L" " << BOX_LIGHT_VERTICAL << L" ";
        // punkty:
        wcout << std::setfill(L' ') << std::setw(2) << table[i].data[7];

        pColors->textcolor(LIGHTGRAY);
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

            if (round.clubNumbers[i] == tableRef.num) {
                tableRef.data[0]++;
                if (i % 2 == 0) {
                    tableRef.data[4] += clubRef.goalsLeague[roundIndex - 16 + i];
                    tableRef.data[5] += clubRef.goalsLeague[roundIndex - 16 + i + 1];
                    if (clubRef.goalsLeague[roundIndex - 16 + i] > clubRef.goalsLeague[roundIndex - 16 + i + 1]) {
                        tableRef.data[1]++; // win
                        tableRef.data[7] += 3; // points +3
                    }
                    else if (clubRef.goalsLeague[roundIndex - 16 + i] == clubRef.goalsLeague[roundIndex - 16 + i + 1]) {
                        tableRef.data[2]++; // draw
                        tableRef.data[7]++; // points +1
                    }
                    else {
                        tableRef.data[3]++; // defeat
                    }
                }
                else { //nieparzysta
                    tableRef.data[4] += clubRef.goalsLeague[roundIndex - 16 + i];
                    tableRef.data[5] += clubRef.goalsLeague[roundIndex - 16 + i - 1];
                    if (clubRef.goalsLeague[roundIndex - 16 + i] > clubRef.goalsLeague[roundIndex - 16 + i - 1]) {
                        tableRef.data[1]++; // win
                        tableRef.data[7] += 3; // points +3
                    }
                    else if (clubRef.goalsLeague[roundIndex - 16 + i] == clubRef.goalsLeague[roundIndex - 16 + i - 1]) {
                        tableRef.data[2]++; // draw
                        tableRef.data[7]++; // points +1
                    }
                    else {
                        tableRef.data[3]++; // defeat
                    }
                }
                tableRef.data[6] = tableRef.data[4] - tableRef.data[5]; // goal difference

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
