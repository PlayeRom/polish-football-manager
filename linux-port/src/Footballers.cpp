
#include <stdio.h>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include "Footballers.h"

Footballers::Footballers() {
}

Footballers::Footballers(const Footballers& orig) {
}

Footballers::~Footballers() {
}

/**
 * Load for initial file
 */
void Footballers::initialPlayerTeam(int clubId)
{
    playerTeam.clear();

    FILE *f = fopen(FILE_FOOTBALLERS, "rb");
    if (!f) {
        char message[512];
        snprintf(message, 512, ERROR_OPEN_FILE, FILE_FOOTBALLERS);
        throw std::invalid_argument(message);
    }

    SFootballer footballer;

    while (fread(&footballer, sizeof (SFootballer), 1, f) == 1) {
        if (footballer.data[22] == clubId) {
            playerTeam.push_back(footballer);
        }
    }
    fclose(f);
}

void Footballers::initialRivals()
{
    rivals.clear();

    FILE *f = fopen(FILE_FOOTBALLERS_RIVALS, "rb");
    if (!f) {
        char message[512];
        snprintf(message, 512, ERROR_OPEN_FILE, FILE_FOOTBALLERS_RIVALS);
        throw std::invalid_argument(message);
    }

    SFootballer footballer;
    while (fread(&footballer, sizeof (SFootballer), 1, f) == 1) {
        rivals.push_back(footballer);
    }
    fclose(f);
}

void Footballers::initialTransfers()
{
    transfers.clear();

    FILE *f = fopen(FILE_FOOTBALLERS_TRANSFER, "rb");
    if (!f) {
        char message[512];
        snprintf(message, 512, ERROR_OPEN_FILE, FILE_FOOTBALLERS_TRANSFER);
        throw std::invalid_argument(message);
    }

    SFootballer footballer;
    while (fread(&footballer, sizeof (SFootballer), 1, f) == 1) {
        transfers.push_back(footballer);
    }
    fclose(f);
}

/**
 * Load saved from game file
 */
void Footballers::loadPlayerTeam()
{
    playerTeam.clear();

    FILE *f = fopen(FILE_SAVE_FOOTBALLERS_MANAGER, "rb");
    if (!f) {
        char message[512];
        snprintf(message, 512, ERROR_OPEN_FILE, FILE_SAVE_FOOTBALLERS_MANAGER);
        throw std::invalid_argument(message);
    }
    SFootballer footballer;
    while (fread(&footballer, sizeof (SFootballer), 1, f) == 1) {
        playerTeam.push_back(footballer);
    }
    fclose(f);
}

/**
 * Load saved from game file
 */
void Footballers::loadRivals()
{
    rivals.clear();

    FILE *f = fopen(FILE_SAVE_FOOTBALLERS_RIVAL, "rb");
    if (!f) {
        char message[512];
        snprintf(message, 512, ERROR_OPEN_FILE, FILE_SAVE_FOOTBALLERS_RIVAL);
        throw std::invalid_argument(message);
    }

    SFootballer footballer;
    while (fread(&footballer, sizeof (SFootballer), 1, f) == 1) {
        rivals.push_back(footballer);
    }
    fclose(f);
}

/**
 * Load saved from game file
 */
void Footballers::loadTransfers()
{
    transfers.clear();

    FILE *f = fopen(FILE_SAVE_FOOTBALLERS_TRANSFER, "rb");
    if (!f) {
        char message[512];
        snprintf(message, 512, ERROR_OPEN_FILE, FILE_SAVE_FOOTBALLERS_TRANSFER);
        throw std::invalid_argument(message);
    }

    SFootballer footballer;
    while (fread(&footballer, sizeof (SFootballer), 1, f) == 1) {
        transfers.push_back(footballer);
    }
    fclose(f);
}

/**
 * Save to game file
 */
void Footballers::savePlayerTeam()
{
    FILE *f = fopen(FILE_SAVE_FOOTBALLERS_MANAGER, "wb");
    fclose(f);

    f = fopen(FILE_SAVE_FOOTBALLERS_MANAGER, "ab");
    for (size_t i = 0; i < playerTeam.size(); i++) {
        fwrite(&playerTeam[i], sizeof (SFootballer), 1, f);
    }
    fclose(f);
}

void Footballers::saveRivals()
{
    FILE *f = fopen(FILE_SAVE_FOOTBALLERS_RIVAL, "wb");
    fclose(f);

    f = fopen(FILE_SAVE_FOOTBALLERS_RIVAL, "ab");
    for (size_t i = 0; i < rivals.size(); i++) {
        fwrite(&rivals[i], sizeof (SFootballer), 1, f);
    }
    fclose(f);
}

