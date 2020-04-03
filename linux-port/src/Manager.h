
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
#include "Squad.h"
#include "Tactic.h"
#include "Match.h"
#include "Logger.h"
#include "Language.h"
#include "Random.h"

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
        News *pNews,
        Language *pLang,
        const Random *pRand
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
    Tactic *pTactic;
    Squad *pSquad;
    Match *pMatch;
    Logger *pLogger;
    Language *pLang;
    const Random *pRand;

    // filtry listy transferowej:
    int filterPosition;
    int filterGoalkeeper;
    int filterDefense;
    int filterMidfield;
    int filterAttact;
    int filterForm;
    float filterMinPrice;
    float filterMaxPrice;

    void info();
    wchar_t displayMainMenu(const wstring *pTactics);
    void swapFootballers();

    void menuItemContinue();
    void menuItemContinueProcessing(SClub &clubRef);
    void menuItemContinueMatch(SClub &clubRef);
    void menuItemContinueUnemployed(const SClub &clubRef);

    void menuItemTactics();
    void menuItemTeamSquad();
    void menuItemTeamSquadFootballerDetails();

    void menuItemTraining();
    void menuItemTrainingWeekDay(wchar_t trainingMenu, int *ilex);
    void menuItemTrainingIndividual();

    void menuItemLastMatch();
    void menuItemRival();
    void menuItemCalendar();
    void menuItemCalendarControlMatch();
    void menuItemTable();
    void menuItemTransfersList();
    void menuItemTransfersListBuyFootballer(int mode);
    void menuItemTransfersListFilters();

    void printFinancesValues(int financesIndex, const wstring label);
    void menuItemFinance();
    void menuItemManagement();
    void menuItemManagerStats();
    void menuItemNewsOld();
    void menuItemNews();
    void menuItemOptions();

    wstring getSortTitle(int sort);
    wstring getTrainingDayName(int dayNumber);
    wstring getFilterByPosition(int pos);
    void setFilterColors(int bar, int barValue);
    void setAssistantMessageAfterMatch();

    void setRivalForPlayer();
    int getRivalSetting(int clubId);
    int getRivalOffsideTrap(int rivalSetting);
    int getRivalContra(int rivalSetting);
    int getRivalAttitude(int rivalSetting);

    bool isGrantedFunds(const SClub &clubRef);
    int getChanceForGrantedFunds(const SClub &clubRef);
};

#endif /* MANAGER_H */
