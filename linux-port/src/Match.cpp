
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <ctype.h>
#include <cwchar>
#include <algorithm>
#include <iomanip>

#include "ConioColors.h"
#include "Match.h"
#include "BoxDrawingChars.h"
#include "Squad.h"

// Where is acrion
#define ACTION_IN_MIDDLEFIELD       1 // ball in the middle field
#define ACTION_IN_WINGER            2 // ball in winger
#define ACTION_IN_PANELTY_AREA      3 // ball in panelty area
#define ACTION_IN_GOAL_SITUATION    4 // shot on goal
#define ACTION_IN_PANELTY_OR_1ON1   5 // critical goal situation (panelty shot or attacker one on one with goalkeeper)
#define ACTION_IN_DIRECT_FREE_KICK  6 // goal situation by direct free kick

using namespace std;

Match::Match(
    PlayerClub *pClub,
    const Colors *pColors,
    const Input *pInput,
    Footballers *pFootballers,
    Table *pTable,
    Rounds *pRounds,
    const TeamInstructions *pTeamInstruction,
    Language *pLang,
    const Random *pRand
) {
    this->pClub = pClub;
    this->pColors = pColors;
    this->pInput = pInput;
    this->pFootballers = pFootballers;
    this->pTable = pTable;
    this->pRounds = pRounds;
    this->pTeamInstruction = pTeamInstruction;
    this->pLang = pLang;
    this->pRand = pRand;

    pTactic = new Tactic(pColors, pLang);
    pLogger = new Logger();
}

Match::Match(const Match& orig)
{
}

Match::~Match()
{
    delete pTactic;
    delete pLogger;
}

void Match::loadMatchMessages()
{
    matchMsgs.clear();

    char bufferFileName[32];
    snprintf(bufferFileName, 32, FILE_MSG_MATCH, pLang->getLngCode().c_str());

    FILE *f = fopen(bufferFileName, "rb");
    if (!f) {
        char message[512];
        snprintf(message, 512, ERROR_OPEN_FILE, FILE_MSG_MATCH);
        throw std::invalid_argument(message);
    }

    wchar_t buffer[MAX_NEWS_LENGTH];
    while (fgetws(buffer, MAX_NEWS_LENGTH, f) != NULL) {
        wstring line = buffer;
        if (line.size() < 3) {
            continue;
        }

        std::size_t colonPos = line.find(L":", 0);
        if (colonPos == wstring::npos) {
            continue;
        }

        wstring strNumber = line.substr(0, colonPos);
        if (!strNumber.empty()) {
            line[line.size() - 1] = L'\0'; // remove \n on end of line

            SNews news;
            news.num = std::stoi(strNumber);
            news.message = line.substr(colonPos + 1);
            matchMsgs.push_back(news);
        }
    }

    fclose(f);
}

