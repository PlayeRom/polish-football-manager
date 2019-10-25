
#ifndef MANAGER_H
#define MANAGER_H

#include <string>
#include "Colors.h"
#include "Input.h"
#include "Footballers.h"
#include "Table.h"
#include "Rounds.h"
#include "PlayerClub.h"
#include "News.h"
#include "TeamInstructions.h"
#include "Match.h"
#include "Logger.h"

#define MAX_MESSAGES 10

using namespace std;

class Manager
{
public:
    Manager(
        PlayerClub *pClub,
        const Colors *pColors,
        const Input *pInput,
        Footballers *pFootballers,
        Table *pTable,
        Rounds *pRounds,
        News *pNews
    );
    virtual ~Manager();

    void runManager();

private:
    PlayerClub *pClub;
    const Colors *pColors;
    const Input *pInput;
    Footballers *pFootballers;
    Table *pTable;
    Rounds *pRounds;
    News *pNews;
    TeamInstructions *pTeamInstr;
    Match *pMatch;
    Logger *pLogger;

    // filtry listy transferowej:
    int filterPosition;
    int filterGoalkeeper;
    int filterDefense;
    int filterMiddlefield;
    int filterAttact;
    int filterForm;
    float filterMinPrice;
    float filterMaxPrice;

    /**
     *  wiadomosc
     * 0 - czy w ogole masz wiadomość (1 - tak, 0 - nie)
     * 1-9 - przechowuje ID wiadomosci
     * Maksymalnie mozesz otrzymac 9 wiadomosci jednoczesnie
     */
    int messages[MAX_MESSAGES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    wstring stringForMessage[4]; // string for message
    int numbersForMessage[MAX_MESSAGES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    void info();
    wchar_t displayMainMenu(const int message[5], const wstring *pTactics);
    void swapFootballers();
    wstring getSortTitle(int sort);
    wstring getTrenningDayName(int dayNumber);
    wstring getFilterByPosition(int pos);
    void setFilterColors(int bar, int barValue);
    void setAssistantMessageAfterMatch();
};

#endif /* MANAGER_H */
