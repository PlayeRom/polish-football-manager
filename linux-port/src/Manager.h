
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

    void info();
    wchar_t displayMainMenu(const int message[5], const wstring *pTactics);
    void swapFootballers();

    void menuItemContinue();
    void menuItemContinueProcessing(SClub &clubRef);
    void menuItemContinueMatch(SClub &clubRef);
    void menuItemContinueUnemployed(const SClub &clubRef);

    wstring getSortTitle(int sort);
    wstring getTrenningDayName(int dayNumber);
    wstring getFilterByPosition(int pos);
    void setFilterColors(int bar, int barValue);
    void setAssistantMessageAfterMatch();

    void setRivalForPlayer();
    int getRivalSetting(int clubId);
    int getRivalOffsideTrap(int rivalSetting);
    int getRivalContra(int rivalSetting);
    int getRivalAttitude(int rivalSetting);
};

#endif /* MANAGER_H */