bool Match::runMatch()
{
    SClub &clubRef = pClub->get();

    if (!clubRef.isMatch || clubRef.isNotAllowedTeamPlayer) {
        pInput->clrscr();
        pColors->textcolor(LIGHTRED);
        wcout << endl << pLang->get(L"Now you can't play a match!");
        return false;
    }

    loadMatchMessages();

    const wstring speed[MAX_SPEED_OPTIONS] = {
        pLang->get(L"V.fast"),
        pLang->get(L"Fast"),
        pLang->get(L"Medium"),
        pLang->get(L"Slow"),
        pLang->get(L"V.slow")
    };

    int rivalFootballerChanges[3] = {0};
    bool isOpenTacticsMenu = false;
    bool isPossibleGoToTactics = true;
    MatchStatus matchStatus = START_MATCH;
    bool isPlayerBall = false; // true - player team has a ball, false - rival team has a ball
    int whoStartedMatch = 0; // 1 - player, 2 - opponent
    int start = 0;
    SpeedOptions speedSelected = MEDIUM;
    int matchMinute = 0; // from 0 to 45 and from 45 to 90
    int trueMinute = 0; // true minute of match (it's always continuous counting)
    SFormationsSum playerFormationsSum;
    SFormationsSum rivalFormationsSum;

    // where is ball/action
    int whereIsAction = ACTION_IN_MIDDLEFIELD;

    int los = 0, x1 = 0, x2 = 0, x3 = 0;
    int playerBallPossessionCounter = 0;
    int ballPossessionPlayer = 0;
    int ballPossessionRival = 0;

    // remembers the number of the player who needs to be moved to the next message,
    // eg. "%s shoots!", footballerMemory = %s, "%s has scored!"
    int footballerMemory;

    int statPlayer[MAX_STATISTICS] = {0}; // match statistics for player
    int statRival[MAX_STATISTICS] = {0}; // match statistics for rival
    bool isShotFromDistance = false;
    bool endHalfMatch = false;

    /**
     * zone[0] - ball possession counter for rival
     * zone[1] - ball possession counter for middlefield
     * zone[2] - ball possession counter for player
     * zone[3] - ball possession % for rival
     * zone[4] - ball possession % for middlefield
     * zone[5] - ball possession % for player
     */
    float zonePossessionCounter[MAX_ZONES] = {0};
    float zonePossession[MAX_ZONES] = {0};

    howManyPlayerChanges = 0;
    isPlayerChanges = true;
    whoPlayerChanges = 0;
    memset(msg, 0, MAX_MESSAGES * sizeof(int));
    memset(msgWhoBall, 0, MAX_MESSAGES * sizeof(bool));
    goalsInfoPlayer.clear();
    goalsInfoRival.clear();

    time_t tmpTime = time(NULL);
    struct tm timeStruct = *localtime(&tmpTime);

    bool isWalkover = pFootballers->getSizePlayerTeam() < 11;
    clubRef.isWalkover = isWalkover ? 1 : 0;

    FILE *f = fopen(FILE_SAVE_LAST_MATCH_REPORTS, "w");
    fclose(f);

    prepareFootballers();

    wchar_t endMenuMatch = 'S';

    do {
        // przepisanie nazwisk zawodnikow to odpwiednich indeksow w tablicy, dla uzycia w wiadomosciach
        if (isPlayerChanges) {
            isPlayerChanges = false;

            prepareFootballersSurnames(clubRef);
        }

        const bool isHome = clubRef.rivalData[1] == 0;

        pInput->clrscr();
        pColors->textcolor(WHITE);
        if (isHome) { //w domu
            pColors->textbackground(BLUE);
            wprintf(L" %19ls %d ", pClub->getClubName(clubRef.clubId - 1).c_str(), goalsInfoPlayer.size());
            pColors->textbackground(RED);
            wprintf(L" %d %-19ls ", goalsInfoRival.size(), pClub->getClubName(clubRef.rivalData[0] - 1).c_str());
        }
        else { //wyjazd
            pColors->textbackground(RED);
            wprintf(L" %19ls %d ", pClub->getClubName(clubRef.rivalData[0] - 1).c_str(), goalsInfoRival.size());
            pColors->textbackground(BLUE);
            wprintf(L" %d %-19ls ", goalsInfoPlayer.size(), pClub->getClubName(clubRef.clubId - 1).c_str());
        }
        pColors->textbackground(BLACK);
        wprintf(pLang->get(L" Time:%3d").c_str(), matchMinute);
        pColors->textcolor(LIGHTGRAY);

        drawWhoScored(
            isHome ? goalsInfoPlayer : goalsInfoRival,
            isHome ? goalsInfoRival : goalsInfoPlayer
        );

        playerFormationsSum = getPlayerFormationsSum(clubRef);
        rivalFormationsSum = getRivalFormationsSum(clubRef);

        DonA = playerFormationsSum.sumDef - rivalFormationsSum.sumAtt;
        MonM = playerFormationsSum.sumMid - rivalFormationsSum.sumMid;
        AonD = playerFormationsSum.sumAtt - rivalFormationsSum.sumDef;
        if (isShotFromDistance) {
            // increase the chance of defending a shot from a distance
            playerFormationsSum.sumGol += 50;
            rivalFormationsSum.sumGol += 50;
            isShotFromDistance = false;
        }

        if (isHome) { // gracz gra w domu
            drawBoard(
                true,
                statPlayer[SHOOTS], statRival[SHOOTS], // shoots
                statPlayer[SHOOTS_ON_TARGET], statRival[SHOOTS_ON_TARGET], // shoots on target
                statPlayer[CORNERS], statRival[CORNERS], // corners
                zonePossession[ZONE_LEFT],
                zonePossession[ZONE_MIDDLE],
                zonePossession[ZONE_RIGHT],
                statPlayer[OFFSIDES], statRival[OFFSIDES], // offsides
                statPlayer[FOULS], statRival[FOULS], // fouls
                statPlayer[PENALTIES], statRival[PENALTIES], // penalties
                statPlayer[YELLOW_CARDS], statRival[YELLOW_CARDS], // yellow cards
                statPlayer[RED_CARDS], statRival[RED_CARDS], // red cards
                ballPossessionPlayer, ballPossessionRival,
                DonA, MonM, AonD,
                whereIsAction,
                isPlayerBall
            );
        }
        else { // gracz gra na wyjezdzie
            drawBoard(
                false,
                statRival[SHOOTS], statPlayer[SHOOTS], // shoots
                statRival[SHOOTS_ON_TARGET], statPlayer[SHOOTS_ON_TARGET], // shoots on target
                statRival[CORNERS], statPlayer[CORNERS], // corners
                zonePossession[ZONE_RIGHT],
                zonePossession[ZONE_MIDDLE],
                zonePossession[ZONE_LEFT],
                statRival[OFFSIDES], statPlayer[OFFSIDES], // offsides
                statRival[FOULS], statPlayer[FOULS], // fouls
                statRival[PENALTIES], statPlayer[PENALTIES], // panelties
                statRival[YELLOW_CARDS], statPlayer[YELLOW_CARDS], // yellow cards
                statRival[RED_CARDS], statPlayer[RED_CARDS], // red cards
                ballPossessionRival, ballPossessionPlayer,
                DonA, MonM, AonD,
                whereIsAction,
                isPlayerBall
            );
        }

        if (start == 1 && !isOpenTacticsMenu && matchStatus != EXIT_MATCH && matchStatus != END_1ST_HALF && clubRef.isMatchAutoMsg) {
            pColors->textcolor(GREEN);
            wcout << endl;
            wprintf(
                pLang->get(L"A Tactics-%ls  P Tactics-%ls  C Speed-%ls").c_str(),
                pClub->getClubName(clubRef.clubId - 1).c_str(),
                pClub->getClubName(clubRef.rivalData[0] - 1).c_str(),
                speed[speedSelected].c_str()
            );
            wcout << endl;
        }

        if (start == 1 && matchStatus < END_MATCH) { // the match has started
            int what = whatHappened(
                isPlayerBall,
                MonM,
                DonA,
                AonD,
                playerFormationsSum.sumGol,
                rivalFormationsSum.sumGol,
                whereIsAction,
                clubRef
            );
            //************ whereIsAction = ACTION_IN_MIDDLEFIELD *******************
            if (what == 9) {
                isShotFromDistance = true;
            }

            if (whereIsAction == ACTION_IN_MIDDLEFIELD) {
                los = pRand->get(2, 4);
                msgFootballers[0] = getFootballerSurname(isPlayerBall, los - 1);
                msgFootballers[1] = getFootballerSurname(isPlayerBall, 6); //czyli 7

                if (what == 1) {
                    msg[0] = 8;
                    msgWhoBall[0] = isPlayerBall;
                    msgWhoBall[1] = isPlayerBall;
                    int teamSetting = isPlayerBall ? clubRef.teamSetting : clubRef.rivalData[2];
                    msgFootballers[2] = getFootballerSurname(isPlayerBall, 6);

                    los = pRand->get(3);
                    if (los == 1) { //na lewe skrzydło
                        msg[1] = 9;
                        whereIsAction = ACTION_IN_WINGER;
                        footballerMemory = getLefttWingerFootballerId(teamSetting);
                        msgFootballers[3] = getFootballerSurname(isPlayerBall, footballerMemory);
                    }
                    else if (los == 2) { // podanie na prawe skrzydło
                        msg[1] = 10;
                        whereIsAction = ACTION_IN_WINGER;
                        footballerMemory = getRightWingerFootballerId(teamSetting);
                        msgFootballers[3] = getFootballerSurname(isPlayerBall, footballerMemory);
                    }
                    else if (los == 3) {
                        msg[1] = 11; // %ls podaje do przodu.
                        whereIsAction = ACTION_IN_PANELTY_AREA;
                        msgFootballers[2] = getFootballerSurname(isPlayerBall, 6);
                    }
                }
                else if (what == 2) { // successful counterattack
                    msg[0] = 12;
                    whereIsAction = ACTION_IN_PANELTY_AREA;
                    msgWhoBall[0] = isPlayerBall;
                }
                else if (what == 3 || what == 4) {
                    msg[0] = 12;
                    msgWhoBall[0] = isPlayerBall;
                    if (what == 4) {
                        msg[0] = 8;
                    }
                    msg[1] = 13;
                    if (what == 4) {
                        msg[1] = 14;
                    }

                    // zamiana pilki
                    isPlayerBall = !isPlayerBall;
                    msgWhoBall[1] = isPlayerBall;
                    msgFootballers[2] = getFootballerSurname(isPlayerBall, 6);
                    msgFootballers[3] = pClub->getClubName((isPlayerBall ? clubRef.clubId : clubRef.rivalData[0]) - 1);

                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }
                else if (what == 5) {
                    msg[0] = 8;
                    msgWhoBall[0] = isPlayerBall;
                    msg[1] = 15;
                    msgWhoBall[1] = isPlayerBall;
                    msg[2] = 16;

                    // zamiana pilki
                    isPlayerBall = !isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    msgFootballers[4] = getFootballerSurname(isPlayerBall, 6);

                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }
                else if (what == 6) {
                    msg[0] = 8;
                    msgWhoBall[0] = isPlayerBall;
                    msg[1] = 17;
                    msg[2] = 18;

                    // zamiana pilki
                    isPlayerBall = !isPlayerBall;
                    msgWhoBall[1] = isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    msgFootballers[2] = getFootballerSurname(isPlayerBall, 6);
                    msgFootballers[3] = msgFootballers[1];
                    msgFootballers[4] = getFootballerSurname(isPlayerBall, 6);

                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }
                else if (what == 7) { //faul przeciwnika
                    msg[0] = 8;
                    msgWhoBall[0] = isPlayerBall;
                    isPossibleGoToTactics = true;
                    msg[1] = 14;
                    msg[2] = 19;
                    int instrTreatment = isPlayerBall ? clubRef.rivalInst[1] : clubRef.inst[1];
                    int teamSetting = isPlayerBall ? clubRef.rivalData[2] : clubRef.teamSetting;
                    isPlayerBall ? statRival[FOULS]++ : statPlayer[FOULS]++;

                    what = getArbiterDecision(instrTreatment);

                    // losuje pomocnika, ktory otrzyma zolta kartke
                    los = getMiddlefieldFootballerIdWhoFouled(teamSetting);

                    // wez na odrot niz pilka wskazuje, ten kto nie ma pilki
                    msgFootballers[2] = getFootballerSurname(!isPlayerBall, los - 1);
                    msgWhoBall[1] = !isPlayerBall;
                    msgWhoBall[2] = !isPlayerBall;

                    if (what == 3) { //jest żółta
                        vector<SFootballer> &tmpFootballers = !isPlayerBall
                            ? pFootballers->getPlayersTeam()
                            : pFootballers->getRivals();
                        int clubId = !isPlayerBall ? clubRef.clubId : clubRef.rivalData[0];

                        bool is2ndYellow = false;
                        for (size_t index = 0; index < tmpFootballers.size(); index++) {
                            SFootballer &footballer = tmpFootballers[index];
                            if (footballer.data[0] == los && clubId == footballer.data[22]) {
                                footballer.data[12]++; // zolta kartka +1
                                footballer.data[7]--; // moral -1
                                if (footballer.data[7] < -3) {
                                    footballer.data[7] = -3;
                                }
                                is2ndYellow = footballer.data[12] >= 2;
                                break;
                            }
                        }

                        if (!isPlayerBall) {
                            pFootballers->savePlayerTeam();
                        }
                        else {
                            pFootballers->saveRivals();
                        }

                        if (is2ndYellow) {
                            msg[3] = 60; // %ls otrzymuje drugą żółtą kartkę i w konsekwencji czerwoną!
                            if (isPlayerBall) {
                                statRival[RED_CARDS]++;
                                statRival[YELLOW_CARDS]++;
                            }
                            else {
                                statPlayer[RED_CARDS]++;
                                statPlayer[YELLOW_CARDS]++;
                            }
                        }
                        else {
                            msg[3] = 20; // %ls otrzymuje żółtą kartkę!
                            if (isPlayerBall) {
                                statRival[YELLOW_CARDS]++;
                            }
                            else {
                                statPlayer[YELLOW_CARDS]++;
                            }
                        }

                        msgFootballers[6] = getFootballerSurname(!isPlayerBall, los - 1);
                        msgWhoBall[3] = !isPlayerBall;
                    }
                    else if (what == 2) { // upomnienie slowne
                        msg[3] = 21;
                        msgFootballers[6] = getFootballerSurname(!isPlayerBall, los - 1);
                        msgWhoBall[3] = !isPlayerBall;
                    }

                    msg[4] = 22; // %ls wznawia grę.
                    msgWhoBall[4] = isPlayerBall;
                    msgWhoBall[5] = isPlayerBall;

                    teamSetting = isPlayerBall ? clubRef.teamSetting : clubRef.rivalData[2];
                    msgFootballers[8] = getFootballerSurname(isPlayerBall, 6);
                    msgFootballers[10] = getFootballerSurname(isPlayerBall, 6);

                    los = pRand->get(3);
                    if (los == 1) { //na lewe skrzydło
                        msg[5] = 9; // %ls podaje na lewe skrzydło do %ls.
                        whereIsAction = ACTION_IN_WINGER;
                        footballerMemory = getLefttWingerFootballerId(teamSetting);
                        msgFootballers[11] = getFootballerSurname(isPlayerBall, footballerMemory);
                    }
                    else if (los == 2) { //podanie na prawe skrzydło
                        msg[5] = 10; // %ls podaje na prawe skrzydło do %ls.
                        whereIsAction = ACTION_IN_WINGER;
                        footballerMemory = getRightWingerFootballerId(teamSetting);
                        msgFootballers[11] = getFootballerSurname(isPlayerBall, footballerMemory);
                    }
                    else if (los == 3) {
                        msg[5] = 11; // %ls podaje do przodu.
                        whereIsAction = ACTION_IN_PANELTY_AREA;
                    }
                }
                else if (what == 8) { //aut dla przeciwnika
                    msg[0] = 8;
                    msgWhoBall[0] = isPlayerBall;
                    isPossibleGoToTactics = true;
                    msg[1] = 23;
                    msgWhoBall[1] = isPlayerBall;
                    msg[2] = pRand->get(24, 25);

                    los = pRand->get(2, 4);

                    // zmiana pilki
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[4] = getFootballerSurname(isPlayerBall, los - 1);
                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                    msgWhoBall[2] = isPlayerBall;
                }
                else if (what == 9) { //strzał z dystansu
                    msg[0] = 8;
                    msgWhoBall[0] = isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    msg[1] = 26;
                    msgWhoBall[1] = isPlayerBall;
                    msg[2] = pRand->get(27, 28);

                    isPlayerBall ? statPlayer[SHOOTS]++ : statRival[SHOOTS]++;

                    int teamSetting = isPlayerBall ? clubRef.teamSetting : clubRef.rivalData[2];
                    los = getFootballerIdWhoShootDistance(teamSetting);

                    msgFootballers[1] = getFootballerSurname(isPlayerBall, los - 1);
                    msgFootballers[2] = msgFootballers[1];
                    msgFootballers[4] = msgFootballers[2];
                    whereIsAction = ACTION_IN_GOAL_SITUATION;
                    footballerMemory = los - 1;
                }
            }
            else if (whereIsAction == ACTION_IN_WINGER) {
                //************** Skrzydłowy przy piłce ******************
                msg[0] = 29; // %ls biegnie wzdłuż bocznej linii boiska.
                msgWhoBall[0] = isPlayerBall;
                msgFootballers[0] = getFootballerSurname(isPlayerBall, footballerMemory);

                if (what == 10) { //udane dośrodkowanie
                    msg[1] = 30; //  %ls mija %ls i biegnie dalej...
                    msgWhoBall[1] = isPlayerBall;
                    msgFootballers[2] = msgFootballers[0];

                    vector<SFootballer> &tmpFootballers = isPlayerBall
                        ? pFootballers->getPlayersTeam()
                        : pFootballers->getRivals();
                    int clubId = isPlayerBall ? clubRef.clubId : clubRef.rivalData[0];
                    msgFootballers[3] = isPlayerBall ? msgRivalSurnames[3] : msgPlayerSurnames[3];

                    for (size_t index = 0; index < tmpFootballers.size(); index++) {
                        SFootballer &footballer = tmpFootballers[index];
                        if (footballer.data[0] == footballerMemory + 1 && footballer.data[22] == clubId) {
                            footballer.data[20]++; // form up
                            break;
                        }
                    }

                    if (isPlayerBall) {
                        pFootballers->savePlayerTeam();
                    }
                    else {
                        pFootballers->saveRivals();
                    }

                    msg[2] = pRand->get(31, 32);

                    msgWhoBall[2] = isPlayerBall;
                    whereIsAction = ACTION_IN_PANELTY_AREA;
                }
                else if (what == 11) { //faul
                    vector<SFootballer> &tmpFootballers = isPlayerBall
                        ? pFootballers->getPlayersTeam()
                        : pFootballers->getRivals();
                    int clubId = isPlayerBall ? clubRef.clubId : clubRef.rivalData[0];
                    msgFootballers[3] = isPlayerBall ? msgRivalSurnames[3] : msgPlayerSurnames[3];

                    for (size_t index = 0; index < tmpFootballers.size(); index++) {
                        SFootballer &footballer = tmpFootballers[index];
                        if (footballer.data[0] == footballerMemory + 1 && footballer.data[22] == clubId) {
                            footballer.data[20]++; // form up
                            break;
                        }
                    }

                    if (isPlayerBall) {
                        pFootballers->savePlayerTeam();
                    }
                    else {
                        pFootballers->saveRivals();
                    }

                    msg[1] = 30; //  %ls mija %ls i biegnie dalej...
                    msgWhoBall[1] = isPlayerBall;
                    msgFootballers[2] = msgFootballers[0];
                    isPossibleGoToTactics = true;

                    int teamSetting = isPlayerBall ? clubRef.rivalData[2] : clubRef.teamSetting;
                    isPlayerBall ? statRival[FOULS]++ : statPlayer[FOULS]++;

                    los = getDefFootballerId(teamSetting);

                    msgFootballers[3] = getFootballerSurname(!isPlayerBall, los - 1);

                    msg[2] = 33;
                    msgFootballers[4] = msgFootballers[3];
                    msgFootballers[5] = msgFootballers[2];
                    msgWhoBall[2] = !isPlayerBall;
                    msgWhoBall[3] = !isPlayerBall;
                    int instrTreatment = isPlayerBall ? clubRef.rivalInst[1] : clubRef.inst[1];
                    what = getArbiterDecision(instrTreatment);

                    if (what == 3) { //jest żółta
                        vector<SFootballer> &tmpFootballers = !isPlayerBall
                            ? pFootballers->getPlayersTeam()
                            : pFootballers->getRivals();
                        int clubId = !isPlayerBall ? clubRef.clubId : clubRef.rivalData[0];

                        bool is2ndYellow = false;
                        for (size_t index = 0; index < tmpFootballers.size(); index++) {
                            SFootballer &footballer = tmpFootballers[index];
                            if (footballer.data[0] == los && clubId == footballer.data[22]) {
                                footballer.data[12]++;
                                footballer.data[7]--;
                                if (footballer.data[7] < -3) {
                                    footballer.data[7] = -3;
                                }

                                is2ndYellow = footballer.data[12] >= 2;
                                break;
                            }
                        }

                        if (!isPlayerBall) {
                            pFootballers->savePlayerTeam();
                        }
                        else {
                            pFootballers->saveRivals();
                        }

                        msgFootballers[6] = msgFootballers[3];
                        if (is2ndYellow) {
                            msg[3] = 60; // %ls otrzymuje drugą żółtą kartkę i w konsekwencji czerwoną!
                            if (isPlayerBall) {
                                statRival[RED_CARDS]++;
                                statRival[YELLOW_CARDS]++;
                            }
                            else {
                                statPlayer[RED_CARDS]++;
                                statPlayer[YELLOW_CARDS]++;
                            }
                        }
                        else {
                            msg[3] = 20; // %ls otrzymuje żółtą kartkę!
                            if (isPlayerBall) {
                                statRival[YELLOW_CARDS]++;
                            }
                            else {
                                statPlayer[YELLOW_CARDS]++;
                            }
                        }
                    }
                    else if (what == 2) { //słownie
                        msg[3] = 21;
                        msgFootballers[6] = msgFootballers[3];
                        msgWhoBall[3] = !isPlayerBall;
                    }

                    msg[4] = 85; // %ls będzie uderzał z wolnego.
                    msgWhoBall[4] = isPlayerBall;

                    footballerMemory = pRand->get(2, 11) - 1;
                    msgFootballers[8] = getFootballerSurname(isPlayerBall, footballerMemory);
                    isPlayerBall ? statPlayer[SHOOTS]++ : statRival[SHOOTS]++;

                    msg[5] = 63; // Sędzia jeszcze ustawia mur.
                    msgWhoBall[5] = isPlayerBall;

                    msg[6] = 64; // %ls uderza z wolnego...
                    msgWhoBall[6] = isPlayerBall;
                    msgFootballers[12] = msgFootballers[8];
                    whereIsAction = ACTION_IN_DIRECT_FREE_KICK;
                }
                else if (what == 12) { // out
                    msg[1] = 34;
                    msgWhoBall[1] = isPlayerBall;
                    isPossibleGoToTactics = true;
                    msg[2] = pRand->get(24, 25);

                    los = pRand->get(2, 4);
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[4] = getFootballerSurname(isPlayerBall, los - 1);
                    msgWhoBall[2] = isPlayerBall;
                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }
                else if (what == 13) { //nie dośrodkował
                    msg[1] = 35;
                    los = pRand->get(2, 4);
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[2] = getFootballerSurname(isPlayerBall, los - 1);
                    msgWhoBall[1] = isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    int instrPasses = isPlayerBall ? clubRef.rivalInst[0] : clubRef.inst[0];

                    vector<SFootballer> &tmpFootballers = !isPlayerBall
                        ? pFootballers->getPlayersTeam()
                        : pFootballers->getRivals();
                    int clubId = !isPlayerBall ? clubRef.clubId : clubRef.rivalData[0];

                    for (size_t index = 0; index < tmpFootballers.size(); index++) {
                        SFootballer &footballer = tmpFootballers[index];
                        if (footballer.data[0] == footballerMemory + 1 && footballer.data[22] == clubId) {
                            footballer.data[20]--; // form down
                            break;
                        }
                    }

                    if (!isPlayerBall) {
                        pFootballers->savePlayerTeam();
                    }
                    else {
                        pFootballers->saveRivals();
                    }

                    msgFootballers[4] = msgFootballers[2];
                    los = pRand->get(6);
                    switch (instrPasses) {
                        case INSTR_PASSES_LONG: { //długie podania
                            if (los == 1) {
                                // %ls podaje do przodu.
                                msg[2] = 11; //whereIsAction = ACTION_IN_MIDDLEFIELD;
                            }
                            else {
                                // %ls wykonuje długie podanie do przodu.
                                msg[2] = 36; //whereIsAction = ACTION_IN_PANELTY_AREA;
                            }
                            break;
                        }
                        case INSTR_PASSES_MIXES:
                        case INSTR_PASSES_MIDDLE: { //mieszane, średnie
                            if (los == 2 || los == 3 || los == 6) {
                                msg[2] = 36; //whereIsAction = ACTION_IN_PANELTY_AREA;
                            }
                            else {
                                msg[2] = 11; //whereIsAction = ACTION_IN_MIDDLEFIELD;
                            }
                            break;
                        }
                        case INSTR_PASSES_SHORT: //któtkie podania
                        default: {
                            if (los == 1) {
                                msg[2] = 36; //whereIsAction = ACTION_IN_PANELTY_AREA
                            }
                            else {
                                msg[2] = 11; //whereIsAction = ACTION_IN_MIDDLEFIELD;
                            }
                            break;
                        }
                    }

                    whereIsAction = msg[2] == 11 ? ACTION_IN_MIDDLEFIELD : ACTION_IN_PANELTY_AREA;
                }
            }
            else if (whereIsAction == ACTION_IN_PANELTY_AREA) {
                //****************** penalty area ********
                if (what == 14) { // offside
                    msg[0] = 37;
                    msgWhoBall[0] = isPlayerBall;
                    isPossibleGoToTactics = true;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, 10);

                    // 38; // %ls na spalonym!
                    // 39; // %ls ruszył za wcześnie. Spalony.
                    // 40; // Chorągiewka w górze! %ls na spalonym.
                    msg[1] = pRand->get(38, 40);

                    msgFootballers[2] = msgFootballers[0];
                    msgWhoBall[1] = isPlayerBall;
                    msg[2] = 41;
                    isPlayerBall ? statPlayer[OFFSIDES]++ : statRival[OFFSIDES]++;
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[4] = getFootballerSurname(isPlayerBall, 0);
                    msgWhoBall[2] = isPlayerBall;
                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }
                else if (what == 15) { //spalony nieudany
                    msg[0] = 37;
                    msgWhoBall[0] = isPlayerBall;
                    msg[1] = 42;
                    msgWhoBall[1] = isPlayerBall;
                    msg[2] = 43;
                    msgWhoBall[2] = isPlayerBall;

                    int teamSetting = isPlayerBall ? clubRef.teamSetting : clubRef.rivalData[2];
                    switch (teamSetting) {
                        case T4_5_1: {
                            los = 11;
                            break;
                        }
                        case T3_4_3:
                        case T4_3_3: {
                            los = pRand->get(9, 11);
                            break;
                        }
                        default: {
                            los = pRand->get(10, 11);
                            break;
                        }
                    }

                    footballerMemory = los - 1;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, footballerMemory);
                    isPlayerBall ? statPlayer[SHOOTS]++ : statRival[SHOOTS]++;

                    msgFootballers[2] = msgFootballers[0];
                    msgFootballers[4] = msgFootballers[0];
                    whereIsAction = ACTION_IN_PANELTY_OR_1ON1;
                }
                else if (what == 16) { // defence kick off
                    int instrPasses = isPlayerBall ? clubRef.rivalInst[0] : clubRef.inst[0];

                    los = pRand->get(6);
                    switch (instrPasses) {
                        case INSTR_PASSES_LONG: { //długie
                            if (los == 1) {
                                whereIsAction = ACTION_IN_MIDDLEFIELD;
                                msg[0] = pRand->get(44, 45);
                            }
                            else {
                                whereIsAction = ACTION_IN_PANELTY_AREA;
                                msg[0] = pRand->get(46, 47);
                            }
                            break;
                        }
                        case INSTR_PASSES_SHORT: { //krótkie
                            if (los == 1) {
                                whereIsAction = ACTION_IN_PANELTY_AREA;
                                msg[0] = pRand->get(46, 47);
                            }
                            else {
                                whereIsAction = ACTION_IN_MIDDLEFIELD;
                                msg[0] = pRand->get(44, 45);
                            }
                            break;
                        }
                        default: { //pozostałe
                            if (los == 1 || los == 2 || los == 3) {
                                whereIsAction = ACTION_IN_PANELTY_AREA;
                                msg[0] = pRand->get(46, 47);
                            }
                            else {
                                whereIsAction = ACTION_IN_MIDDLEFIELD;
                                msg[0] = pRand->get(44, 45);
                            }
                            break;
                        }
                    }
                    los = pRand->get(2, 4);
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, los - 1);
                    msgWhoBall[0] = isPlayerBall;
                }
                else if (what == 17) { // Gollkeeper catch
                    msg[0] = 48;
                    msg[1] = 49;
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, 0);
                    msgWhoBall[0] = isPlayerBall;
                    msgWhoBall[1] = isPlayerBall;
                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }
                else if (what == 18) { // penalty kick
                    msg[0] = 37;
                    msgWhoBall[0] = isPlayerBall;
                    msg[1] = 50;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, 10);
                    msgWhoBall[1] = !isPlayerBall;
                    int teamSetting = isPlayerBall ? clubRef.rivalData[2] : clubRef.teamSetting;
                    if (isPlayerBall) {
                        statRival[FOULS]++;
                        statPlayer[PENALTIES]++;
                    }
                    else {
                        statPlayer[FOULS]++;
                        statRival[PENALTIES]++;
                    }

                    los = getDefFootballerId(teamSetting);

                    msgFootballers[3] = msgFootballers[0];
                    msgFootballers[2] = getFootballerSurname(!isPlayerBall, los - 1);

                    msg[2] = pRand->get(51, 52);

                    int instrTreatment = isPlayerBall ? clubRef.rivalInst[1] : clubRef.inst[1];
                    msgWhoBall[2] = !isPlayerBall;
                    msgWhoBall[3] = !isPlayerBall;
                    isPlayerBall ? statPlayer[SHOOTS]++ : statRival[SHOOTS]++;

                    what = getArbiterDecision(instrTreatment);

                    if (what == 3) { // yellow card
                        vector<SFootballer> &tmpFootballers = !isPlayerBall
                            ? pFootballers->getPlayersTeam()
                            : pFootballers->getRivals();
                        int clubId = !isPlayerBall ? clubRef.clubId : clubRef.rivalData[0];

                        bool is2ndYellow = false;
                        for (size_t index = 0; index < tmpFootballers.size(); index++) {
                            SFootballer &footballer = tmpFootballers[index];
                            if (footballer.data[0] == los && clubId == footballer.data[22]) {
                                footballer.data[12]++;
                                footballer.data[7]--;
                                if (footballer.data[7] < -3) {
                                    footballer.data[7] = -3;
                                }

                                is2ndYellow = footballer.data[12] >= 2;
                                break;
                            }
                        }

                        if (!isPlayerBall) {
                            pFootballers->savePlayerTeam();
                        }
                        else {
                            pFootballers->saveRivals();
                        }

                        msgFootballers[6] = msgFootballers[2];
                        if (is2ndYellow) {
                            msg[3] = 60; // %ls otrzymuje drugą żółtą kartkę i w konsekwencji czerwoną!
                            if (isPlayerBall) {
                                statRival[RED_CARDS]++;
                                statRival[YELLOW_CARDS]++;
                            }
                            else {
                                statPlayer[RED_CARDS]++;
                                statPlayer[YELLOW_CARDS]++;
                            }
                        }
                        else {
                            msg[3] = 20; // %ls otrzymuje żółtą kartkę!
                            isPlayerBall ? statRival[YELLOW_CARDS]++ : statPlayer[YELLOW_CARDS]++;
                        }
                    }
                    else if (what == 2) { // verbal
                        msg[3] = 21;
                        msgFootballers[6] = msgFootballers[2];
                        msgWhoBall[3] = !isPlayerBall;
                    }

                    msg[4] = pRand->get(53, 54);
                    footballerMemory = pRand->get(8, 11) - 1;
                    msgFootballers[8] = getFootballerSurname(isPlayerBall, footballerMemory);
                    whereIsAction = ACTION_IN_PANELTY_OR_1ON1;
                    msgWhoBall[4] = isPlayerBall;
                }
                else if (what == 19) { // sim foul
                    msg[0] = 37; // %ls przyjmuje piłkę w polu karnym...
                    msgWhoBall[0] = isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, 10);
                    msgWhoBall[1] = !isPlayerBall;
                    msgWhoBall[3] = !isPlayerBall;
                    int teamSetting = isPlayerBall ? clubRef.rivalData[2] : clubRef.teamSetting;

                    los = getDefFootballerId(teamSetting);

                    msgFootballers[3] = msgFootballers[0];
                    msgFootballers[2] = getFootballerSurname(!isPlayerBall, los - 1);
                    msg[1] = 50; // %ls wślizgiem atakuje %ls...
                    msg[2] = 55; // %ls pada na murawę, ale sędzia nie dał się nabrać.
                    msgWhoBall[2] = isPlayerBall;
                    msgFootballers[4] = msgFootballers[0];
                    msg[3] = pRand->get(44, 45);
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[6] = msgFootballers[2];
                    msgWhoBall[3] = isPlayerBall;
                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }
                else if (what == 20) { // corner
                    msg[0] = 37;
                    msgWhoBall[0] = isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    msgWhoBall[3] = isPlayerBall;
                    isPossibleGoToTactics = true;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, 10);
                    msgWhoBall[1] = !isPlayerBall;
                    int teamSetting = isPlayerBall ? clubRef.rivalData[2] : clubRef.teamSetting;
                    isPlayerBall ? statPlayer[CORNERS]++ : statRival[CORNERS]++;

                    los = getDefFootballerId(teamSetting);

                    msgFootballers[3] = msgFootballers[0];
                    msgFootballers[2] = getFootballerSurname(!isPlayerBall, los - 1);
                    msg[1] = 50;
                    msg[2] = 56;
                    msg[3] = 78;
                    msgFootballers[6] = getFootballerSurname(isPlayerBall, 6);
                    whereIsAction = ACTION_IN_PANELTY_AREA;
                }
                else if (what == 21) { // shoot
                    msg[0] = 37;
                    msgWhoBall[0] = isPlayerBall;
                    msgWhoBall[1] = isPlayerBall;
                    msg[1] = (pRand->get(2) == 1) ? 43 : 57;
                    int teamSetting = isPlayerBall ? clubRef.teamSetting : clubRef.rivalData[2];
                    if (teamSetting == T3_4_3 || teamSetting == T4_3_3) {
                        los = pRand->get(9, 11);
                    }
                    else if (teamSetting == T4_5_1) {
                        los = 11;
                    }
                    else {
                        los = pRand->get(10, 11);
                    }

                    msgFootballers[0] = getFootballerSurname(isPlayerBall, los - 1);
                    isPlayerBall ? statPlayer[SHOOTS]++ : statRival[SHOOTS]++;
                    msgFootballers[2] = msgFootballers[0];
                    footballerMemory = los - 1;
                    whereIsAction = ACTION_IN_GOAL_SITUATION;
                }
                else if (what == 22) { // pass and shoot
                    msg[0] = 37;
                    msgWhoBall[0] = isPlayerBall;
                    msg[1] = 58;
                    msgWhoBall[1] = isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    msg[2] = pRand->get(2) == 1 ? 59 : 57;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, 10);
                    los = pRand->get(7, 10);
                    msgFootballers[2] = getFootballerSurname(isPlayerBall, los - 1);
                    isPlayerBall ? statPlayer[SHOOTS]++ : statRival[SHOOTS]++;
                    msgFootballers[4] = msgFootballers[2];
                    footballerMemory = los - 1;
                    whereIsAction = ACTION_IN_GOAL_SITUATION;
                }
                else if (what == 23) { // attacker foul
                    msg[0] = 37;
                    msgWhoBall[0] = isPlayerBall;
                    isPossibleGoToTactics = true;
                    msgFootballers[2] = getFootballerSurname(!isPlayerBall, 1);
                    msgWhoBall[1] = !isPlayerBall;
                    msgWhoBall[2] = !isPlayerBall;
                    int instrTreatment = isPlayerBall ? clubRef.inst[1] : clubRef.rivalInst[1];
                    int teamSetting = isPlayerBall ? clubRef.teamSetting : clubRef.rivalData[2];

                    isPlayerBall ? statPlayer[FOULS]++ : statRival[FOULS]++;

                    if (teamSetting == T3_4_3 || teamSetting == T4_3_3) {
                        los = pRand->get(9, 11);
                    }
                    else if (teamSetting == T4_5_1) {
                        los = 11;
                    }
                    else {
                        los = pRand->get(10, 11);
                    }

                    msgFootballers[0] = getFootballerSurname(isPlayerBall, los - 1);
                    msgFootballers[3] = msgFootballers[0];
                    msg[1] = 50;
                    msg[2] = 61;
                    msg[3] = 62;
                    msgWhoBall[3] = isPlayerBall;
                    msgFootballers[6] = msgFootballers[0];
                    msg[4] = 19;
                    msgWhoBall[4] = isPlayerBall;

                    what = getArbiterDecision(instrTreatment);

                    if (what == 3) { // yellow card
                        vector<SFootballer> &tmpFootballers = isPlayerBall
                            ? pFootballers->getPlayersTeam()
                            : pFootballers->getRivals();
                        int clubId = isPlayerBall ? clubRef.clubId : clubRef.rivalData[0];

                        bool is2ndYellow = false;
                        for (size_t index = 0; index < tmpFootballers.size(); index++) {
                            SFootballer &footballer = tmpFootballers[index];
                            if (footballer.data[0] == los && clubId == footballer.data[22]) {
                                footballer.data[12]++;
                                footballer.data[7]--;
                                if (footballer.data[7] < -3) {
                                    footballer.data[7] = -3;
                                }

                                is2ndYellow = footballer.data[12] >= 2;
                                break;
                            }
                        }

                        if (isPlayerBall) {
                            pFootballers->savePlayerTeam();
                        }
                        else {
                            pFootballers->saveRivals();
                        }

                        msgFootballers[10] = msgFootballers[0];
                        msgWhoBall[5] = isPlayerBall;

                        if (is2ndYellow) {
                            msg[5] = 60;
                            if (isPlayerBall) {
                                statPlayer[YELLOW_CARDS]++;
                                statPlayer[RED_CARDS]++;
                            }
                            else {
                                statRival[YELLOW_CARDS]++;
                                statRival[RED_CARDS]++;
                            }
                        }
                        else {
                            msg[5] = 20;
                            if (isPlayerBall) {
                                statPlayer[YELLOW_CARDS]++;
                            }
                            else {
                                statRival[YELLOW_CARDS]++;
                            }
                        }
                    }
                    else if (what == 2) { // verbal
                        msg[5] = 21;
                        msgFootballers[10] = msgFootballers[3];
                        msgWhoBall[5] = isPlayerBall;
                    }
                    msg[6] = 41;
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[12] = getFootballerSurname(isPlayerBall, 0);
                    msgWhoBall[6] = isPlayerBall;
                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }
            }
            //*********************** obron B ***********************************
            else if (whereIsAction == ACTION_IN_GOAL_SITUATION ||
                     whereIsAction == ACTION_IN_PANELTY_OR_1ON1 ||
                     whereIsAction == ACTION_IN_DIRECT_FREE_KICK
            ) { //obrona B
                if (what == 24) { // udana
                    msg[0] = pRand->get(65, 66);
                    msg[1] = 67; // I łapie piłkę! Dobra obrona.
                    msg[2] = 49; // Bramkarz wykopuje piłkę.
                    isPlayerBall ? statPlayer[SHOOTS_ON_TARGET]++ : statRival[SHOOTS_ON_TARGET]++;
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, 0);
                    msgWhoBall[0] = isPlayerBall;
                    msgWhoBall[1] = isPlayerBall;

                    msgFootballers[2] = msgFootballers[0];
                    msgWhoBall[2] = isPlayerBall;
                    whereIsAction = ACTION_IN_MIDDLEFIELD;

                    vector<SFootballer> &tmpFootballers = isPlayerBall
                        ? pFootballers->getPlayersTeam()
                        : pFootballers->getRivals();
                    int clubId = isPlayerBall ? clubRef.clubId : clubRef.rivalData[0];

                    for (size_t index = 0; index < tmpFootballers.size(); index++) {
                        SFootballer &footballer = tmpFootballers[index];
                        if (footballer.data[0] == 1 && clubId == footballer.data[22]) {
                            footballer.data[20]++; // form up
                            footballer.data[7]++; // morale up
                            if (footballer.data[7] > 3) {
                                footballer.data[7] = 3;
                            }
                            break;
                        }
                    }

                    if (isPlayerBall) {
                        pFootballers->savePlayerTeam();
                    }
                    else {
                        pFootballers->saveRivals();
                    }
                }
                else if (what == 25) { //no problem
                    //68. Dobrze ustawiony %ls, bez trudu łapie piłkę.
                    //69. %ls spokojnie łapie piłkę.
                    msg[0] = pRand->get(68, 69);
                    msg[1] = 49; // Bramkarz wykopuje piłkę.

                    isPlayerBall ? statPlayer[SHOOTS_ON_TARGET]++ : statRival[SHOOTS_ON_TARGET]++;
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, 0);
                    msgWhoBall[0] = isPlayerBall;
                    msgWhoBall[1] = isPlayerBall;
                    whereIsAction = ACTION_IN_MIDDLEFIELD;

                    vector<SFootballer> &tmpFootballers = isPlayerBall
                        ? pFootballers->getPlayersTeam()
                        : pFootballers->getRivals();
                    int clubId = isPlayerBall ? clubRef.clubId : clubRef.rivalData[0];

                    for (size_t index = 0; index < tmpFootballers.size(); index++) {
                        SFootballer &footballer = tmpFootballers[index];
                        if (footballer.data[0] == 1 && clubId == footballer.data[22]) {
                            footballer.data[20]++; // form up
                            footballer.data[7]++; // morale up
                            if (footballer.data[7] > 3) {
                                footballer.data[7] = 3;
                            }
                            break;
                        }
                    }

                    if (isPlayerBall) {
                        pFootballers->savePlayerTeam();
                    }
                    else {
                        pFootballers->saveRivals();
                    }
                }
                else if (what == 26) { //niepewnie
                    los = pRand->get(4);
                    if (los == 1) {
                        msg[0] = 70; // %ls z trudem broni, ale nie zdołał złapać piłki...
                        msgWhoBall[0] = !isPlayerBall;
                        if (isPlayerBall) {
                            statPlayer[SHOOTS_ON_TARGET]++;
                        }
                        else {
                            statRival[SHOOTS_ON_TARGET]++;
                        }
                    }
                    else if (los == 2) {
                        msg[0] = 71; // Piłka uderza w słupek!!
                        msgWhoBall[0] = isPlayerBall;
                    }
                    else if (los == 3) {
                        msg[0] = 72; // Piłka uderza w poprzeczkę!!
                        msgWhoBall[0] = isPlayerBall;
                    }
                    else {
                        msg[0] = 73; // Piłka uderza w któregoś z obrońców!
                        msgWhoBall[0] = !isPlayerBall;
                    }
                    msgFootballers[0] = getFootballerSurname(!isPlayerBall, 0);
                    whereIsAction = ACTION_IN_PANELTY_AREA;
                }
                else if (what == 27) { // corner
                    isPossibleGoToTactics = true;
                    if (pRand->get(2) == 1) {
                        msg[0] = 70; // %ls hardly defends but fails to catch the ball ...
                        isPlayerBall ? statPlayer[SHOOTS_ON_TARGET]++ : statRival[SHOOTS_ON_TARGET]++;
                    }
                    else {
                        msg[0] = 73; // The ball hits one of the defenders!
                    }

                    msgWhoBall[0] = !isPlayerBall;
                    msgFootballers[0] = getFootballerSurname(!isPlayerBall, 0);
                    msgFootballers[4] = getFootballerSurname(isPlayerBall, 6);
                    isPlayerBall ? statPlayer[CORNERS]++ : statRival[CORNERS]++;
                    msg[1] = 56;
                    msgWhoBall[1] = isPlayerBall;
                    msg[2] = 78;
                    msgWhoBall[2] = isPlayerBall;
                    whereIsAction = ACTION_IN_PANELTY_AREA;
                }
                else if (what == 28) { // GOOL
                    isPossibleGoToTactics = true;
                    msg[0] = pRand->get(65, 66);
                    msg[1] = 74;
                    msgWhoBall[1] = isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    msg[2] = pRand->get(75, 76);
                    msg[3] = 22;
                    int footballerIdForm = pRand->get(11);

                    vector<SFootballer> &tmpFootballers = isPlayerBall
                        ? pFootballers->getPlayersTeam()
                        : pFootballers->getRivals();
                    int clubId = isPlayerBall ? clubRef.clubId : clubRef.rivalData[0];

                    for (size_t index = 0; index < tmpFootballers.size(); index++) {
                        SFootballer &footballer = tmpFootballers[index];
                        if (footballer.data[0] == footballerMemory + 1 && clubId == footballer.data[22]) {
                            footballer.data[21]++; // gole w meczu++
                            footballer.data[20]++; // forma podczas meczu
                            footballer.data[8] += 3; // morale
                            if (footballer.data[8] >= 5) {
                                footballer.data[8] = 0;
                                footballer.data[7]++;
                            }

                            if (footballer.data[7] > 3) {
                                footballer.data[7] = 3;
                            }

                            if (isPlayerBall) {
                                clubRef.finances[7] += footballer.finances[2]; // premia za gola
                            }
                        }
                        if (footballer.data[0] == footballerIdForm && clubId == footballer.data[22]) {
                            footballer.data[20]++; // losowy gracz dosatje formę
                        }
                    }

                    if (isPlayerBall) {
                        pFootballers->savePlayerTeam();
                    }
                    else {
                        pFootballers->saveRivals();
                    }

                    SGoalsInfo goalInfo;
                    goalInfo.minute = getGoooalMinute(matchMinute, matchStatus);

                    if (isPlayerBall) {
                        statPlayer[SHOOTS_ON_TARGET]++;
                        goalInfo.footballerName = msgPlayerSurnames[footballerMemory];
                        goalsInfoPlayer.push_back(goalInfo);
                    }
                    else {
                        statRival[SHOOTS_ON_TARGET]++;
                        goalInfo.footballerName = msgRivalSurnames[footballerMemory];
                        goalsInfoRival.push_back(goalInfo);
                    }

                    msgFootballers[0] = getFootballerSurname(!isPlayerBall, 0);
                    msgFootballers[2] = pClub->getClubName((isPlayerBall ? clubRef.clubId : clubRef.rivalData[0]) - 1);
                    msgFootballers[4] = getFootballerSurname(isPlayerBall, footballerMemory);
                    msgFootballers[6] = pClub->getClubName((isPlayerBall ? clubRef.rivalData[0] : clubRef.clubId) - 1);
                    isPlayerBall = !isPlayerBall;
                    msgWhoBall[3] = isPlayerBall;
                    msgWhoBall[0] = isPlayerBall;

                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                    int footballerIdFormMinus = pRand->get(11);

                    // musi byc raz jeszcze pobranie zespolu i klubu bo zmienila sie pilka
                    vector<SFootballer> &losersFootballers = isPlayerBall
                        ? pFootballers->getPlayersTeam()
                        : pFootballers->getRivals();
                    clubId = isPlayerBall ? clubRef.clubId : clubRef.rivalData[0];

                    for (size_t index = 0; index < losersFootballers.size(); index++) {
                        SFootballer &footballer = losersFootballers[index];

                        if (footballer.data[0] == 1 && clubId == footballer.data[22]) {
                            footballer.data[20]--; // bramkarz traci formę i morale
                            footballer.data[8] -= 3; // morale
                            if (footballer.data[8] < -5) {
                                footballer.data[8] = 0;
                                footballer.data[7]--;
                            }

                            if (footballer.data[7] < PLAYERS_MORALE_FATAL) {
                                footballer.data[7] = PLAYERS_MORALE_FATAL;
                            }
                        }

                        if (footballer.data[0] == footballerIdFormMinus && clubId == footballer.data[22]) {
                            footballer.data[20]--; // losowy zawodnik dostaje tez minus forma
                        }
                    }

                    if (isPlayerBall) {
                        pFootballers->savePlayerTeam();
                    }
                    else {
                        pFootballers->saveRivals();
                    }
                }
                else if (what == 29) { // goal not recognized
                    isPossibleGoToTactics = true;
                    msg[0] = pRand->get(65, 66);
                    msg[1] = 74;
                    msgWhoBall[1] = isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    msg[2] = pRand->get(75, 76);
                    msg[3] = 77;
                    msgWhoBall[3] = isPlayerBall;
                    msg[4] = 41;

                    isPlayerBall ? statPlayer[SHOOTS]-- : statRival[SHOOTS]--;
                    msgFootballers[0] = getFootballerSurname(!isPlayerBall, 0);
                    msgFootballers[2] = pClub->getClubName((isPlayerBall ? clubRef.clubId : clubRef.rivalData[0]) - 1);
                    msgFootballers[4] = getFootballerSurname(isPlayerBall, footballerMemory);
                    msgFootballers[8] = msgFootballers[0];
                    isPlayerBall = !isPlayerBall;
                    msgWhoBall[4] = isPlayerBall;
                    msgWhoBall[0] = isPlayerBall;

                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }
                else if (what == 30) { // shot off target
                    isPossibleGoToTactics = true;
                    // 79; // %ls walnął nad poprzeczką!
                    // 80; // Ale strzelił fatalnie!
                    // 81; // %ls minimalnie chybił!
                    // 82; // Poszło w trybuny! Fatalny strzał.
                    msg[0] = pRand->get(79, 82);

                    msgWhoBall[0] = isPlayerBall;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, footballerMemory);
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[2] = getFootballerSurname(!isPlayerBall, 0);
                    msg[1] = 41; // %ls resumes the game.
                    msgWhoBall[1] = isPlayerBall;
                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }
            }

            pColors->textcolor(LIGHTGRAY);
            if (matchMinute == 0 && matchStatus == START_MATCH) { // The beginning of the match.
                memset(msg, 0, 10 * sizeof(int));
                msg[0] = 1; // %ls starts the match.
                whoStartedMatch = pRand->get(2);
                if (whoStartedMatch == 1) { // player starts the match
                    msgFootballers[0] = pClub->getClubName(clubRef.clubId - 1);
                    isPlayerBall = true;
                }
                else { // opponent start the match
                    msgFootballers[0] = pClub->getClubName(clubRef.rivalData[0] - 1);
                    isPlayerBall = false;
                }
                whereIsAction = ACTION_IN_MIDDLEFIELD;
                msgWhoBall[0] = isPlayerBall;
            }
            else if (matchMinute == 45 && matchStatus == START_2ND_HALF) { // The beginning of the 2nd half
                if (whoStartedMatch == 1) { // started played so 2nd half starts rival
                    msgFootballers[0] = pClub->getClubName(clubRef.rivalData[0] - 1);
                    isPlayerBall = false;
                }
                else { // started rival so 2nd half starts player
                    msgFootballers[0] = pClub->getClubName(clubRef.clubId - 1);
                    isPlayerBall = true;
                }
                whereIsAction = ACTION_IN_MIDDLEFIELD;
                msgWhoBall[0] = isPlayerBall;
                memset(msg, 0, 10 * sizeof(int));
                msg[0] = 2; // %ls starts the second half.
            }
            else if (matchStatus == END_1ST_HALF) {
                isPossibleGoToTactics = true;
                matchMinute = 43;
                matchStatus = START_2ND_HALF; // poczatek 2 polowy
                start = 0;
                whereIsAction = ACTION_IN_MIDDLEFIELD;
                memset(msg, 0, 10 * sizeof(int));
                msg[0] = 3; // End of first half.
            }

            //************************ injury *************************
            if (pRand->get(100) == 1) { // lottery for injury 1/100
                bool isPlayerInjury = pRand->get(2) == 1; // lottery which team 50/50
                los = pRand->get(11); // lottery which player

                vector<SFootballer> &tmpFootballers = isPlayerInjury
                    ? pFootballers->getPlayersTeam()
                    : pFootballers->getRivals();
                int clubId = isPlayerInjury ? clubRef.clubId : clubRef.rivalData[0];

                for (size_t index = 0; index < tmpFootballers.size(); index++) {
                    SFootballer &footballer = tmpFootballers[index];
                    if (footballer.data[0] == los && footballer.data[22] == clubId && footballer.data[12] < 2) {
                        footballer.data[11] = 0;
                        footballer.data[19] = 1;

                        int k = getEmptyMsgSlot();
                        msg[k] = pRand->get(83, 84); // %ls doznaje kontuzji! // %ls kontuzjowany!

                        if (isPlayerInjury) {
                            msgWhoBall[k] = true;
                            msgFootballers[k * 2] = msgPlayerSurnames[los - 1];
                        }
                        else {
                            msgWhoBall[k] = false;
                            msgFootballers[k * 2] = msgRivalSurnames[los - 1];
                        }

                        break;
                    }
                }

                if (isPlayerInjury) {
                    pFootballers->savePlayerTeam();
                }
                else {
                    pFootballers->saveRivals();
                }
            }
            //************************ injury end *************************

            //******************** rival - exchange injured footballer ***********
            if (isPossibleGoToTactics) {
                x1 = 0;
                x3 = 0;
                for (size_t index = 0; index < pFootballers->getSizeRivals(); index++) {
                    SFootballer &footballer = pFootballers->getRival(index);
                    if (footballer.data[11] == 0 &&
                        clubRef.rivalData[0] == footballer.data[22] &&
                        footballer.data[0] < 12
                    ) {
                        x1 = footballer.data[0]; // footballer number
                        x2 = footballer.data[2]; // the formation he's playing on
                    }
                }

                for (int i = 0; i < 3; i++) {
                    if (rivalFootballerChanges[i] == 0) {
                        rivalFootballerChanges[i] = x1;
                        break;
                    }
                }

                for (int i = 0; i < 3; i++) {
                    if (rivalFootballerChanges[i] == x1) {
                        x3++;
                    }
                }

                if (x3 > 1) {
                    for (int i = 2; i >= 0; i--) {
                        if (rivalFootballerChanges[i] == x1) {
                            rivalFootballerChanges[i] = 0;
                            break;
                        }
                    }
                    x1 = 0;
                }

                if (x1 > 0) {
                    for (size_t index = 0; index < pFootballers->getSizeRivals(); index++) {
                        const SFootballer &footballer = pFootballers->getRival(index);
                        if (footballer.data[0] > 11 &&
                            footballer.data[0] < 17 &&
                            clubRef.rivalData[0] == footballer.data[22] &&
                            x2 == footballer.data[2]
                        ) {
                            x3 = footballer.data[0];
                        }
                    }

                    for (size_t index = 0; index < pFootballers->getSizeRivals(); index++) {
                        SFootballer &footballer = pFootballers->getRival(index);
                        if (clubRef.rivalData[0] == footballer.data[22]) {
                            if (footballer.data[0] == x1) {
                                footballer.data[0] = x3;
                            }
                            else if (footballer.data[0] == x3) {
                                footballer.data[0] = x1;
                            }
                        }
                    }
                    pFootballers->saveRivals();
                    whoPlayerChanges = 2;
                    isPlayerChanges = true;
                }
            }
            //************************************************
            if (whoPlayerChanges > 0) { // exchange the footballer
                const int k = getEmptyMsgSlot();
                msg[k] = 5; // %ls makes a change...
                if (whoPlayerChanges == 1) { // player exchange
                    msgWhoBall[k] = true;
                    msgFootballers[k * 2] = pClub->getClubName(clubRef.clubId - 1);
                }
                else { // rival exchange
                    msgWhoBall[k] = false;
                    msgFootballers[k * 2] = pClub->getClubName(clubRef.rivalData[0] - 1);
                    msg[k + 1] = 7;
                    msgWhoBall[k + 1] = false;
                    msgFootballers[(k + 1) * 2] = msgRivalSurnames[x3 - 1];
                    msgFootballers[(k + 1) * 2 + 1] = msgRivalSurnames[x1 - 1];
                }
                whoPlayerChanges = 0;
            }
            //*************** zadyma **********************
            if (pRand->get(100) == 1 && whereIsAction == ACTION_IN_MIDDLEFIELD) {
                const int k = getEmptyMsgSlot();
                msg[k] = 86;
                msg[k + 1] = 87;
                msg[k + 2] = 88;
                msg[k + 3] = 89;
                if (clubRef.rivalData[1] == 0) { // player play in home
                    msgWhoBall[k] = true;
                    msgWhoBall[k + 1] = true;
                    msgWhoBall[k + 2] = true;
                    msgWhoBall[k + 3] = true;
                    clubRef.isRiot = 1;

                    pClub->save();
                }
                else {
                    msgWhoBall[k] = false;
                    msgWhoBall[k + 1] = false;
                    msgWhoBall[k + 2] = false;
                    msgWhoBall[k + 3] = false;
                }
            }
            //*********************************
            wchar_t tmpMessage[MAX_NEWS_LENGTH];
            for (int i = 0; i < MAX_MESSAGES; i++) {
                const int k = i * 2;
                if (msg[i] != 0) {
                    wcout << endl;
                }

                for (int index = 0; index < matchMsgs.size(); index++) {
                    const SNews &matchMasgItem = matchMsgs[index];
                    if (msg[i] == matchMasgItem.num) {
                        int color = 0;
                        const bool isGoal = msg[i] == 74; // GOOOL dla %ls!!!
                        if (isGoal) {
                            color = 143;
                        }
                        else if (msg[i] == 20) { // %ls otrzymuje żółtą kartkę!
                            color = YELLOW;
                        }
                        else if (msg[i] == 60) { //  %ls otrzymuje drugą żółtą kartkę i w konsekwencji czerwoną!
                            color = LIGHTRED;
                        }
                        else if (msg[i] == 83 || msg[i] == 84 ||
                                 msg[i] == 86 || msg[i] == 87 || msg[i] == 88 || msg[i] == 89
                        ) {
                            // %ls doznaje kontuzji! / %ls kontuzjowany!
                            // Na chwilę przenosimy się na trybuny...
                            // Tam trwają starcia pseudo kibiców z policją!
                            // No cóż, przykry to widok...
                            // Ale wracamy do meczu.
                            color = LIGHTGREEN;
                        }
                        else {
                            color = WHITE;
                        }

                        int bkgcolor = msgWhoBall[i] == true ? BLUE : RED;

                        pColors->textcolor(color);
                        pColors->textbackground(bkgcolor);

                        swprintf(
                            tmpMessage,
                            MAX_NEWS_LENGTH,
                            matchMasgItem.message.c_str(),
                            msgFootballers[k].c_str(),
                            msgFootballers[k + 1].c_str()
                        );

                        if (isGoal) {
                            pColors->startBlinking();
                        }

                        wcout << tmpMessage;

                        if (isGoal) {
                            pColors->stopBlinking();
                        }

                        saveMatchMessageToFile(bkgcolor, matchMinute, tmpMessage);

                        break;
                    }
                }

                if (msg[i] != 0) { //&&msg[i+1]!=0)
                    if (clubRef.isMatchAutoMsg) {
                        time_t tmpTime = time(NULL);
                        timeStruct = *localtime(&tmpTime);
                        int k = timeStruct.tm_sec;

                        if (speedSelected == 1) {
                            if (k == 59) k = 0;
                            else k += speedSelected;
                        }
                        else if (speedSelected == 2) {
                            if (k == 58) k = 0;
                            else if (k == 59) k = 1;
                            else k += speedSelected;
                        }
                        else if (speedSelected == 3) {
                            if (k == 57) k = 0;
                            else if (k == 58) k = 1;
                            else if (k == 59) k = 2;
                            else k += speedSelected;
                        }
                        else if (speedSelected == 4) {
                            if (k == 56) k = 0;
                            else if (k == 57) k = 1;
                            else if (k == 58) k = 2;
                            else if (k == 59) k = 3;
                            else k += speedSelected;
                        }

                        while (timeStruct.tm_sec != k) {
                            time_t tmpTime = time(NULL);
                            timeStruct = *localtime(&tmpTime);

                            // UWAGA kbhit sprawdza czy nacisnieto klawisz, ale nie wstrzymuje programu
                            if (pInput->kbhit() && isPossibleGoToTactics) {
                                isOpenTacticsMenu = true; // wcisnieto klawisz, i mozna wejsc w taktykę, wtedy isOpenTacticsMenu = 1
                            }
                            fflush(stdout);
                        }
                    }
                    else {
                        // The user must press the key after each message
                        wchar_t key = pInput->getKeyBoardPressed();
                        if (clubRef.isMatchAutoMsg) {
                            if (isPossibleGoToTactics && (key == 'A' || key == 'P')) {
                                isOpenTacticsMenu = true; // user chce wejsc w taktyke swoja albo rywala
                            }
                        }
                    }
                }
            }
            pColors->textbackground(BLACK);
            memset(msg, 0, MAX_MESSAGES * sizeof(int));
            matchMinute += 2;
            trueMinute += 2;
            //****************** ball possessions and zones **********************
            if (isPlayerBall) {
                playerBallPossessionCounter++;
            }
            ballPossessionPlayer = (playerBallPossessionCounter * 100) / (trueMinute / 2); // ball possession by player in %
            ballPossessionRival = 100 - ballPossessionPlayer; // ball possession by rival in %

            if (whereIsAction == ACTION_IN_MIDDLEFIELD) {
                zonePossessionCounter[ZONE_MIDDLE]++;
            }
            else {
                zonePossessionCounter[isPlayerBall ? ZONE_RIGHT : ZONE_LEFT]++;
            }
            zonePossession[ZONE_LEFT] = (zonePossessionCounter[ZONE_LEFT] * 100) / (trueMinute / 2);
            zonePossession[ZONE_MIDDLE] = (zonePossessionCounter[ZONE_MIDDLE] * 100) / (trueMinute / 2);
            zonePossession[ZONE_RIGHT] = (zonePossessionCounter[ZONE_RIGHT] * 100) / (trueMinute / 2);
            //***************** ball possessions and zones *********************

            //***************** kondycja ***************************
            int k = (clubRef.inst[2] == 1) ? 2 : 3;

            for (size_t index = 0; index < pFootballers->getSizePlayerTeam(); index++) {
                SFootballer &footballer = pFootballers->getPlayerTeam(index);
                los = pRand->get(k);
                if (los > 1) {
                    los = 0;
                }

                if (clubRef.clubId == footballer.data[22] && footballer.data[0] < 12) {
                    footballer.data[11] -= los;
                }

                if (footballer.data[11] < 0) {
                    footballer.data[11] = 0;
                }
            }
            pFootballers->savePlayerTeam();

            k = (clubRef.rivalInst[2] == 1) ? 2 : 3;

            for (size_t index = 0; index < pFootballers->getSizeRivals(); index++) {
                SFootballer &footballer = pFootballers->getRival(index);
                los = pRand->get(k);
                if (los > 1) {
                    los = 0;
                }

                if (clubRef.rivalData[0] == footballer.data[22] && footballer.data[0] < 12) {
                    footballer.data[11] -= los;
                }

                if (footballer.data[11] < 0) {
                    footballer.data[11] = 0;
                }
            }
            pFootballers->saveRivals();
        }//dla start=1
        //************ wypisz wiadomość *************

        if (matchMinute > 45 && whereIsAction == ACTION_IN_MIDDLEFIELD && matchStatus == START_MATCH && !endHalfMatch) {
            matchStatus = END_1ST_HALF;
            matchMinute = 45;
            endHalfMatch = true;
            whereIsAction = ACTION_IN_MIDDLEFIELD;
        }

        if (matchMinute > 90 && whereIsAction == ACTION_IN_MIDDLEFIELD && matchStatus == START_2ND_HALF) {
            matchStatus = END_MATCH;
        }

        if (matchStatus == EXIT_MATCH) {
            wcout << endl << pLang->get(L"END OF THE MATCH");
        }
        pColors->textcolor(GREEN);
        wcout << endl;
        if (start == 0) {
            wcout << endl << pLang->get(L"S Start a match") << endl;
        }

        if (clubRef.isMatchAutoMsg && matchStatus != END_MATCH) {
            wcout << endl;
            wprintf(
                pLang->get(L"A Tactics-%ls  P Tactics-%ls  C Speed-%ls").c_str(),
                pClub->getClubName(clubRef.clubId - 1).c_str(),
                pClub->getClubName(clubRef.rivalData[0] - 1).c_str(),
                speed[speedSelected].c_str()
            );
            wcout << endl;
        }

        if (!clubRef.isMatchAutoMsg) {
            if (start && matchStatus < EXIT_MATCH) {
                wcout << pLang->get(L"S Continue     ");
            }
            wprintf(
                pLang->get(L"A Tactics - %ls    P Tactics - %ls").c_str(),
                pClub->getClubName(clubRef.clubId - 1).c_str(),
                pClub->getClubName(clubRef.rivalData[0] - 1).c_str()
            );
            wcout << endl;
        }

        if (matchStatus == EXIT_MATCH) {
            pColors->textcolor(RED);
            wcout << endl << pLang->get(L"Q Quit");
        }

        wchar_t menu = L'y';
        if (clubRef.isMatchAutoMsg && (start == 0 || isOpenTacticsMenu || matchStatus == EXIT_MATCH)) {
            menu = pInput->getKeyBoardPressed();
        }
        else if (!clubRef.isMatchAutoMsg &&
                (start == 0 || isPossibleGoToTactics || matchStatus == END_1ST_HALF || matchStatus == EXIT_MATCH)
        ) {
            menu = pInput->getKeyBoardPressed();
        }

        if (matchStatus == END_MATCH) {
            matchStatus = EXIT_MATCH;
        }

        isPossibleGoToTactics = false;
        isOpenTacticsMenu = false;

        if (isWalkover) {
            endMenuMatch = L'Q';
            pColors->textcolor(LIGHTRED);
            wcout << endl << pLang->get(L"You lose by default 0-3");
            pInput->getKeyBoardPressed();

            goalsInfoPlayer.clear();

            SGoalsInfo goalsInfo;
            goalsInfoRival.push_back(goalsInfo);
            goalsInfoRival.push_back(goalsInfo);
            goalsInfoRival.push_back(goalsInfo);
        }

        switch (menu) {
            case 'C': { // speed
                pInput->clrscr();
                pColors->textcolor(GREEN);
                wcout << endl <<
                    pLang->get(L"Enter the subtitle display speed:") << endl <<
                    pLang->get(L"0. Very fast") << endl <<
                    pLang->get(L"1. Fast") << endl <<
                    pLang->get(L"2. Medium") << endl <<
                    pLang->get(L"3. Slow") << endl <<
                    pLang->get(L"4. Very Slow") << endl;
                speedSelected = static_cast<SpeedOptions>(pInput->getNumber());
                if (speedSelected < VERY_FAST || speedSelected > VERY_SLOW) {
                    speedSelected = MEDIUM;
                }
                break;
            }
            case 'Q': { // quit
                if (matchStatus == EXIT_MATCH) {
                    endMenuMatch = L'Q';
                }
                else {
                    endMenuMatch = L'S'; // Q dziala jak start, to nie jest doby pomysl
                }
                // break; // no break
            }
            case 'S': {
                if (start == 0) {
                    start = 1;
                }
                break;
            }
            case 'A': { // taktyka gracza
                if (matchStatus < END_MATCH) {
                    start = 0;
                }

                playerTactics();
                break;
            }
            case 'P': { // taktyka przeciwnika
                if (matchStatus < END_MATCH) {
                    start = 0;
                }

                rivalTactics();
                break;
            }
        }
    }
    while (endMenuMatch != 'Q');

    pColors->textcolor(LIGHTGRAY);
    wcout << endl << pLang->get(L"Please wait...");
    if (clubRef.rivalData[1] == 0) { // player home
        float tickets = pRand->get(30, 59) * 5000.0;
        clubRef.finances[0] += tickets; //bilety

        float tv = pRand->get(30, 59) * 5000.0;
        clubRef.finances[1] += tv; //Tv

        clubRef.finances[2] += pRand->get(10) * 1000; //handel

        clubRef.finances[4] += 300000;
    }
    else {
        clubRef.finances[4] += 50000;
    }

    clubRef.finances[5] = clubRef.finances[0] + clubRef.finances[1] + clubRef.finances[2] + clubRef.finances[3] + clubRef.finances[4];
    clubRef.finances[10] = clubRef.finances[6] + clubRef.finances[7] + clubRef.finances[8] + clubRef.finances[9];
    clubRef.finances[11] = clubRef.finances[5] - clubRef.finances[10];
    clubRef.isMatch = 0; //bylM=1;
    clubRef.playerGoals = static_cast<int>(goalsInfoPlayer.size());
    clubRef.rivalGoals = static_cast<int>(goalsInfoRival.size());
    clubRef.isAssistantMsg = 1;
    //******** manager stats ************
    clubRef.managerStats[4]++;
    if (goalsInfoPlayer.size() > goalsInfoRival.size()) {
        clubRef.managerStats[5]++;
    }
    else if (goalsInfoPlayer.size() == goalsInfoRival.size()) {
        clubRef.managerStats[6]++;
    }
    else {
        clubRef.managerStats[7]++;
    }
    clubRef.managerStats[8] += static_cast<int>(goalsInfoPlayer.size());
    clubRef.managerStats[9] += static_cast<int>(goalsInfoRival.size());
    clubRef.managerStats[3] = (clubRef.managerStats[0] * 100)
        + (clubRef.managerStats[2] * 10)
        + (clubRef.managerStats[4] * 10)
        + (clubRef.managerStats[5] * 3)
        + clubRef.managerStats[6]
        - clubRef.managerStats[7]
        + clubRef.managerStats[8]
        - clubRef.managerStats[9];
    //*******************************

    if (clubRef.roundNumber <= 0) { //mecze kontrolne
        int k = clubRef.roundNumber + 4;
        clubRef.goalsControls[k * 2] = static_cast<int>(goalsInfoPlayer.size());
        clubRef.goalsControls[k * 2 + 1] = static_cast<int>(goalsInfoRival.size());
    }
    else if (clubRef.roundNumber > 0) { // kolejka ligowa
        int k = -2;
        while (k != 14) { //wszystkie mecze kolejki, losujemy wyniki meczy
            k += 2;
            int round = clubRef.roundNumber * 16;
            int homeGoals = pRand->get(0, 4);
            int awayGoals = pRand->get(0, 3);
            clubRef.goalsLeague[round - 16 + k] = homeGoals;
            clubRef.goalsLeague[round - 16 + k + 1] = awayGoals;
        }

        int clubIndexInRound = pRounds->getClubIndexInRoundByClubId(clubRef.roundNumber, clubRef.clubId);

        int roundIndex = clubRef.roundNumber * 16;
        if (clubIndexInRound % 2 == 0) {
            clubRef.goalsLeague[roundIndex - 16 + clubIndexInRound] = static_cast<int>(goalsInfoPlayer.size());
            clubRef.goalsLeague[roundIndex - 16 + clubIndexInRound + 1] = static_cast<int>(goalsInfoRival.size());
        }
        else {
            clubRef.goalsLeague[roundIndex - 16 + clubIndexInRound] = static_cast<int>(goalsInfoPlayer.size());
            clubRef.goalsLeague[roundIndex - 16 + clubIndexInRound - 1] = static_cast<int>(goalsInfoRival.size());
        }

        for (size_t index = 0; pRounds->getSize(); index++) {
            SRound &round = pRounds->get(index);

            if (clubRef.roundNumber == round.number) {
                pTable->updateAfterMatch(round, roundIndex, clubRef);
                break;
            }
        }

        pTable->save();
    }
    pClub->save();
    //**************

    for (size_t index = 0; index < pFootballers->getSizePlayerTeam(); index++) {
        SFootballer &footballer = pFootballers->getPlayerTeam(index);
        if (footballer.data[13] >= 2) {
            footballer.data[13] = 0; // reset yellow cards
        }

        if (footballer.data[14] == 1) {
            footballer.data[14] = 0; // reset red cards
        }

        if (footballer.data[21] > 0) { // goals in match
            footballer.data[16] += footballer.data[21]; // sum it
            footballer.data[21] = 0; // and reset for match
        }

        if (footballer.data[12] >= 2) { // 2 yellow cart in match
            footballer.data[14] = 1; // the red card
            footballer.data[12] = 0; // reset yellow in match
            footballer.data[13] = 0; // reset yellow general
        }

        if (footballer.data[12] > 0) { // is yellow cart in match
            footballer.data[13] += footballer.data[12]; // sum it
            footballer.data[12] = 0; // reset yellow card in match
        }

        footballer.data[9] += footballer.data[20]; // form in match change general form

        if (footballer.data[9] > 10) { // form more then limit
            footballer.data[9] = 10;
        }

        if (footballer.data[9] < 1) { // form less then minimum
            footballer.data[9] = 1;
        }
    }
    pFootballers->savePlayerTeam();

    updateTable();

    pColors->textcolor(LIGHTGREEN);
    return true;
}