void Footballers::saveTransfers()
{
    FILE *f = fopen(FILE_SAVE_FOOTBALLERS_TRANSFER, "wb");
    fclose(f);

    f = fopen(FILE_SAVE_FOOTBALLERS_TRANSFER, "ab");
    for (size_t i = 0; i < transfers.size(); i++) {
        fwrite(&transfers[i], sizeof (SFootballer), 1, f);
    }
    fclose(f);
}

void Footballers::changeTransferList()
{
    for (size_t i = 0; i < transfers.size(); i++) {
        SFootballer &footballer = transfers[i];
        // losuj kluczowe umiejetności zawodnika od 1 do 15
        switch (footballer.data[2]) {
            case PLAYERS_POS_B: footballer.data[3] = (rand() % 15) + 1; break; //random(15)+1;
            case PLAYERS_POS_O: footballer.data[4] = (rand() % 15) + 1; break; //random(15)+1;
            case PLAYERS_POS_P: footballer.data[5] = (rand() % 15) + 1; break; //random(15)+1;
            case PLAYERS_POS_N: footballer.data[6] = (rand() % 15) + 1; break; //random(15)+1;
        }
        footballer.data[7] = (rand() % 7) - 3; //random(7)-3; // losuj morale zawodnika (-3 do 3)
        footballer.data[9] = (rand() % 6) + 5; //(rand() % 6)+5; // losuj formę zawodnika (5 - 10)

        //cena zawodnika, ustal wartość zawodnika w zaleznosci od jego umiejetnosci * 5
        footballer.finances[0] = 0;
        switch (footballer.data[2]) {
            case PLAYERS_POS_B: footballer.finances[0] = footballer.data[3] * 5; break;
            case PLAYERS_POS_O: footballer.finances[0] = footballer.data[4] * 5; break;
            case PLAYERS_POS_P: footballer.finances[0] = footballer.data[5] * 5; break;
            case PLAYERS_POS_N: footballer.finances[0] = footballer.data[6] * 5; break;
        }

        // wez kluczowa umiejetnosc zawodnika
        int mainSkill = footballer.data[footballer.data[2] + 2];
        float salary = 6000.0;

        if (mainSkill == 20) salary = 36000.0;
        else if (mainSkill >= 17) salary = 27000.0;
        else if (mainSkill >= 15) salary = 18000.0;
        else if (mainSkill >= 12) salary = 14000.0;
        else if (mainSkill >= 10) salary = 9000.0;
        else if (mainSkill >= 5) salary = 7000.0;

        // podbij cenę zawodnika o kolejne parametry
        footballer.finances[0] += footballer.data[3] + footballer.data[4] + footballer.data[5] + footballer.data[6];
        footballer.finances[0] *= salary; // x salary
        footballer.finances[0] += footballer.data[9] * 10000.0; // x forma
    }

    saveTransfers();
}

void Footballers::moraleUp(SFootballer &footballer, int modifierValue)
{
    footballer.data[8] += modifierValue;
    if (footballer.data[8] >= MORALE_THRESHOLD) {
        footballer.data[7]++;
        footballer.data[8] = 0;
    }

    if (footballer.data[7] > PLAYERS_MORALE_SUPER) {
        footballer.data[7] = PLAYERS_MORALE_SUPER;
    }
}

void Footballers::moraleDown(SFootballer &footballer, int modifierValue)
{
    footballer.data[8] -= modifierValue;
    if (footballer.data[8] <= -MORALE_THRESHOLD) {
        footballer.data[7]--;
        footballer.data[8] = 0;
    }

    if (footballer.data[7] < PLAYERS_MORALE_FATAL) {
        footballer.data[7] = PLAYERS_MORALE_FATAL;
    }
}

bool Footballers::isNotAllowedTeamPlayer()
{
    for (size_t i = 0; i < playerTeam.size(); i++) {
        SFootballer &footballer = playerTeam[i];
        if (footballer.data[0] < 12 && // zawodnik wystawiony w pierwszej 11-ce
            (footballer.data[13] == 2 || // ma 2 zolte kartki
             footballer.data[14] == 1 || // ma czerwona kartke
             footballer.data[15] > 7) // ma wiecej niz 7 dni do wyleczenia kontuzji
        ) {
            // jest niedozowolony gracz
            return true;
        }
    }

    return false;
}

bool compareFootballers(const SFootballer &footballer1, const SFootballer &footballer2)
{
    return footballer1.data[0] > footballer2.data[0];
}

void Footballers::sortPlayersTeam()
{
    std::sort(playerTeam.begin(), playerTeam.end(), compareFootballers);
}
