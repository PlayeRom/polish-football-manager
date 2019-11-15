
#ifndef MATCH_H
#define MATCH_H

#include <string>
#include <vector>
#include "Input.h"
#include "Colors.h"
#include "Structs.h"
#include "Footballers.h"
#include "Table.h"
#include "Rounds.h"
#include "PlayerClub.h"
#include "TeamInstructions.h"
#include "Tactic.h"
#include "Logger.h"
#include "Language.h"

#define MAX_MESSAGES 10

struct SFormationsSum {
    SFormationsSum() { clean(); }
    void clean() {
        sumGol = 0;
        sumDef = 0;
        sumMid = 0;
        sumAtt = 0;
        goalBonus = 0.0;
    }
    int sumGol;
    int sumDef;
    int sumMid;
    int sumAtt;
    float goalBonus;
};

class Match {
public:
    Match(
        PlayerClub *pClub,
        const Colors *pColors,
        const Input *pInput,
        Footballers *pFootballers,
        Table *pTable,
        Rounds *pRounds,
        const TeamInstructions *pTeamInstruction,
        Language *pLang
    );
    Match(const Match& orig);
    virtual ~Match();

    bool runMecz();

    int getDonA() const { return OnaA; }
    int getMonM() const { return PnaP; }
    int getAonD() const { return AnaO; }

private:
    PlayerClub *pClub;
    const Colors *pColors;
    const Input *pInput;
    Footballers *pFootballers;
    Table *pTable;
    Rounds *pRounds;
    const TeamInstructions *pTeamInstruction;
    Language *pLang;
    const Tactic *pTactic;
    vector<SNews> matchMsgs;
    Logger *pLogger;

    // roznice {Obrona gracz} minus {Atak przeciwnika}, itd
    int OnaA;
    int PnaP;
    int AnaO;

    int howManyPlayerChanges; // how many footballer changes the player made
    bool isPlayerChanges; // gracz lub przeciwnik dokonał zmiany
    int whoPlayerChanges; // kto robi zmiane, 0 - nikt, 1 - gracz, 2 - rywal

    wchar_t tmpMessage[MAX_NEWS_LENGTH];
    wstring msgFootballers[20]; // footabllers names for match messages
    wstring msgPlayerSurnames[16];
    wstring msgRivalSurnames[16];
    int wiado[MAX_MESSAGES];
    int msgWhoBall[MAX_MESSAGES]; // dla wiadomosci, kto przy piłce 1 - gracz, 2 - rywal

    void updateTable();
    void prepareFootballers();
    void prepareFootballersSurnames(const SClub &clubRef);
    void drawTeam(int usta, int tryb, int kto);
    int whatHappened(
        bool isPlayerBall,
        int PnaP,
        int OnaA,
        int AnaO,
        int playerGoalkeeperSkills,
        int rivalGoalkeeperSkills,
        int whereIsAction,
        const SClub &clubRef
    );
    void loadMatchMessages();
    void drawBoard(
        bool isHome,
        int shootHome, int shootAway,
        int accurateShootHome, int accurateShootAway,
        int cornersHome, int cornersAway,
        float zoneLeft, float zoneCenter, float zoneRight,
        int offsidesHome, int offsidesAway,
        int faulsHome, int faulsAway,
        int paneltiesHome, int paneltiesAway,
        int yellowsHome, int yellowsAway,
        int redsHome, int redsAway,
        int ballPossHome, int ballPossAway,
        int OnaA, int PnaP, int AnaO,
        int whereIsAction,
        bool isPlayerBall
    );
    void drawOnaA(bool isHome, int OnaA);
    void drawAnaO(bool isHome, int AnaO);
    void drawWhoScored(int mingol1[], int mingol2[], wstring dlagol1[], wstring dlagol2[]);
    void saveMatchMessageToFile(int bkgcolor, int minuta, const wchar_t *message);

    void playerTactics();
    void rivalTactics();

    wstring getFootballerSurname(bool isPlayerBall, int footabllerId);
    int getArbiterDecision(int instrTreatment);
    int getMiddlefieldFootballerIdWhoFouled(int teamSetting);
    int getDefFootballerId(int teamSetting);
    int getFootballerIdWhoShootDistance(int teamSetting);
    int getGoooalMinute(int minuta, int koniec);
    int getRightWingerFootballerId(int teamSetting);
    int getLefttWingerFootballerId(int teamSetting);

    int getFooballerStats(const SFootballer& footballer);
    int getFooballerStatsGoalkeeper(const SFootballer& footballer);
    int getFooballerStatsDefence(const SFootballer& footballer);
    int getFooballerStatsMiddlefield(const SFootballer& footballer);
    int getFooballerStatsAttack(const SFootballer& footballer);

    SFormationsSum getFormationsSum(const SClub &clubRef, bool isPlayerTeam);
    SFormationsSum getPlayerFormationsSum(const SClub &clubRef);
    SFormationsSum getRivalFormationsSum(const SClub &clubRef);
};

#endif /* MATCH_H */

