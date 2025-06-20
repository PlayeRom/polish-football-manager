
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
#include "Random.h"

#define MAX_MESSAGES 10

enum MatchStatus
{
    START_MATCH     = 0,
    END_1ST_HALF    = 1,
    START_2ND_HALF  = 2,
    END_MATCH       = 3,
    EXIT_MATCH      = 4
};

enum Statistics
{
    SHOOTS              = 0,
    SHOOTS_ON_TARGET    = 1,
    CORNERS             = 2,
    OFFSIDES            = 3,
    YELLOW_CARDS        = 4,
    RED_CARDS           = 5,
    PENALTIES           = 6,
    FOULS               = 7,
    MAX_STATISTICS
};

enum SpeedOptions
{
    VERY_FAST           = 0,
    FAST                = 1,
    MEDIUM              = 2,
    SLOW                = 3,
    VERY_SLOW           = 4,
    MAX_SPEED_OPTIONS
};

enum PitchZone
{
    ZONE_LEFT           = 0,
    ZONE_MIDDLE         = 1,
    ZONE_RIGHT          = 2,
    MAX_ZONES
};

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

struct SGoalsInfo
{
    wstring footballerName; // who scored
    int minute; // and in which minute
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
        Language *pLang,
        const Random *pRand
    );
    Match(const Match& orig);
    virtual ~Match();

    bool runMatch();

    int getDonA() const { return DonA; }
    int getMonM() const { return MonM; }
    int getAonD() const { return AonD; }

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
    const Random *pRand;
    vector<SGoalsInfo> goalsInfoPlayer;
    vector<SGoalsInfo> goalsInfoRival;

    int DonA; // differences {player Defence} minus {rival Attack}
    int MonM; // differences {player Midfield} minus {rival Midfield}
    int AonD; // differences {player Attack} minus {rival Defence}

    int howManyPlayerChanges; // how many footballer changes the player made
    bool isPlayerChanges; // gracz lub przeciwnik dokonał zmiany
    int whoPlayerChanges; // kto robi zmiane, 0 - nikt, 1 - gracz, 2 - rywal

    wstring msgFootballers[20]; // footballers names for formatting match messages
    wstring msgPlayerSurnames[16];
    wstring msgRivalSurnames[16];
    int msg[MAX_MESSAGES];
    bool msgWhoBall[MAX_MESSAGES]; // for messages, who has a ball for set bkg color, true - player, false - rival

    void updateTable();
    void prepareFootballers();
    void prepareFootballersSurnames(const SClub &clubRef);
    void drawTeam(int setting, int mode, int who);
    int whatHappened(
        bool isPlayerBall,
        int MonM,
        int DonA,
        int AonD,
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
        int foulsHome, int foulsAway,
        int penaltiesHome, int penaltiesAway,
        int yellowsHome, int yellowsAway,
        int redsHome, int redsAway,
        int ballPossHome, int ballPossAway,
        int DonA, int MonM, int AonD,
        int whereIsAction,
        bool isPlayerBall
    );
    void drawOnaA(bool isHome, int DonA);
    void drawAnaO(bool isHome, int AonD);
    void drawWhoScored(const vector<SGoalsInfo> &home, const vector<SGoalsInfo> &away);
    void saveMatchMessageToFile(int bkgColor, int matchMinute, const wchar_t *message);

    void playerTactics();
    void rivalTactics();

    wstring getFootballerSurname(bool isPlayerBall, int footballerId);
    int getArbiterDecision(int instrTreatment);
    int getMiddlefieldFootballerIdWhoFouled(int teamSetting);
    int getDefFootballerId(int teamSetting);
    int getFootballerIdWhoShootDistance(int teamSetting);
    int getGoooalMinute(int matchMinute, MatchStatus matchStatus);
    int getRightWingerFootballerId(int teamSetting);
    int getLeftWingerFootballerId(int teamSetting);

    int getFootballerStats(const SFootballer& footballer);
    int getFootballerStatsGoalkeeper(const SFootballer& footballer);
    int getFootballerStatsDefence(const SFootballer& footballer);
    int getFootballerStatsMiddlefield(const SFootballer& footballer);
    int getFootballerStatsAttack(const SFootballer& footballer);

    SFormationsSum getFormationsSum(const SClub &clubRef, bool isPlayerTeam);
    SFormationsSum getPlayerFormationsSum(const SClub &clubRef);
    SFormationsSum getRivalFormationsSum(const SClub &clubRef);

    int getEmptyMsgSlot();
};

#endif /* MATCH_H */

