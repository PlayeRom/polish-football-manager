
#include <stdexcept>
#include <stdio.h>
#include <cstring>
#include <cwchar>
#include "PlayerClub.h"

const int defaultTrainingPlan[TRAINING_SLOTS_NUMBER] = {
    TRAINING_B,         // pn
    TRAINING_B,         // wt
    TRAINING_CONDITIONS,// sr
    TRAINING_TACTICS,   // cz
    TRAINING_FREE_KICKS,// pt
    TRAINING_PASSES,    // so
    TRAINING_HOLIDAY,   // n
    TRAINING_O,         // pn
    TRAINING_O,         // wt
    TRAINING_PASSES,    // sr
    TRAINING_CONDITIONS,// cz
    TRAINING_TACTICS,   // pt
    TRAINING_FREE_KICKS,// so
    TRAINING_EMPTY,     // n
    TRAINING_P,         // pn
    TRAINING_P,         // wt
    TRAINING_FREE_KICKS,// sr
    TRAINING_PASSES,    // cz
    TRAINING_CONDITIONS,// pt
    TRAINING_TACTICS,   // so
    TRAINING_EMPTY,     // n
    TRAINING_N,         // pn
    TRAINING_N,         // wt
    TRAINING_EMPTY,     // sr
    TRAINING_EMPTY,     // cz
    TRAINING_EMPTY,     // pt
    TRAINING_EMPTY,     // so
    TRAINING_EMPTY      // n
};

PlayerClub::PlayerClub()
{
}

PlayerClub::PlayerClub(const PlayerClub& orig)
{
}

PlayerClub::~PlayerClub()
{
}

/**
 * Set the club structure with initialize data on the start of game
 */
void PlayerClub::init(const wchar_t* name, const wchar_t* surname, const wchar_t* nick, int clubId)
{
    wcscpy(club.managerName, name);
    wcscpy(club.managerSurname, surname);
    wcscpy(club.nick, nick);
    club.isNick = wcslen(nick) ? 1 : 0;

    club.day = 1;
    club.month = 7;
    club.year = 2002;
    club.weekNumber = 1;
    club.roundNumber = 1;
    club.totalExpensesTransfers = 0;
    club.totalRevenuesTransfers = 0;
    club.season = 0;
    club.isMatchAutoMsg = 0;
    club.lastMatchRivalId = 0;

    memset(club.goalsLeague, 0, 480 * sizeof(int));
    memset(club.goalsControls, 0, 10 * sizeof(int));
    memset(club.managerStats, 0, 12 * sizeof(int));

    initNewClub(clubId);

    club.finances[12] = 500000.0;
}

/**
 * Gracz rozpoczyna pracę w nowym klubie, resetujemy część informacji
 */
void PlayerClub::initNewClub(int clubId)
{
    club.clubId = clubId;
    club.treBOPN = 0;
    club.isMatch = 0;
    club.teamSetting = 1;
    club.isRivalSet = 0;
    club.controlMatchesAmount = 1;
    club.isNotAllowedTeamPlayer = 0;
    club.isAssistantMsg = 0;
    club.playerGoals = 0;
    club.rivalGoals = 0;
    club.isRiot = 0;
    club.isBlockTransferFunds = 0;

    for(int i = 0; i < TRAINING_SLOTS_NUMBER; i++) {
        club.training[i] = defaultTrainingPlan[i];
    }

    club.trainedCondition = 0;
    club.trainedPasses = 0;
    club.trainedSetPieces = 0;
    club.trainedTactic = 0;
    club.trainedNotUse = 0;

    memset(club.finances, 0, 14 * sizeof(float));
    memset(club.financesLastMonth, 0, 14 * sizeof(float));

    club.instrPasses = INSTR_PASSES_MIXES;
    club.instrTreatment = INSTR_TREATMENT_NORMAL;
    club.instrPressing = INSTR_NO;
    club.instrOffsides = INSTR_NO;
    club.instrContra = INSTR_NO;
    club.instrAttitude = INSTR_ATTITUDE_NORMAL;
}

void PlayerClub::save() const
{
    FILE *f = fopen(FILE_SAVE_CLUB, "wb");
    fwrite(&club, sizeof(SClub), 1, f);
    fclose(f);
}

void PlayerClub::load()
{
    FILE *f = fopen(FILE_SAVE_CLUB, "rb");
    if (!f) {
        char message[512];
        snprintf(message, 512, ERROR_OPEN_FILE, FILE_SAVE_CLUB);
        throw std::invalid_argument(message);
    }

    (void) !fread(&club, sizeof(SClub), 1, f);
    fclose(f);
}

const wstring PlayerClub::getClubName(int index) const
{
    switch (index) {
        case  0: return L"Amica Wronki";
        case  1: return L"Garbarnia Szczak."; // Szczakowianka Jaworzno
        case  2: return L"GKS Katowice";
        case  3: return L"Górnik Zabrze";
        case  4: return L"Groclin Dyskobolia";
        case  5: return L"KSZO Ostrowiec";
        case  6: return L"Lech Poznań";
        case  7: return L"Legia Warszawa";
        case  8: return L"Odra Wodzisław";
        case  9: return L"Polonia Warszawa";
        case 10: return L"Pogoń Szczecin";
        case 11: return L"Ruch Chorzów";
        case 12: return L"Widzew Łódź";
        case 13: return L"Wisła Kraków";
        case 14: return L"Wisła Płock";
        case 15: return L"Zagłębie Lubin";
        default: return L"Error"; // should never happen
    }
}