void Match::updateTable()
{
    int pkt[16], clubId[16], golr[16], gol[16], blokada[16];
    int loop = 0;
    int tmpReplace = 0;

    while (loop != 3) {
        loop++;

        for (size_t i = 0; i < pTable->getSize(); i++) {
            STable &tableRef = pTable->get(i);
            pkt[i]     = tableRef.points; //zapisuje punkty do pkt
            clubId[i]  = tableRef.clubId; // numer klubu
            golr[i]    = tableRef.goalsDiff; // roznica goli
            gol[i]     = tableRef.goalsScored; //gole zdobyte
            blokada[i] = 0;
        }

        for (int i = 0; i < 16; i++) { //sortowanie wg. pkt
            blokada[i] = 0;
            for (int k = i + 1; k < 16; k++) {
                if (pkt[i] < pkt[k]) {
                    tmpReplace = clubId[i]; // numery klubów, zamieniam wg. kolejności
                    clubId[i] = clubId[k];
                    clubId[k] = tmpReplace;
                    //**********
                    tmpReplace = pkt[i]; //punkty klubów
                    pkt[i] = pkt[k];
                    pkt[k] = tmpReplace;
                }
            }
        }

        for (int i = 0; i < 16; i++) { //sortowanie wg. +/-
            for (int k = i + 1; k < 16; k++) {
                if (golr[i] < golr[k] && pkt[i] == pkt[k]) {
                    tmpReplace = clubId[i]; // numery klubów, zamieniam wg. kolejności
                    clubId[i] = clubId[k];
                    clubId[k] = tmpReplace;
                    //**********
                    tmpReplace = pkt[i]; //punkty klubów
                    pkt[i] = pkt[k];
                    pkt[k] = tmpReplace;
                    //**********
                    tmpReplace = golr[i]; //różnica goli
                    golr[i] = golr[k];
                    golr[k] = tmpReplace;
                }
            }
        }

        for (int i = 0; i < 16; i++) {//sortowanie wg. goli zdobytych
            for (int k = i + 1; k < 16; k++) {
                if (gol[i] < gol[k] && golr[i] == golr[k] && pkt[i] == pkt[k]) {
                    tmpReplace = clubId[i]; // numery klubów, zamieniam wg. kolejności
                    clubId[i] = clubId[k];
                    clubId[k] = tmpReplace;
                    //**********
                    tmpReplace = pkt[i]; //punkty klubów
                    pkt[i] = pkt[k];
                    pkt[k] = tmpReplace;
                    //**********
                    tmpReplace = golr[i]; //różnica goli
                    golr[i] = golr[k];
                    golr[k] = tmpReplace;
                    //***********
                    tmpReplace = gol[i]; //gole zdobyte
                    gol[i] = gol[k];
                    gol[k] = tmpReplace;
                }
            }
        }

        vector<STable> newTable;
        for (int i = 0; i < 16; i++) {
            for (int k = 0; k < 16; k++) {
                for (int v = 0; v < 16; v++) {
                    for (size_t index = 0; index < pTable->getSize(); index++) {
                        STable &tableRef = pTable->get(index);
                        if (clubId[i] == tableRef.clubId &&
                            pkt[i] == tableRef.points &&
                            golr[k] == tableRef.goalsDiff &&
                            gol[v] == tableRef.goalsScored &&
                            blokada[i] == 0
                        ) {
                            tableRef.tablePosition = i + 1;
                            blokada[i] = tableRef.clubId;
                            newTable.push_back(tableRef);
                        }
                    }
                }
            }
        }
        pTable->overrideTable(newTable);
        pTable->save();
    }
}

void Match::prepareFootballers()
{
    for (size_t index = 0; index < pFootballers->getSizePlayerTeam(); index++) {
        SFootballer &footballer = pFootballers->getPlayerTeam(index);

        if (footballer.data[0] <= 11) { // increase morale of footballers from first squad
            footballer.data[8] += 3;
            if (footballer.data[8] > 5) {
                footballer.data[7]++;
                footballer.data[8] = 0;
                if (footballer.data[7] > 3) {
                    footballer.data[7] = 3;
                }
            }
        }

        if (footballer.data[0] > 16) { // decrease morale of footballers who have not been called to the match
            footballer.data[8] -= 2;
            if (footballer.data[8] < -5) {
                footballer.data[7]--;
                footballer.data[8] = 0;
                if (footballer.data[7] < -3) {
                    footballer.data[7] = -3;
                }
            }
        }

        footballer.data[20] = 0; // reset form during the match
        footballer.data[21] = 0; // reset goals during the match
    }
    pFootballers->savePlayerTeam();

    for (size_t index = 0; index < pFootballers->getSizeRivals(); index++) {
        SFootballer &footballer = pFootballers->getRival(index);

        footballer.data[20] = 0; // reset form during the match
        footballer.data[21] = 0; // reset goals during the match
        footballer.data[12] = 0; // yellow cards during the match
    }
    pFootballers->saveRivals();
}

void Match::prepareFootballersSurnames(const SClub &clubRef)
{
    for (int counter = 1; counter < 16; ++counter) { // <- 16 max zawodnikow razem z rezerwowymi
        for (size_t index = 0; index < pFootballers->getSizePlayerTeam(); index++) {
            const SFootballer &footballer = pFootballers->getPlayerTeam(index);
            if (footballer.data[0] == counter) {
                msgPlayerSurnames[counter - 1] = footballer.surname;
                break;
            }
        }

        for (size_t index = 0; index < pFootballers->getSizeRivals(); index++) {
            const SFootballer &footballer = pFootballers->getRival(index);
            if (footballer.data[0] == counter && footballer.data[22] == clubRef.rivalData[0]) {
                msgRivalSurnames[counter - 1] = footballer.surname;
                break;
            }
        }
    }
}

/**
 * @param int setting
 * @param int mode It can be 11 (1st squad) or 16 (reserve)
 * @param int who = 0 - player club id, who > 0 - rival club id
 */
void Match::drawTeam(int setting, int mode, int who)
{
    Squad squad(pColors, pLang);
    int i = 0;

    pColors->textcolor(GREEN);
    wcout << endl << pLang->get(L"No    Footballer       Po.  ");

    pColors->textcolor(LIGHTBLUE);
    wcout << pLang->get(L"G") << L"  ";

    pColors->textcolor(MAGENTA);
    wcout << pLang->get(L"D") << L"  ";

    pColors->textcolor(LIGHTCYAN);
    wcout << pLang->get(L"M") << L"  ";
    pColors->textcolor(LIGHTGREEN);

    wcout << pLang->get(L"A") << L"  ";
    pColors->textcolor(GREEN);

    wcout << pLang->get(L"Morale  For. ForM Con. Goals");
    pColors->textcolor(LIGHTBLUE);
    int color = LIGHTBLUE;
    if (mode == 16) {
        i = 11; // dla pokazania rezerwowych w taktyce
    }
    else if (mode == 40) {
        i = 20; // dla dalej w Składzie
    }

    while (i != mode) {
        i++;
        if (mode == 16 || i == 12) {
            color = YELLOW;
            pColors->textcolor(color);
        }
        else if (mode == 40) {
            color = LIGHTGRAY;
            pColors->textcolor(color);
        }

        vector<SFootballer> &tmpFoorballers = (who > 0) ? pFootballers->getRivals() : pFootballers->getPlayersTeam();
        int clubId = (who > 0) ? who : pClub->get().clubId;

        for (int index = 0; index < tmpFoorballers.size(); index++) {
            SFootballer &footballer = tmpFoorballers[index];

            if (i == footballer.data[0] && clubId == footballer.data[22]) {
                wcout << endl;
                wprintf(L"%2d.", footballer.data[0]);

                if (footballer.data[15] > 0 || footballer.data[19]) {
                    pColors->textbackground(RED);
                    pColors->textcolor(LIGHTGRAY);
                    wcout << pLang->get(L"In");
                    pColors->textbackground(BLACK);
                    pColors->textcolor(color);
                }
                else if (footballer.data[12] == 1) {
                    pColors->textcolor(YELLOW);
                    wcout << BOX_FULL_BLOCK << L" ";
                    pColors->textcolor(color);
                }
                else if (footballer.data[12] >= 2) {
                    pColors->textcolor(RED);
                    wcout << BOX_FULL_BLOCK << L" ";
                    pColors->textcolor(color);
                }
                else {
                    wcout << L"  ";
                }

                wprintf(
                    L"%3ls%-15ls %ls  %2d %2d %2d %2d  %-7ls  %2d",
                    footballer.name,
                    footballer.surname,
                    squad.getFootballerPosition(footballer.data[2]).c_str(),
                    footballer.data[3],
                    footballer.data[4],
                    footballer.data[5],
                    footballer.data[6],
                    squad.getMorale(footballer.data[7]).c_str(),
                    footballer.data[9]
                );

                if (footballer.data[20] > 0){
                    wprintf(L"   +%d  %3d%%  %2d", footballer.data[20], footballer.data[11], footballer.data[21]);
                }
                else {
                    wprintf(L"   %2d  %3d%%  %2d", footballer.data[20], footballer.data[11], footballer.data[21]);
                }
            }
        }

        if (i == 1) {
            color = MAGENTA;
            pColors->textcolor(color);
        }

        if ((i == 5) &&
            (setting == 1 || setting == 2 || setting == 3 || setting == 4 || setting == 9 || setting == 10 || setting == 11)
        ) {
            color = LIGHTCYAN;
            pColors->textcolor(color);
        }
        else if ((i == 4) && (setting == 5 || setting == 6 || setting == 7 || setting == 8)) {
            color = LIGHTCYAN;
            pColors->textcolor(color);
        }
        else if ((i == 6) && (setting == 12 || setting == 13 || setting == 14)) {
            color = LIGHTCYAN;
            pColors->textcolor(color);
        }

        if ((i == 7) && (setting == 9)) {
            color = LIGHTGREEN;
            pColors->textcolor(color);
        }
        else if ((i == 8) && (setting == 10 || setting == 5)) {
            color = LIGHTGREEN;
            pColors->textcolor(color);
        }
        else if ((i == 10) && (setting == 11)) {
            color = LIGHTGREEN;
            pColors->textcolor(color);
        }
        else if ((i == 9) &&
            (
                setting == 1 || setting == 2 || setting == 3 || setting == 4 || setting == 6 ||
                setting == 7 || setting == 8 || setting == 12 || setting == 13 || setting == 14
            )
        ) {
            color = LIGHTGREEN;
            pColors->textcolor(color);
        }

        if (i == 16) {
            color = LIGHTGRAY;
            pColors->textcolor(color);
        }
    }
}

int Match::whatHappened(
    bool isPlayerBall,
    int MonM,
    int DonA,
    int AonD,
    int playerGoalkeeperSkills,
    int rivalGoalkeeperSkills,
    int whereIsAction,
    const SClub &clubRef
) {
    int x1, x2, x3, los, what;
    if (whereIsAction == ACTION_IN_MIDDLEFIELD) { // P/P
        if (isPlayerBall) {
            x1 = clubRef.inst[4]; //gra z kontry twoja
            x2 = clubRef.trained[1]; //podania trening
            x3 = clubRef.rivalInst[2]; //pressing rywala
        }
        else {
            x1 = clubRef.rivalInst[4]; //gra z kontry rywala
            x2 = 2; //podania trening rywala
            x3 = clubRef.inst[2]; //pressing twój
        }
        los = pRand->get(27);
        if (los < 10) {
            what = los;
        }
        else {
            if ((MonM > 20 && isPlayerBall) || (MonM < -20 && !isPlayerBall)) { // przewaga duża
                if (x1 == 0 && x2 >= 2 && x3 == 0) {
                    what = 1;
                }
                else if (x1 == 0 && x2 >= 2 && x3 == 1) {
                    los = pRand->get(18);
                    if (los == 1 || los == 2 || los == 3) what = 6;
                    else what = 1;
                }
                else if (x1 == 0 && x2 < 2 && x3 == 0) {
                    los = pRand->get(18);
                    if (los == 1 || los == 2) what = 5;
                    else what = 1;
                }
                else if (x1 == 0 && x2 < 2 && x3 == 1) {
                    los = pRand->get(18);
                    if (los == 1 || los == 2) what = 6;
                    else if (los == 3 || los == 4) what = 5;
                    else what = 1;
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 0) {
                    los = pRand->get(18);
                    if (los == 1 || los == 2 || los == 3) what = 1; //udana normal
                    else what = 2; //udana z kontry
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 1) {
                    los = pRand->get(18);
                    if (los == 1 || los == 2 || los == 3) what = 1; //udana normal
                    else if (los == 4 || los == 5 || los == 6) what = 6; //pressing rywala
                    else what = 2; //udana z kontry
                }
                else if (x1 == 1 && x2 < 2 && x3 == 0) {
                    los = pRand->get(18);
                    if (los == 1 || los == 2 || los == 3) what = 1; //udana normal
                    else if (los == 4 || los == 5) what = 5; //słabe podania
                    else what = 2; //udana z kontry
                }
                else if (x1 == 1 && x2 < 2 && x3 == 1) {
                    los = pRand->get(18);
                    if (los == 1 || los == 2 || los == 3) what = 1; //udana normal
                    else if (los == 4 || los == 5) what = 5; //słabe podania
                    else if (los == 6 || los == 7) what = 6; //pressing rywala
                    else what = 2; //udana z kontry
                }
            }
            else if ((MonM > 10 && MonM < 21 && isPlayerBall) || (MonM < -10 && MonM > -21 && !isPlayerBall))//przewaga mała
            {
                if (x1 == 0 && x2 >= 2 && x3 == 0) {
                    if (los == 14 || los == 22) what = 4;
                    else if (los == 15 || los == 23 || los == 26) what = 7;
                    else if (los == 16 || los == 24 || los == 27) what = 8;
                    else if (los == 17 || los == 18) what = 9;
                    else what = 1; //if (los==10||los==11||los==12||los==13||los==19||los==20||los==21||los==25)
                }
                else if (x1 == 0 && x2 >= 2 && x3 == 1) {
                    if (los == 13 || los == 21 || los == 22 || los == 26) what = 6; //pressing przeciwnika
                    else if (los == 14 || los == 23) what = 4;
                    else if (los == 15 || los == 24) what = 7;
                    else if (los == 16) what = 8;
                    else if (los == 17 || los == 18) what = 9;
                    else what = 1; //if (los==10||los==11||los==12||los==19||los==20||los==25||los==27)
                }
                else if (x1 == 0 && x2 < 2 && x3 == 0) {
                    if (los == 13 || los == 23 || los == 24 || los == 27) what = 5; //słabe podania
                    else if (los == 14) what = 4;
                    else if (los == 15 || los == 25 || los == 26) what = 7;
                    else if (los == 16) what = 8;
                    else if (los == 17 || los == 18) what = 9;
                    else what = 1; //if (los==10||los==11||los==12||los==19||los==20||los==21||los==22)
                }
                else if (x1 == 0 && x2 < 2 && x3 == 1) {
                    if (los == 12 || los == 22 || los == 24) what = 6; //pressing przeciwnika
                    else if (los == 13 || los == 23 || los == 25) what = 5; //słabe podania
                    else if (los == 14) what = 4;
                    else if (los == 15 || los == 26) what = 7;
                    else if (los == 16) what = 8;
                    else if (los == 17 || los == 18) what = 9;
                    else what = 1; //if (los==10||los==11||los==19||los==20||los==21||los==27)
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 0) {
                    if (los == 13 || los == 24 || los == 25 || los == 26) what = 3; //nie udana z kontry
                    else if (los == 14) what = 4;
                    else if (los == 15 || los == 27) what = 7;
                    else if (los == 16) what = 8;
                    else if (los == 17 || los == 18) what = 9;
                    else what = 2; //udana z kontryif (los==10||los==11||los==12||los==19||los==20||los==21||los==22||los==23)
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 1) {
                    if (los == 12 || los == 23 || los == 25) what = 6; //pressing przeciwnika
                    else if (los == 13 || los == 24 || los == 14) what = 3; //nie udana z kontry
                    else if (los == 15 || los == 26) what = 7;
                    else if (los == 16) what = 8;
                    else if (los == 17 || los == 18) what = 9;
                    else what = 2; //udana z kontry if (los==10||los==11||los==19||los==20||los==21||los==22||los==27)
                }
                else if (x1 == 1 && x2 < 2 && x3 == 0) {
                    if (los == 12 || los == 23 || los == 24) what = 5; //słaby trening podaä
                    else if (los == 13 || los == 26 || los == 14) what = 3; //nie udana z kontry
                    else if (los == 15 || los == 27) what = 7;
                    else if (los == 16) what = 8;
                    else if (los == 17 || los == 18) what = 9;
                    else what = 2; //udana z kontry if (los==10||los==11||los==19||los==20||los==21||los==22||los==25)
                }
                else { //if (x1==1&&x2<2&&x3==1)
                    if (los == 13 || los == 22 || los == 26) what = 5; //słaby trening podaä
                    else if (los == 14 || los == 23 || los == 27) what = 3; //nie udana z kontry
                    else if (los == 16) what = 7;
                        //else if (los==7) what=8;//aut
                    else if (los == 17) what = 9;
                    else if (los == 15 || los == 18 || los == 24) what = 6; //pressing rywala3x
                    else what = 2; //udana z kontryif (los==10||los==11||los==12||los==19||los==20||los==21||los==25)
                }
            }
            else if ((MonM > 20 && !isPlayerBall) || (MonM < -20 && isPlayerBall)) { //osłabienie duże
                if (x1 == 0 && x2 >= 2 && x3 == 0){
                    what = 4;
                }
                else if (x1 == 0 && x2 >= 2 && x3 == 1) {
                    los = pRand->get(18);
                    if (los == 1 || los == 2 || los == 3) what = 4;
                    else what = 6;
                }
                else if (x1 == 0 && x2 < 2 && x3 == 0) {
                    los = pRand->get(18);
                    if (los == 1 || los == 2) what = 4;
                    else what = 5;
                }
                else if (x1 == 0 && x2 < 2 && x3 == 1) {
                    los = pRand->get(18);
                    if (los == 1 || los == 2) what = 5;
                    else if (los == 3 || los == 4) what = 4;
                    else what = 6;
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 0) {
                    los = pRand->get(18);
                    if (los == 1 || los == 2 || los == 3) what = 4; //nieudana normal
                    else what = 3; //nieudana z kontry
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 1) {
                    los = pRand->get(18);
                    if (los == 1 || los == 2 || los == 3) what = 4; //nieudana normal
                    else if (los == 4 || los == 5 || los == 6) what = 6; //pressing rywala
                    else what = 3; //nieudana z kontry
                }
                else if (x1 == 1 && x2 < 2 && x3 == 0) {
                    los = pRand->get(18);
                    if (los == 1 || los == 2 || los == 3) what = 4; //nieudana normal
                    else if (los == 4 || los == 5) what = 5; //słabe podania
                    else what = 3; //nieudana z kontry
                }
                else if (x1 == 1 && x2 < 2 && x3 == 1) {
                    los = pRand->get(18);
                    if (los == 1 || los == 2 || los == 3) what = 4; //nieudana normal
                    else if (los == 4 || los == 5) what = 5; //słabe podania
                    else if (los == 6 || los == 7) what = 6; //pressing rywala
                    else what = 3; //nieudana z kontry
                }
            }
            else if ((MonM < -10 && MonM > -21 && isPlayerBall) || (MonM > 10 && MonM < 21 && !isPlayerBall)) { //osłabienie
                if (x1 == 0 && x2 >= 2 && x3 == 0) {
                    if (los == 14 || los == 22 || los == 23 || los == 27) what = 1;
                    else if (los == 15 || los == 24) what = 7;
                    else if (los == 16 || los == 25) what = 8;
                    else if (los == 17 || los == 18) what = 9;
                    else what = 4; //if (los==10||los==11||los==12||los==13||los==19||los==20||los==21||los==26)
                }
                else if (x1 == 0 && x2 >= 2 && x3 == 1) {
                    if (los == 10 || los == 26) what = 4;
                    else if (los == 14 || los == 23 || los == 25 || los == 27) what = 1;
                    else if (los == 15 || los == 24) what = 7;
                    else if (los == 16) what = 8;
                    else if (los == 17 || los == 18) what = 9;
                    else what = 6; //pressing przeciwnikaif (los==11||los==12||los==13||los==19||los==20||los==21||los==22)
                }
                else if (x1 == 0 && x2 < 2 && x3 == 0) {
                    if (los == 10 || los == 23) what = 4;
                    else if (los == 14 || los == 25 || los == 26 || los == 27) what = 1;
                    else if (los == 15 || los == 24) what = 7;
                    else if (los == 16) what = 8;
                    else if (los == 17 || los == 18) what = 9;
                    else what = 5; //słabe podania if (los==11||los==12||los==13||los==19||los==20||los==21||los==22)
                }
                else if (x1 == 0 && x2 < 2 && x3 == 1) {
                    if (los == 10) what = 4;
                    else if (los == 11 || los == 12 || los == 19 || los == 20) what = 6; //pressing przeciwnika
                    else if (los == 13 || los == 22 || los == 23 || los == 21) what = 5; //słabe podania
                    else if (los == 14 || los == 25 || los == 26 || los == 27) what = 1;
                    else if (los == 15 || los == 24) what = 7;
                    else if (los == 16) what = 8;
                    else what = 9;
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 0) {
                    //if (los==10||los==11||los==12||los==19||los==20||los==21||los==27) what=3;//nie udana z kontry
                    if (los == 13 || los == 22 || los == 23 || los == 24) what = 2; //udana z kontry
                    else if (los == 14 || los == 25) what = 4;
                    else if (los == 15 || los == 26) what = 7;
                    else if (los == 16) what = 8;
                    else if (los == 17 || los == 18) what = 9;
                    else what = 3; //nie udana z kontry if (los==10||los==11||los==12||los==19||los==20||los==21||los==27)
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 1) {
                    if (los == 10 || los == 11 || los == 20 || los == 21) what = 3; //nieudana z kontry
                    else if (los == 12 || los == 22 || los == 23 || los == 27) what = 6; //pressing przeciwnika
                    else if (los == 13 || los == 24 || los == 25 || los == 19) what = 2; //udana z kontry
                    else if (los == 14) what = 4;
                    else if (los == 15 || los == 26) what = 7;
                    else if (los == 16) what = 8;
                    else what = 9;
                }
                else if (x1 == 1 && x2 < 2 && x3 == 0) {
                    if (los == 10 || los == 11 || los == 19 || los == 20) what = 3; //nieudana z kontry
                    else if (los == 12 || los == 21 || los == 22 || los == 23) what = 5; //słaby trening podaä
                    else if (los == 13 || los == 24 || los == 25 || los == 26) what = 2; //udana z kontry
                    else if (los == 14) what = 4;
                    else if (los == 15 || los == 27) what = 7;
                    else if (los == 16) what = 8;
                    else what = 9;
                }
                else { //if (x1==1&&x2<2&&x3==1)
                    if (los == 10 || los == 11 || los == 12 || los == 19) what = 3; //nieudana z kontry
                    else if (los == 13 || los == 22 || los == 23) what = 5; //słaby trening podaä
                    else if (los == 14 || los == 24 || los == 25 || los == 26) what = 2; // udana z kontry
                    else if (los == 15) what = 4;
                    else if (los == 16 || los == 27) what = 7;
                        //else if (los==7) what=8;//aut
                    else if (los == 17) what = 9;
                    else what = 6; //pressing rywala3x
                }
            }
            else { //if (MonM>=-10&&MonM<=10)//siły wyrównane
                if (x1 == 0 && x2 >= 2 && x3 == 0) {
                    if (los == 10 || los == 11 || los == 19 || los == 24 || los == 25) what = 1;
                    else if (los == 12 || los == 13 || los == 20 || los == 26) what = 4;
                    else if (los == 14 || los == 15 || los == 21) what = 7;
                    else if (los == 16 || los == 17 || los == 22 || los == 27) what = 8;
                    else what = 9;
                }
                else if (x1 == 0 && x2 >= 2 && x3 == 1) {
                    if (los == 10 || los == 11 || los == 19 || los == 20 || los == 24) what = 1;
                    else if (los == 12 || los == 21) what = 4;
                    else if (los == 13 || los == 22) what = 6; //pressing rywala
                    else if (los == 14 || los == 15 || los == 23) what = 7;
                    else if (los == 16 || los == 17 || los == 25 || los == 26) what = 8;
                    else what = 9;
                }
                else if (x1 == 0 && x2 < 2 && x3 == 0) {
                    if (los == 10 || los == 11 || los == 25 || los == 26 || los == 27) what = 1;
                    else if (los == 12 || los == 24) what = 4;
                    else if (los == 13 || los == 23) what = 5; //słabe podania
                    else if (los == 14 || los == 15 || los == 22) what = 7;
                    else if (los == 16 || los == 17 || los == 20 || los == 21) what = 8;
                    else what = 9;
                }
                else if (x1 == 0 && x2 < 2 && x3 == 1) {
                    if (los == 10 || los == 11 || los == 25 || los == 26 || los == 27) what = 1;
                    else if (los == 12 || los == 24) what = 6; //pressing przeciwnika
                    else if (los == 13 || los == 23) what = 5; //słabe podania
                    else if (los == 14 || los == 15 || los == 22) what = 7;
                    else if (los == 16 || los == 17 || los == 20 || los == 21) what = 8;
                    else what = 9;
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 0) {
                    if (los == 10 || los == 11 || los == 25 || los == 26 || los == 27) what = 2; //udanie z kontry
                    else if (los == 12 || los == 13 || los == 23 || los == 24) what = 3; //nieudana kontra
                    else if (los == 14 || los == 15 || los == 22) what = 7;
                    else if (los == 16 || los == 17 || los == 20 || los == 21) what = 8;
                    else what = 9;
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 1) {
                    if (los == 10 || los == 11 || los == 25 || los == 26 || los == 27) what = 2; //udanie z kontry
                    else if (los == 12 || los == 24) what = 3; //nieudana kontra
                    else if (los == 13 || los == 23) what = 6; //pressing przeciwnika
                    else if (los == 14 || los == 15 || los == 22) what = 7;
                    else if (los == 16 || los == 17 || los == 20 || los == 21) what = 8;
                    else what = 9;
                }
                else if (x1 == 1 && x2 < 2 && x3 == 0) {
                    if (los == 10 || los == 11 || los == 25 || los == 26 || los == 27) what = 2; //udanie z kontry
                    else if (los == 12 || los == 24) what = 3; //nieudana kontra
                    else if (los == 13 || los == 23) what = 5; //słaby trening
                    else if (los == 14 || los == 15 || los == 22) what = 7;
                    else if (los == 16 || los == 17 || los == 20 || los == 21) what = 8;
                    else what = 9;
                }
                else { //if (x1==1&&x2<2&&x3==1)
                    if (los == 10 || los == 11 || los == 25 || los == 26 || los == 27) what = 2; //udanie z kontry
                    else if (los == 12 || los == 24) what = 3; //nieudana kontra
                    else if (los == 13 || los == 23) what = 5; //słaby trening
                    else if (los == 14 || los == 15 || los == 22) what = 7;
                    else if (los == 16 || los == 21) what = 8;
                    else if (los == 17 || los == 20) what = 6; //pressing rywala
                    else what = 9;
                }
            }
        }
    }//whereIsAction = ACTION_IN_MIDDLEFIELD  P/P
    else if (whereIsAction == ACTION_IN_WINGER) { // winger has a ball, A/D
        los = pRand->get(12);
        if (los < 5) {
            what = los + 9;
        }
        else {
            if ((AonD > 10 && isPlayerBall) || (DonA < -10 && !isPlayerBall)) { //duża przewaga ataku
                los = pRand->get(10);
                if (los == 1 || los == 2) what = 11;
                else what = 10;
            }
            else if ((AonD > 0 && AonD < 11 && isPlayerBall) || (DonA > -11 && DonA < 0 && !isPlayerBall)) { //przewaga ataku
                if (los == 6 || los == 7 || los == 8 || los == 9) what = 10;
                else if (los == 10 || los == 11 || los == 12) what = 11;
                else what = 13;
            }
            else if ((AonD < -40 && isPlayerBall) || (DonA > 40 && !isPlayerBall)) { //duża przewaga obrony
                los = pRand->get(10);
                if (los == 1 || los == 2) what = 12;
                else what = 13;
            }
            else if ((AonD > -41 && AonD < -20 && isPlayerBall) || (DonA > 20 && DonA < 41 && !isPlayerBall)) { //przewaga obrony
                if (los == 5 || los == 6 || los == 7 || los == 8 || los == 9) what = 13;
                else if (los == 10 || los == 11) what = 10;
                else what = 11;
            }
            else { //if ((AonD > -21&&AonD<1&& isPlayerBall)||(DonA<21&&DonA > -1&& !isPlayerBall))
                if (los > 4 && los < 9) what = los + 5;
                else what = los + 1;
            }
        }
    }
    else if (whereIsAction == ACTION_IN_PANELTY_AREA) { // penalty area A/D
        los = pRand->get(30);
        if (isPlayerBall) {
            x1 = clubRef.rivalInst[3]; // offsides trap by rival
        }
        else {
            x1 = clubRef.inst[3];
        }

        if (los < 11) {
            what = los + 13;
        }
        else {
            if ((AonD > 10 && isPlayerBall) || (DonA < -10 && !isPlayerBall)) { //duża przewaga ataku
                if (x1 == 0) {
                    los = pRand->get(10);
                    if (los == 1 || los == 2 || los == 3) what = 21;
                    else what = 22;
                }
                else if (x1 == 1) {
                    los = pRand->get(15);
                    if (los == 1 || los == 2 || los == 3) what = 21;
                    else if (los == 4 || los == 5 || los == 6) what = 14;
                    else if (los == 7) what = 15;
                    else what = 22;
                }
            }
            else if ((AonD > 0 && AonD < 11 && isPlayerBall) || (DonA > -11 && DonA < 0 && !isPlayerBall)) { //przewaga ataku
                if (x1 == 0) {
                    if (los == 11 || los == 27) what = 16;
                    else if (los == 12 || los == 26) what = 17;
                    else if (los == 13 || los == 25) what = 18;
                    else if (los == 14 || los == 15 || los == 24 || los == 30) what = 20;
                    else if (los == 16 || los == 17 || los == 23) what = 21;
                    else if (los == 18 || los == 19 || los == 21 || los == 22 || los == 29) what = 22;
                    else what = 23;
                }
                else if (x1 == 1) {
                    if (los == 11 || los == 23 || los == 27) what = 14;
                    else if (los == 12 || los == 28) what = 15;
                    else if (los == 13) what = 16;
                    else if (los == 14) what = 17;
                    else if (los == 15 || los == 29) what = 18;
                    else if (los == 16 || los == 24) what = 20;
                    else if (los == 17 || los == 22 || los == 26) what = 21;
                    else if (los == 18 || los == 19 || los == 21 || los == 25 || los == 30) what = 22;
                    else what = 23;
                }
            }
            else if ((AonD < -40 && isPlayerBall) || (DonA > 40 && !isPlayerBall)) { //duża przewaga obrony
                if (x1 == 0) {
                    los = pRand->get(10);
                    if (los == 1 || los == 2 || los == 3) what = 17;
                    else what = 16;
                }
                else if (x1 == 1) {
                    los = pRand->get(15);
                    if (los == 1 || los == 2 || los == 3) what = 17;
                    else if (los == 4 || los == 5 || los == 6) what = 16;
                    else if (los == 7) what = 15;
                    else what = 14;
                }
            }
            else if ((AonD > -41 && AonD < -20 && isPlayerBall) || (DonA > 20 && DonA < 41 && !isPlayerBall)) { //przewaga obrony
                if (x1 == 0) {
                    if (los == 11 || los == 12 || los == 13 || los == 21 || los == 22 || los == 30) what = 16;
                    else if (los == 14 || los == 23 || los == 24) what = 17;
                    else if (los == 15) what = 18;
                    else if (los == 16 || los == 17 || los == 27) what = 20;
                    else if (los == 18 || los == 25) what = 21;
                    else if (los == 19 || los == 26) what = 22;
                    else what = 23; //3x
                }
                else if (x1 == 1) {
                    if (los == 11 || los == 12 || los == 13 || los == 21 || los == 22 || los == 29) what = 14;
                    else if (los == 30) what = 16;
                    else if (los == 14 || los == 23) what = 15;
                    else if (los == 15 || los == 28) what = 17;
                    else if (los == 16) what = 18;
                    else if (los == 17 || los == 27) what = 20;
                    else if (los == 18 || los == 26) what = 21;
                    else if (los == 19 || los == 25) what = 22;
                    else what = 23;
                }
            }
            else if ((AonD > -21 && AonD < 1 && isPlayerBall) || (DonA < 21 && DonA > -1 && !isPlayerBall)) {
                if (x1 == 0) {
                    if (los == 11 || los == 12 || los == 13 || los == 21) what = 16;
                    else if (los == 30) what = 14;
                    else if (los == 14 || los == 23) what = 17;
                    else if (los == 15) what = 18;
                    else if (los == 16 || los == 17 || los == 24 || los == 27) what = 20;
                    else if (los == 18 || los == 25 || los == 29) what = 21;
                    else if (los == 19 || los == 26 || los == 22) what = 22;
                    else what = 23;
                }
                else if (x1 == 1) {
                    los = pRand->get(10);
                    what = los + 13;
                }
            }
        }
    }//dla whereIsAction == ACTION_IN_PANELTY_AREA A/O
    else if (whereIsAction == ACTION_IN_GOAL_SITUATION ||
             whereIsAction == ACTION_IN_DIRECT_FREE_KICK
    ) { //obrona B playerGoalkeeperSkills max 43, mini 8, śred 26
        los = pRand->get(12);
        if (whereIsAction == ACTION_IN_GOAL_SITUATION) {
            if (isPlayerBall) {
                x1 = rivalGoalkeeperSkills; //umiejętności bramkarza
            }
            else {
                x1 = playerGoalkeeperSkills;
            }
        }
        else if (whereIsAction == ACTION_IN_DIRECT_FREE_KICK) {
            if (isPlayerBall) {
                x1 = rivalGoalkeeperSkills - (clubRef.trained[2] * 2); //umiejętności bramkarza
            }
            else {
                x1 = playerGoalkeeperSkills - 2;
            }
        }

        if (x1 < 15) {
            if (los == 1 || los == 2 || los == 3 || los == 4) what = 28; //gol
            else if (los == 5 || los == 6 || los == 7) what = 27; //róg
            else if (los == 8 || los == 9) what = 26; //niepewnia obrona
            else if (los == 10) what = 29; //gol nieuznany
            else if (los == 11) what = 24; //obronił
            else what = 30; //strzał niecelny 1x
        }
        else if (x1 >= 15 && x1 < 25) {
            if (los == 1 || los == 2 || los == 3) what = 28; //gol
            else if (los == 4 || los == 5 || los == 6) what = 27; //róg
            else if (los == 7 || los == 8) what = 26; //niepewnia obrona
            else if (los == 9) what = 29; //gol nieuznany
            else if (los == 10 || los == 11) what = 24; //obronił
            else what = 30; //strzał niecelny
        }
        else if (x1 >= 25 && x1 < 35) {
            if (los == 1 || los == 2 || los == 3) what = 28; //gol
            else if (los == 4 || los == 5) what = 27; //róg
            else if (los == 6 || los == 7) what = 26; //niepewnia obrona
            else if (los == 8) what = 29; //gol nieuznany
            else if (los == 9 || los == 10) what = 24; //obronił
            else if (los == 11) what = 25; //bez problemu
            else what = 30; //strzał niecelny1x
        }
        else if (x1 > 34) {
            if (los == 1 || los == 2) what = 28; //gol
            else if (los == 3 || los == 4) what = 27; //róg
            else if (los == 5) what = 26; //niepewnia obrona
            else if (los == 6) what = 29; //gol nieuznany
            else if (los == 7 || los == 8) what = 24; //obronił
            else if (los == 9 || los == 10) what = 25; //bez problemu
            else what = 30; //strzał niecelny2x
        }
    } // whereIsAction = ACTION_IN_GOAL_SITUATION // obrona B
    else if (whereIsAction == ACTION_IN_PANELTY_OR_1ON1) { //obrona B - karny, sma na sam
        los = pRand->get(9);
        if (isPlayerBall) {
            x1 = rivalGoalkeeperSkills - clubRef.trained[2]; //um. bramkarza-stałe fragmenty
        }
        else {
            x1 = playerGoalkeeperSkills - 2;
        }

        if (x1 < 15) {
            if (los == 1 || los == 2 || los == 3 || los == 4 || los == 5 || los == 6 || los == 7) what = 28; //gol
            else if (los == 8) what = 24; //obronił
            else what = 30; //strzał niecelny 1x
        }
        else if (x1 >= 15 && x1 < 25) {
            if (los == 1 || los == 2 || los == 3 || los == 4 || los == 5 || los == 6) what = 28; //gol
            else if (los == 8) what = 24; //obronił
            else what = 30; //strzał niecelny 2x
        }
        else if (x1 >= 25 && x1 < 35) {
            if (los == 1 || los == 2 || los == 3 || los == 4 || los == 5) what = 28; //gol
            else if (los == 8 || los == 6) what = 24; //obronił
            else what = 30; //strzał niecelny 2x
        }
        else if (x1 > 34) {
            if (los == 1 || los == 2 || los == 3 || los == 4) what = 28; //gol
            else if (los == 8 || los == 6 || los == 5) what = 24; //obronił
            else what = 30; //strzał niecelny 2x
        }
    }

    return what;
}

void Match::drawBoard(
    bool isHome,
    int shootHome, int shootAway,
    int accurateShootHome, int accurateShootAway,
    int cornersHome, int cornersAway,
    float zoneLeft, float zoneCenter, float zoneRight,
    int offsidesHome, int offsidesAway,
    int foulsHome, int foulsAway,
    int paneltiesHome, int paneltiesAway,
    int yellowsHome, int yellowsAway,
    int redsHome, int redsAway,
    int ballPossHome, int ballPossAway,
    int DonA, int MonM, int AonD,
    int whereIsAction,
    bool isPlayerBall
) {
    pColors->textcolor(WHITE);
    pColors->textbackground(GREEN);
    wcout << endl << BOX_LIGHT_DOWN_RIGHT <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_DOWN_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_DOWN_LEFT;


    pColors->textbackground(BLACK);
    wprintf(pLang->get(L"     %2d     Shots       %2d").c_str(), shootHome, shootAway);

    pColors->textbackground(GREEN);
    wcout << endl << BOX_LIGHT_VERTICAL;

    pColors->textcolor(isHome ? BLUE : RED);
    wcout << L"   " << pLang->get(L"D");

    pColors->textcolor(WHITE);
    wcout << L"/";

    pColors->textcolor(isHome ? RED : BLUE);
    wcout << pLang->get(L"A") << L"   ";

    pColors->textcolor(WHITE);
    wcout << BOX_LIGHT_DASH_VERTICAL;

    pColors->textcolor(isHome ? BLUE : RED);
    wcout << L"     " << pLang->get(L"M");

    pColors->textcolor(WHITE);
    wcout << L"/";

    pColors->textcolor(isHome ? RED : BLUE);
    wcout << pLang->get(L"M") << L"     ";

    pColors->textcolor(WHITE);
    wcout << BOX_LIGHT_DASH_VERTICAL;

    pColors->textcolor(isHome ? BLUE : RED);
    wcout << L"   " << pLang->get(L"A");

    pColors->textcolor(WHITE);
    wcout << L"/";

    pColors->textcolor(isHome ? RED : BLUE);
    wcout << pLang->get(L"D") << L"   ";

    pColors->textcolor(WHITE);
    wcout << BOX_LIGHT_VERTICAL;

    pColors->textbackground(BLACK);
    wprintf(pLang->get(L"     %2d Shots on target %2d").c_str(), accurateShootHome, accurateShootAway);

    pColors->textbackground(GREEN);
    wcout << endl <<
            BOX_LIGHT_VERTICAL_RIGHT <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_DOWN_LEFT <<
            L"    " << BOX_LIGHT_DASH_VERTICAL << L"      " <<
            BOX_LIGHT_VERTICAL <<
            L"      " << BOX_LIGHT_DASH_VERTICAL << L"    " <<
            BOX_LIGHT_DOWN_RIGHT <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_VERTICAL_LEFT;

    pColors->textbackground(BLACK);
    wprintf(pLang->get(L"     %2d     Corners     %2d").c_str(), cornersHome, cornersAway);

    pColors->textbackground(GREEN);
    wcout << endl <<
            BOX_LIGHT_VERTICAL_RIGHT <<
            BOX_LIGHT_DOWN_LEFT;

    wprintf(L" %3.0f%%   ", zoneLeft);
    wcout << BOX_LIGHT_DASH_VERTICAL;
    wprintf(L"    %3.0f%%     ", zoneCenter);
    wcout << BOX_LIGHT_DASH_VERTICAL;
    wprintf(L"  %3.0f%%  ", zoneRight);
    wcout << BOX_LIGHT_DOWN_RIGHT <<
            BOX_LIGHT_VERTICAL_LEFT;

    pColors->textbackground(BLACK);
    wprintf(pLang->get(L"     %2d     Offsides    %2d").c_str(), offsidesHome, offsidesAway);

    pColors->textbackground(GREEN);
    wcout << endl <<
            BOX_LIGHT_VERTICAL <<
            BOX_LIGHT_VERTICAL <<
            L" " << BOX_SMALL_DOT << L" " <<
            BOX_LIGHT_VERTICAL <<
            L"    " << BOX_LIGHT_DASH_VERTICAL << L"      " <<
            BOX_LIGHT_CROSS <<
            L"      " << BOX_LIGHT_DASH_VERTICAL << L"    " <<
            BOX_LIGHT_VERTICAL <<
            L" " << BOX_SMALL_DOT << L" " <<
            BOX_LIGHT_VERTICAL <<
            BOX_LIGHT_VERTICAL;

    pColors->textbackground(BLACK);
    wprintf(pLang->get(L"     %2d      Fouls      %2d").c_str(), foulsHome, foulsAway);

    pColors->textbackground(GREEN);
    wcout << endl <<
            BOX_LIGHT_VERTICAL_RIGHT <<
            BOX_LIGHT_UP_LEFT <<
            L"   " <<
            BOX_LIGHT_VERTICAL <<
            L"    " << BOX_LIGHT_DASH_VERTICAL << L"      " <<
            BOX_LIGHT_VERTICAL <<
            L"      " << BOX_LIGHT_DASH_VERTICAL << L"    " <<
            BOX_LIGHT_VERTICAL <<
            L"   " <<
            BOX_LIGHT_UP_RIGHT <<
            BOX_LIGHT_VERTICAL_LEFT;

    pColors->textbackground(BLACK);
    wprintf(pLang->get(L"     %2d    Penalties    %2d").c_str(), paneltiesHome, paneltiesAway);

    pColors->textbackground(GREEN);
    wcout << endl <<
            BOX_LIGHT_VERTICAL_RIGHT <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_UP_LEFT <<
            L"    " << BOX_LIGHT_DASH_VERTICAL << L"      " <<
            BOX_LIGHT_VERTICAL <<
            L"      " << BOX_LIGHT_DASH_VERTICAL << L"    " <<
            BOX_LIGHT_UP_RIGHT <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_VERTICAL_LEFT;

    pColors->textbackground(BLACK);
    pColors->textcolor(YELLOW);
    wprintf(pLang->get(L"     %2d   Yellow cards  %2d").c_str(), yellowsHome, yellowsAway);

    pColors->textcolor(WHITE);
    pColors->textbackground(GREEN);
    wcout << endl << BOX_LIGHT_VERTICAL << L"  ";
    isHome
        ? drawOnaA(isHome, DonA)
        : drawAnaO(isHome, AonD);

    pColors->textcolor(WHITE);
    wcout << L"   " << BOX_LIGHT_DASH_VERTICAL << L"    ";
    if (MonM < -20) {
        pColors->textcolor(RED);
        wcout << BALL_FULL << BALL_FULL << BALL_FULL << BALL_FULL;
    }
    else if (MonM >= -20 && MonM < -10) {
        if (isHome) {
            pColors->textcolor(BLUE);
            wcout << BALL_FULL;
            pColors->textcolor(RED);
            wcout << BALL_FULL << BALL_FULL << BALL_FULL;
        }
        else {
            pColors->textcolor(RED);
            wcout << BALL_FULL << BALL_FULL << BALL_FULL;
            pColors->textcolor(BLUE);
            wcout << BALL_FULL;
        }
    }
    else if (MonM >= -10 && MonM < 11) {
        if (isHome) {
            pColors->textcolor(BLUE);
            wcout << BALL_FULL << BALL_FULL;
            pColors->textcolor(RED);
            wcout << BALL_FULL << BALL_FULL;
        }
        else {
            pColors->textcolor(RED);
            wcout << BALL_FULL << BALL_FULL;
            pColors->textcolor(BLUE);
            wcout << BALL_FULL << BALL_FULL;
        }
    }
    else if (MonM >= 11 && MonM < 21) {
        if (isHome) {
            pColors->textcolor(BLUE);
            wcout << BALL_FULL << BALL_FULL << BALL_FULL;
            pColors->textcolor(RED);
            wcout << BALL_FULL;
        }
        else {
            pColors->textcolor(RED);
            wcout << BALL_FULL;
            pColors->textcolor(BLUE);
            wcout << BALL_FULL << BALL_FULL << BALL_FULL;
        }
    }
    else if (MonM >= 21) {
        pColors->textcolor(BLUE);
        wcout << BALL_FULL << BALL_FULL << BALL_FULL << BALL_FULL;
    }

    pColors->textcolor(WHITE);
    wcout << L"     " << BOX_LIGHT_DASH_VERTICAL << L"   ";

    isHome
        ? drawAnaO(isHome, AonD)
        : drawOnaA(isHome, DonA);

    pColors->textcolor(WHITE);
    wcout << L"  " << BOX_LIGHT_VERTICAL;

    pColors->textcolor(LIGHTRED);
    pColors->textbackground(BLACK);
    wprintf(pLang->get(L"     %2d    Red cards    %2d").c_str(), redsHome, redsAway);

    pColors->textcolor(WHITE);
    pColors->textbackground(GREEN);
    wcout << endl << BOX_LIGHT_UP_RIGHT << BOX_LIGHT_HORIZONTAL;

    pColors->textcolor(BLACK);
    wprintf(L"%4d", isHome ? DonA : AonD);

    pColors->textcolor(WHITE);
    wcout << BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL;
    pColors->textcolor(BLACK);
    wprintf(L"%4d", MonM);
    pColors->textcolor(WHITE);
    wcout << BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL <<
            BOX_LIGHT_HORIZONTAL;
    pColors->textcolor(BLACK);
    wprintf(L"%4d", isHome ? AonD : DonA);
    pColors->textcolor(WHITE);
    wcout << BOX_LIGHT_HORIZONTAL << BOX_LIGHT_HORIZONTAL << BOX_LIGHT_UP_LEFT;

    pColors->textbackground(BLACK);
    wprintf(pLang->get(L"   %3d%%  Ball possession %d%%").c_str(), ballPossHome, ballPossAway);

    pColors->textbackground(BLACK);
    pColors->textcolor(WHITE);
    wcout << endl;
    if (whereIsAction == ACTION_IN_MIDDLEFIELD) { // srodek pola
        wcout << L"                 ^";
    }
    else {
        if (isPlayerBall == isHome) {
            wcout << L"                              ^";
        }
        else {
            wcout << L"    ^";
        }
    }
}

void Match::drawOnaA(bool isHome, int DonA)
{
    if (DonA < -10) {
        pColors->textcolor(RED);
        wcout << BALL_FULL << BALL_FULL << BALL_FULL << BALL_FULL;
    }
    else if (DonA >= -10 && DonA < 0) {
        if (isHome) {
            pColors->textcolor(BLUE);
            wcout << BALL_FULL;
            pColors->textcolor(RED);
            wcout << BALL_FULL << BALL_FULL << BALL_FULL;
        }
        else {
            pColors->textcolor(RED);
            wcout << BALL_FULL << BALL_FULL << BALL_FULL;
            pColors->textcolor(BLUE);
            wcout << BALL_FULL;
        }
    }
    else if (DonA >= 0 && DonA < 21) {
        if (isHome) {
            pColors->textcolor(BLUE);
            wcout << BALL_FULL << BALL_FULL;
            pColors->textcolor(RED);
            wcout << BALL_FULL << BALL_FULL;
        }
        else {
            pColors->textcolor(RED);
            wcout << BALL_FULL << BALL_FULL;
            pColors->textcolor(BLUE);
            wcout << BALL_FULL << BALL_FULL;
        }
    }
    else if (DonA >= 21 && DonA < 41) {
        if (isHome) {
            pColors->textcolor(BLUE);
            wcout << BALL_FULL << BALL_FULL << BALL_FULL;
            pColors->textcolor(RED);
            wcout << BALL_FULL;
        }
        else {
            pColors->textcolor(RED);
            wcout << BALL_FULL;
            pColors->textcolor(BLUE);
            wcout << BALL_FULL << BALL_FULL << BALL_FULL;
        }
    }
    else if (DonA >= 41) {
        pColors->textcolor(BLUE);
        wcout << BALL_FULL << BALL_FULL << BALL_FULL << BALL_FULL;
    }
}

void Match::drawAnaO(bool isHome, int AonD)
{
    if (AonD < -40) {
        pColors->textcolor(RED);
        wcout << BALL_FULL << BALL_FULL << BALL_FULL << BALL_FULL;
    }
    else if (AonD >= -40 && AonD < -20) {
        if (isHome) {
            pColors->textcolor(BLUE);
            wcout << BALL_FULL;
            pColors->textcolor(RED);
            wcout << BALL_FULL << BALL_FULL << BALL_FULL;
        }
        else {
            pColors->textcolor(RED);
            wcout << BALL_FULL << BALL_FULL << BALL_FULL;
            pColors->textcolor(BLUE);
            wcout << BALL_FULL;
        }
    }
    else if (AonD >= -20 && AonD < 1) {
        if (isHome) {
            pColors->textcolor(BLUE);
            wcout << BALL_FULL << BALL_FULL;
            pColors->textcolor(RED);
            wcout << BALL_FULL << BALL_FULL;
        }
        else {
            pColors->textcolor(RED);
            wcout << BALL_FULL << BALL_FULL;
            pColors->textcolor(BLUE);
            wcout << BALL_FULL << BALL_FULL;
        }
    }
    else if (AonD >= 1 && AonD < 11) {
        if (isHome) {
            pColors->textcolor(BLUE);
            wcout << BALL_FULL << BALL_FULL << BALL_FULL;
            pColors->textcolor(RED);
            wcout << BALL_FULL;
        }
        else {
            pColors->textcolor(RED);
            wcout << BALL_FULL;
            pColors->textcolor(BLUE);
            wcout << BALL_FULL << BALL_FULL << BALL_FULL;
        }
    }
    else if (AonD >= 11) {
        pColors->textcolor(BLUE);
        wcout << BALL_FULL << BALL_FULL << BALL_FULL << BALL_FULL;
    }
}

void Match::drawWhoScored(const vector<SGoalsInfo> &home, const vector<SGoalsInfo> &away)
{
    size_t homeSize = home.size();
    size_t awaySize = away.size();
    size_t max = std::max(homeSize, awaySize);
    for (size_t i = 0; i < max; i++) {
        wcout << endl;

        if (i < homeSize) {
            const SGoalsInfo homeItem = home[i];
            wprintf(L"%19ls %2d ", homeItem.footballerName.c_str(), homeItem.minute);
        }

        if (i < awaySize) {
            if (i >= homeSize) {
                wcout << L"                       ";
            }

            const SGoalsInfo awayItem = away[i];
            wprintf(L" %2d %-19ls", awayItem.minute, awayItem.footballerName.c_str());
        }
    }

    if (max < 3) {
        size_t newLine = 3 - max;
        for (size_t i = 0; i < newLine; i++) {
            wcout << endl;
        }
    }
}

void Match::saveMatchMessageToFile(int bkgcolor, int matchMinute, const wchar_t *message)
{
    SLastMatch lastMatch;
    lastMatch.textcolor = bkgcolor == BLUE ? LIGHTBLUE : RED;

    swprintf(lastMatch.text, MAX_NEWS_LENGTH, pLang->get(L"%02d min. %ls").c_str(), matchMinute + 2, message);

    FILE *f = fopen(FILE_SAVE_LAST_MATCH_REPORTS, "aw");
    fwrite(&lastMatch, sizeof(SLastMatch), 1, f);
    fclose(f);
}

void Match::playerTactics()
{
    SClub &clubRef = pClub->get();

    wchar_t menuTeamSetting = 0;
    do {
        pInput->clrscr();
        pColors->textcolor(WHITE);
        wcout << L" " << pClub->getClubName(clubRef.clubId - 1);

        pTactic->drawTeamSetting(clubRef.teamSetting);

        drawTeam(clubRef.teamSetting, 11, 0);

        menuTeamSetting = pInput->getKeyBoardPressed();
        switch (menuTeamSetting) {
            case 'U': { // ustawienie
                bool isDone = false;
                while (!isDone) {
                    const wstring *settings = pTactic->getTeamSettings();
                    pInput->clrscr();
                    pColors->textbackground(BLACK);
                    pColors->textcolor(LIGHTRED);
                    wcout << pLang->get(L"Current formation: ") << settings[clubRef.teamSetting - 1];
                    pColors->textcolor(LIGHTGRAY);
                    wcout << endl << pLang->get(L"Choose a team formation:") << endl;
                    for (int i = 0; i < 14; i++) {
                        wcout << endl;
                        if (i < 9) {
                            wcout << L" ";
                        }
                        wcout << i + 1 << L". " << settings[i];
                    }
                    wcout << endl << endl << pLang->get(L"Enter the appropriate number: ");
                    clubRef.teamSetting = pInput->getNumber();
                    if (clubRef.teamSetting > 0 && clubRef.teamSetting < 15) {
                        isDone = true;
                        pClub->save();
                    }
                    else {
                        clubRef.teamSetting = 15; // wyswietl blad
                    }
                }
                break;
            }
            case 'P': {
                int footballer1;
                int footballer2;
                pColors->textcolor(LIGHTGRAY);
                wcout << endl << pLang->get(L"Enter the number, space, second number and press Enter: ");
                pInput->getNumbersExchange(footballer1, footballer2);

                for (size_t index = 0; index < pFootballers->getSizePlayerTeam(); index++) {
                    SFootballer &footballer = pFootballers->getPlayerTeam(index);
                    if ((footballer.data[0] == footballer1 && footballer.data[12] > 1) ||
                        (footballer.data[0] == footballer2 && footballer.data[12] > 1) ||
                        (footballer.data[0] == footballer1 && footballer.data[13] > 1) ||
                        (footballer.data[0] == footballer2 && footballer.data[13] > 1) ||
                        (footballer.data[0] == footballer1 && footballer.data[15] > 7) ||
                        (footballer.data[0] == footballer2 && footballer.data[15] > 5) ||
                        (footballer.data[0] == footballer1 && footballer.data[14] == 1) ||
                        (footballer.data[0] == footballer2 && footballer.data[14] == 1)
                    ) {
                        footballer1 = 1; // uniemozliwia zmiane
                        footballer2 = 1;
                    }
                }

                if (footballer1 < 1 || footballer1 > 16 || footballer2 < 1 || footballer2 > 16 || howManyPlayerChanges >= 3) {
                    footballer1 = 1; // uniemozliwia zmiane
                    footballer2 = 1;
                }

                if (footballer1 != footballer2) {
                    if ((footballer1 < 12 && footballer2 > 11) || (footballer2 < 12 && footballer1 > 11)) {
                        howManyPlayerChanges++;
                        msgWhoBall[9] = true;
                        whoPlayerChanges = 1;
                        msg[9] = 6; // %ls comes in for %ls.
                        if (footballer1 <= 11 && footballer2 > 11) { // ustalenie w wiadomosci kto kogo zastepuje
                            msgFootballers[18] = msgPlayerSurnames[footballer2 - 1];
                            msgFootballers[19] = msgPlayerSurnames[footballer1 - 1];
                        }
                        else { //if (footballer2 <= 11 && footballer1 > 11) {
                            msgFootballers[18] = msgPlayerSurnames[footballer1 - 1];
                            msgFootballers[19] = msgPlayerSurnames[footballer2 - 1];
                        }
                    }

                    for (size_t index = 0; index < pFootballers->getSizePlayerTeam(); index++) {
                        SFootballer &footballer = pFootballers->getPlayerTeam(index);
                        if (footballer.data[0] == footballer1) {
                            footballer.data[0] = footballer2;
                        }
                        else if (footballer.data[0] == footballer2) {
                            footballer.data[0] = footballer1;
                        }
                    }
                    pFootballers->savePlayerTeam();
                }

                pFootballers->sortPlayersTeam();

                pFootballers->savePlayerTeam();
                isPlayerChanges = true;
                break;
            }
            case 'R': {
                pInput->clrscr();
                pColors->textcolor(WHITE);
                wcout << pLang->get(L"SUBSTITUTE'S BENCH") << L" - " << pClub->getClubName(clubRef.clubId - 1);

                drawTeam(clubRef.teamSetting, 16, 0);

                pColors->textcolor(LIGHTGRAY);
                wcout << endl << endl << pLang->get(L"Press any key...");
                pInput->getKeyBoardPressed();
                break;
            }
            case 'I': {
                int belka = 1;
                wchar_t menuInstructions = 0;
                do {
                    pInput->clrscr();
                    pColors->textcolor(WHITE);
                    wcout << pLang->get(L"TEAM INSTRUCTIONS") << L" - " << pClub->getClubName(clubRef.clubId - 1);

                    pTeamInstruction->draw(
                        clubRef.inst[0],
                        clubRef.inst[1],
                        clubRef.inst[2],
                        clubRef.inst[3],
                        clubRef.inst[4],
                        clubRef.inst[5],
                        belka
                    );

                    pColors->textcolor(RED);
                    wcout << endl << L" " << pLang->get(L"Q Back") << endl;

                    pColors->textcolor(GREEN);
                    menuInstructions = pInput->getKeyBoardPressed();
                    switch (menuInstructions) {
                        case _KEY_UP: {
                            belka--;
                            if (belka == 0) {
                                belka = 6;
                            }
                            break;
                        }
                        case _KEY_DOWN: {
                            belka++;
                            if (belka == 7) {
                                belka = 1;
                            }
                            break;
                        }
                        case _KEY_RIGHT: {
                            if (belka == 1) {
                                clubRef.inst[5]++;
                                if (clubRef.inst[5] == 4) clubRef.inst[5] = 1;
                            }
                            else if (belka == 2) {
                                clubRef.inst[0]++;
                                if (clubRef.inst[0] == 5) clubRef.inst[0] = 1;
                            }
                            else if (belka == 3) {
                                clubRef.inst[1]++;
                                if (clubRef.inst[1] == 4) clubRef.inst[1] = 1;
                            }
                            else if (belka == 4) {
                                clubRef.inst[2]++;
                                if (clubRef.inst[2] == 2) clubRef.inst[2] = 0;
                            }
                            else if (belka == 5) {
                                clubRef.inst[3]++;
                                if (clubRef.inst[3] == 2) clubRef.inst[3] = 0;
                            }
                            else if (belka == 6) {
                                clubRef.inst[4]++;
                                if (clubRef.inst[4] == 2) clubRef.inst[4] = 0;
                            }
                            break;
                        }
                        case _KEY_LEFT: {
                            if (belka == 1) {
                                clubRef.inst[5]--;
                                if (clubRef.inst[5] == 0) clubRef.inst[5] = 3;
                            }
                            else if (belka == 2) {
                                clubRef.inst[0]--;
                                if (clubRef.inst[0] == 0) clubRef.inst[0] = 4;
                            }
                            else if (belka == 3) {
                                clubRef.inst[1]--;
                                if (clubRef.inst[1] == 0) clubRef.inst[1] = 3;
                            }
                            else if (belka == 4) {
                                clubRef.inst[2]--;
                                if (clubRef.inst[2] == -1) clubRef.inst[2] = 1;
                            }
                            else if (belka == 5) {
                                clubRef.inst[3]--;
                                if (clubRef.inst[3] == -1) clubRef.inst[3] = 1;
                            }
                            else if (belka == 6) {
                                clubRef.inst[4]--;
                                if (clubRef.inst[4] == -1) clubRef.inst[4] = 1;
                            }
                            break;
                        }
                    }

                    pClub->save();
                }
                while (menuInstructions != 'Q');
                break;
            }
        }
    }
    while (menuTeamSetting != 'Q');
}

void Match::rivalTactics()
{
    const SClub &clubRef = pClub->get();

    wchar_t menuTeamSetting = 'y';
    do {
        pInput->clrscr();
        pColors->textcolor(WHITE);
        wcout << L" " << pClub->getClubName(clubRef.rivalData[0] - 1);

        pTactic->drawTeamSetting(clubRef.rivalData[2], false);

        drawTeam(clubRef.rivalData[2], 11, clubRef.rivalData[0]);

        menuTeamSetting = pInput->getKeyBoardPressed();
        switch (menuTeamSetting) {
            case 'R': {
                pInput->clrscr();
                pColors->textcolor(WHITE);
                wcout << pLang->get(L"SUBSTITUTE'S BENCH") << L" - " << pClub->getClubName(clubRef.rivalData[0] - 1);

                drawTeam(clubRef.rivalData[2], 16, clubRef.rivalData[0]);

                pColors->textcolor(LIGHTGRAY);
                wcout << endl << endl << pLang->get(L"Press any key...");
                pInput->getKeyBoardPressed();
                break;
            }
            case 'I': {
                pInput->clrscr();
                pColors->textcolor(WHITE);
                wcout << pLang->get(L"TEAM INSTRUCTIONS") << L" - " << pClub->getClubName(clubRef.rivalData[0] - 1);

                pTeamInstruction->draw(
                    clubRef.rivalInst[0],
                    clubRef.rivalInst[1],
                    clubRef.rivalInst[2],
                    clubRef.rivalInst[3],
                    clubRef.rivalInst[4],
                    clubRef.rivalInst[5],
                    0
                );

                pColors->textcolor(LIGHTGRAY);
                wcout << endl << endl << pLang->get(L"Press any key...");
                pInput->getKeyBoardPressed();
                break;
            }
        }
    }
    while (menuTeamSetting != 'Q');
}

/**
 * @param bool isPlayerBall
 * @param int footabllerId
 * @return wstring
 */
wstring Match::getFootballerSurname(bool isPlayerBall, int footabllerId)
{
    return isPlayerBall ? msgPlayerSurnames[footabllerId] : msgRivalSurnames[footabllerId];
}

int Match::getArbiterDecision(int instrTreatment)
{
    int chance = pRand->get(6);

    switch (instrTreatment) {
        case INSTR_TREATMENT_SOFT: {
            if (chance == 1 || chance == 2 || chance == 3) {
                return 1; // nie ma kartki 1/2
            }
            else if (chance == 4 || chance == 5) {
                return 2; // słowne upomienie 1/3
            }

            return 3; // żólta kartka 1/6
        }
        case INSTR_TREATMENT_HARD: {
            if (chance == 1) {
                return 1; //nie ma kartki 1/6
            }
            else if (chance == 2 || chance == 3) {
                return 2; //słowne upomienie 1/3
            }

            return 3; //żólta kartka 1/2
        }
        case INSTR_TREATMENT_NORMAL:
        default: {
            if (chance == 1 || chance == 2) {
                return 1; // nie ma kartki 1/3
            }
            else if (chance == 3 || chance == 4) {
                return 2; // słowne upomienie 1/3
            }

            return 3; // żólta kartka 1/3
        }
    }
}

/**
 * Get ID of middlefield footballer who fouled
 *
 * @param teamSetting
 * @return
 */
int Match::getMiddlefieldFootballerIdWhoFouled(int teamSetting)
{
    switch (teamSetting) {
        case T3_4_3: {
            return pRand->get(5, 8); // middlefield 5-8
        }
        case T3_5_2:
        case T3_5_2_DEF:
        case T3_5_2_ATT: {
            return pRand->get(5, 9); // middlefield 5-9
        }
        case T4_2_4: {
            return pRand->get(6, 7);
        }
        case T4_3_3: {
            return pRand->get(6, 8);
        }
        case T4_5_1: {
            return pRand->get(6, 10);
        }
        case T5_3_2:
        case T5_3_2_DEF:
        case T5_3_2_ATT: {
            return pRand->get(7, 9);
        }
        case T4_4_2:
        case T4_4_2_DEF:
        case T4_4_2_ATT:
        case T4_4_2_DIA:
        default: {
            return pRand->get(6, 9); // middlefield 6-9
        }
    }
}

int Match::getDefFootballerId(int teamSetting)
{
    switch (teamSetting) {
        case T4_4_2:
        case T4_4_2_DEF:
        case T4_4_2_ATT:
        case T4_4_2_DIA:
        case T4_2_4:
        case T4_3_3:
        case T4_5_1: {
            return pRand->get(2, 5);
        }
        case T3_4_3:
        case T3_5_2:
        case T3_5_2_DEF:
        case T3_5_2_ATT: {
            return pRand->get(2, 4);
        }
        default: {
            return pRand->get(2, 6);
        }
    }
}

int Match::getFootballerIdWhoShootDistance(int teamSetting)
{
    switch (teamSetting) {
        case T3_5_2:
        case T3_5_2_DEF:
        case T3_5_2_ATT: {
            return pRand->get(5, 9);
        }
        case T4_5_1: {
            return pRand->get(6, 10);
        }
        case T5_3_2:
        case T5_3_2_DEF:
        case T5_3_2_ATT: {
            return pRand->get(7, 9);
        }
        case T3_4_3: {
            return pRand->get(5, 8);
        }
        case T4_3_3: {
            return pRand->get(6, 9);
        }
        default: {
            return pRand->get(6, 9);
        }
    }
}

int Match::getGoooalMinute(int matchMinute, MatchStatus matchStatus)
{
    int result = matchMinute + 2;
    if (result > 45 && (matchStatus == START_MATCH || matchStatus == END_1ST_HALF)) {
        result = 45;
    }

    if (result > 90) {
        result = 90;
    }

    return result;
}

int Match::getRightWingerFootballerId(int teamSetting)
{
    switch (teamSetting) {
        case T4_4_2:
        case T4_4_2_DEF:
        case T4_4_2_ATT:
        case T3_5_2:
        case T3_5_2_DEF:
        case T4_2_4: {
            return 8;
        }
        case T4_4_2_DIA:
        case T3_4_3:
        case T3_5_2_ATT:
        case T4_3_3: {
            return 7;
        }
        case T4_5_1: {
            return 9;
        }
        case T5_3_2:
        case T5_3_2_DEF:
        case T5_3_2_ATT:
        default: {
            return 5;
        }
    }
}

int Match::getLefttWingerFootballerId(int teamSetting)
{
    switch (teamSetting) {
        case T4_4_2:
        case T4_4_2_DEF:
        case T4_4_2_ATT:
        case T4_4_2_DIA:
        case T4_3_3:
        case T4_5_1: {
            return 5;
        }
        case T3_4_3:
        case T3_5_2:
        case T3_5_2_DEF:
        case T3_5_2_ATT:
        case T5_3_2:
        case T5_3_2_DEF:
        case T5_3_2_ATT: {
            return 4;
        }
        case T4_2_4:
        default: {
            return 7;
        }
    }
}

int Match::getFooballerStats(const SFootballer& footballer)
{
    return footballer.data[7] + footballer.data[9] + footballer.data[20] + footballer.data[11] / 10;
}

int Match::getFooballerStatsGoalkeeper(const SFootballer& footballer)
{
    return footballer.data[3] + getFooballerStats(footballer);
}

int Match::getFooballerStatsDefence(const SFootballer& footballer)
{
    return footballer.data[4] + getFooballerStats(footballer);
}

int Match::getFooballerStatsMiddlefield(const SFootballer& footballer)
{
    return footballer.data[5] + getFooballerStats(footballer);
}

int Match::getFooballerStatsAttack(const SFootballer& footballer)
{
    return footballer.data[6] + getFooballerStats(footballer);
}

SFormationsSum Match::getFormationsSum(const SClub &clubRef, bool isPlayerTeam)
{
    SFormationsSum formationSum;

    vector<SFootballer> &tmpFootballers = isPlayerTeam ? pFootballers->getPlayersTeam() : pFootballers->getRivals();
    int clubId      = isPlayerTeam ? clubRef.clubId : clubRef.rivalData[0];
    int clubTactic  = isPlayerTeam ? clubRef.teamSetting : clubRef.rivalData[2];

    for (size_t index = 0; index < tmpFootballers.size(); index++) {
        SFootballer &footballer = tmpFootballers[index];

        if (clubId == footballer.data[22] && footballer.data[12] < 2) {
            if (footballer.data[0] < 12) {
                formationSum.goalBonus += footballer.finances[2];
            }

            if (footballer.data[0] == 1) {
                formationSum.sumGol = getFooballerStatsGoalkeeper(footballer);
            }

            if (footballer.data[0] == 2 ||
                footballer.data[0] == 3 ||
                footballer.data[0] == 4
            ) {
                formationSum.sumDef += getFooballerStatsDefence(footballer);
            }

            if (footballer.data[0] == 5) {
                if (clubTactic == T4_4_2 || clubTactic == T4_4_2_DEF || clubTactic == T4_4_2_ATT ||
                    clubTactic == T4_4_2_DIA || clubTactic == T4_2_4 || clubTactic == T4_3_3 ||
                    clubTactic == T4_5_1 || clubTactic == T5_3_2 || clubTactic == T5_3_2_DEF || clubTactic == T5_3_2_ATT
                ) {
                    formationSum.sumDef += getFooballerStatsDefence(footballer);
                }
                else {
                    formationSum.sumMid += getFooballerStatsMiddlefield(footballer);
                }
            }

            if (footballer.data[0] == 6) {
                if (clubTactic == T5_3_2 || clubTactic == T5_3_2_DEF || clubTactic == T5_3_2_ATT) {
                    formationSum.sumDef += getFooballerStatsDefence(footballer);
                }
                else {
                    formationSum.sumMid += getFooballerStatsMiddlefield(footballer);
                }
            }

            if (footballer.data[0] == 7) {
                formationSum.sumMid += getFooballerStatsMiddlefield(footballer);
            }

            if (footballer.data[0] == 8) {
                if (clubTactic == T4_2_4) {
                    formationSum.sumAtt += getFooballerStatsAttack(footballer);
                }
                else {
                    formationSum.sumMid += getFooballerStatsMiddlefield(footballer);
                }
            }

            if (footballer.data[0] == 9) {
                if (clubTactic == T3_4_3 || clubTactic == T4_3_3 || clubTactic == T4_2_4) {
                    formationSum.sumAtt += getFooballerStatsAttack(footballer);
                }
                else {
                    formationSum.sumMid += getFooballerStatsMiddlefield(footballer);
                }
            }

            if (footballer.data[0] == 10) {
                if (clubTactic == T4_5_1) {
                    formationSum.sumMid += getFooballerStatsMiddlefield(footballer);
                }
                else {
                    formationSum.sumAtt += getFooballerStatsAttack(footballer);
                }
            }

            if (footballer.data[0] == 11) {
                formationSum.sumAtt += getFooballerStatsAttack(footballer);
            }
        }
    }

    return formationSum;
}

SFormationsSum Match::getPlayerFormationsSum(const SClub &clubRef)
{
    SFormationsSum playerFormationsSum = getFormationsSum(clubRef, true);

    playerFormationsSum.sumAtt += playerFormationsSum.goalBonus / 1000;
    playerFormationsSum.sumMid += playerFormationsSum.goalBonus / 1000;

    switch (clubRef.inst[1]) {
        case INSTR_TREATMENT_HARD: { // obchodzenie sie twarde
            playerFormationsSum.sumDef += 5;
            playerFormationsSum.sumMid += 5;
            playerFormationsSum.sumAtt += 5;
            break;
        }
        case INSTR_TREATMENT_SOFT: { // obchodzenie sie delikatne
            playerFormationsSum.sumDef -= 5;
            playerFormationsSum.sumMid -= 5;
            playerFormationsSum.sumAtt -= 5;
            break;
        }
    }

    switch (clubRef.inst[5]) {
        case INSTR_ATTIT_DEFENSIVE: { //nastawienie obronne
            playerFormationsSum.sumDef += 10;
            playerFormationsSum.sumAtt -= 10;
            break;
        }
        case INSTR_ATTIT_ATTACK: { //nastawienie atak
            playerFormationsSum.sumDef -= 10;
            playerFormationsSum.sumAtt += 10;
            break;
        }
    }

    // add points from tactic training
    playerFormationsSum.sumDef += clubRef.trained[3];
    playerFormationsSum.sumMid += clubRef.trained[3];
    playerFormationsSum.sumAtt += clubRef.trained[3];

    // tactic
    switch (clubRef.teamSetting) {
        case T4_4_2_DEF:
        case T3_5_2_DEF:
        case T5_3_2_DEF: { //ustawienie obronne
            playerFormationsSum.sumDef += 10;
            playerFormationsSum.sumAtt -= 10;
            break;
        }
        case T4_4_2_ATT:
        case T3_5_2_ATT:
        case T5_3_2_ATT: { //ustawienie atak
            playerFormationsSum.sumDef -= 10;
            playerFormationsSum.sumAtt += 10;
            break;
        }
        case T4_4_2_DIA: { //ustawienie diamond
            playerFormationsSum.sumDef += 10;
            playerFormationsSum.sumAtt += 10;
            playerFormationsSum.sumMid -= 15;
            break;
        }
    }
    playerFormationsSum.sumAtt += clubRef.trained[2] * 2; // stale fragmenty, ale tylko dla gracza

    return playerFormationsSum;
}

SFormationsSum Match::getRivalFormationsSum(const SClub &clubRef)
{
    SFormationsSum rivalFormationsSum = getFormationsSum(clubRef, false);

    rivalFormationsSum.sumAtt += rivalFormationsSum.goalBonus / 1000;
    rivalFormationsSum.sumMid += rivalFormationsSum.goalBonus / 1000;

    if (clubRef.rivalData[1] == 1) { // rywal gra u siebie
        rivalFormationsSum.sumGol += 5;
        rivalFormationsSum.sumDef += 10;
        rivalFormationsSum.sumMid += 10;
        rivalFormationsSum.sumAtt += 10;
    }

    switch (clubRef.rivalInst[1]) {
        case INSTR_TREATMENT_HARD: {
            rivalFormationsSum.sumDef += 5;
            rivalFormationsSum.sumMid += 5;
            rivalFormationsSum.sumAtt += 5;
            break;
        }
        case INSTR_TREATMENT_SOFT: {
            rivalFormationsSum.sumDef -= 5;
            rivalFormationsSum.sumMid -= 5;
            rivalFormationsSum.sumAtt -= 5;
            break;
        }
    }

    switch (clubRef.rivalInst[5]) {
        case INSTR_ATTIT_DEFENSIVE: { // nastawienie obronne
            rivalFormationsSum.sumDef += 10;
            rivalFormationsSum.sumAtt -= 10;
            break;
        }
        case INSTR_ATTIT_ATTACK: { // nastawienie atak
            rivalFormationsSum.sumDef -= 10;
            rivalFormationsSum.sumAtt += 10;
            break;
        }
    }

    switch (clubRef.rivalData[2]) {
        case T4_4_2_DEF:
        case T3_5_2_DEF:
        case T5_3_2_DEF: { // ustawienie obronne
            rivalFormationsSum.sumDef += 10;
            rivalFormationsSum.sumAtt -= 10;
            break;
        }
        case T4_4_2_ATT:
        case T3_5_2_ATT:
        case T5_3_2_ATT: { // ustawienie atak
            rivalFormationsSum.sumDef -= 10;
            rivalFormationsSum.sumAtt += 10;
            break;
        }
    }

    return rivalFormationsSum;
}

int Match::getEmptyMsgSlot()
{
    int index = 0;
    for (int i = MAX_MESSAGES; i > 0; i--) {
        if (msg[i] == 0) {
            index = i;
        }
    }
    return index;
}
