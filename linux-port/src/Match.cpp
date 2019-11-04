
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
#include "TeamComposition.h"

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
    const TeamInstructions *pTeamInstruction
) {
    this->pClub = pClub;
    this->pColors = pColors;
    this->pInput = pInput;
    this->pFootballers = pFootballers;
    this->pTable = pTable;
    this->pRounds = pRounds;
    this->pTeamInstruction = pTeamInstruction;

    pTactic = new Tactic(pColors);
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

    FILE *f = fopen(FILE_MSG_MATCH, "rb");
    if (!f) {
        char message[512];
        snprintf(message, 512, ERROR_OPEN_FILE, FILE_MSG_MATCH);
        throw std::invalid_argument(message);
    }

    wchar_t buffer[MAX_NEWS_LENGTH];
    while (fgetws(buffer, MAX_NEWS_LENGTH, f) != NULL) {
        wstring line = buffer;
        if (line.size() > 1) {
            line[line.size() - 1] = L'\0'; // remove \n on end of line
            std::size_t colonPos = line.find(L":", 0);
            SNews news;
            news.num = std::stoi(line.substr(0, colonPos));
            news.message = line.substr(colonPos + 1);
            matchMsgs.push_back(news);
        }
    }

    fclose(f);
}

bool Match::runMecz() {
    SClub &clubRef = pClub->get();

    if (!clubRef.isMatch || clubRef.isNotAllowedTeamPlayer) {
        pInput->clrscr();
        pColors->textcolor(LIGHTRED);
        wcout << endl << L"Teraz nie można rozegrać meczu!";
        return false;
    }

    loadMatchMessages();

    wstring speed[5] = {
        L"B.szybko",
        L"Szybko",
        L"Średnio",
        L"Wolno",
        L"B.wolno"
    };

    wstring dlagol1[10], dlagol2[10];
    int mingol1[10], mingol2[10], minuta2 = 0, juzzmienil[3] = {0, 0, 0};
    bool isOpenTacticsMenu = false;
    bool isPossibleGoToTactics = true;
    int koniec = 0; // 0 - poczatek meczu, 1 - koniec 1 polowy, 2 - poczatek 2 polowy, 3 - koniec meczu, 4 - wyjscie z meczu
    int playerGoals = 0, rivalGoals = 0; // playerGoals - gole gracza, rivalGoals - gole przeciwnika
    bool isPlayerBall = false; // true - player team has ball, false - rival team has a ball
    int ktoZacz = 0, minuta = 0, start = 0, k, czas = 2;
    SFormationsSum playerFormationsSum;
    SFormationsSum rivalFormationsSum;

    // where is ball/action
    int whereIsAction = ACTION_IN_MIDDLEFIELD;

    int los = 0, co = 0, x1 = 0, x2 = 0, x3 = 0, pos1 = 0, pos2 = 0, pos = 0;

    // remembers the number of the player who needs to be moved to the next message,
    // eg. "%s shoots!", footballerMemory = %s, "%s has scored!"
    int footballerMemory;

    int str[8] = {0, 0, 0, 0, 0, 0, 0, 0}, str2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    bool isShotFromDistance = false;
    bool endHalfMatch = false;
    float strefa[6] = {0, 0, 0, 0, 0, 0};

    howManyPlayerChanges = 0;
    isPlayerChanges = true;
    whoPlayerChanges = 0;

    //time_t tmptime = time(NULL);
    //struct tm t = *localtime(&tmptime);

    int walkower = pFootballers->getSizePlayerTeam();

    memset(mingol1, 0, 10 * sizeof(int));
    memset(mingol2, 0, 10 * sizeof(int));

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

        bool isHome = clubRef.rivalData[1] == 0;

        pInput->clrscr();
        pColors->textcolor(WHITE);
        if (isHome) { //w domu
            pColors->textbackground(BLUE);
            wprintf(L" %19ls %d ", pClub->getClubName(clubRef.clubId - 1).c_str(), playerGoals);
            pColors->textbackground(RED);
            wprintf(L" %d %-19ls ", rivalGoals, pClub->getClubName(clubRef.rivalData[0] - 1).c_str());
        }
        else { //wyjazd
            pColors->textbackground(RED);
            wprintf(L" %19ls %d ", pClub->getClubName(clubRef.rivalData[0] - 1).c_str(), rivalGoals);
            pColors->textbackground(BLUE);
            wprintf(L" %d %-19ls ", playerGoals, pClub->getClubName(clubRef.clubId - 1).c_str());
        }
        pColors->textbackground(BLACK);
        wprintf(L" Czas:%3d", minuta);
        pColors->textcolor(LIGHTGRAY);

        drawWhoScored(
            isHome ? mingol1 : mingol2,
            isHome ? mingol2 : mingol1,
            isHome ? dlagol1 : dlagol2,
            isHome ? dlagol2 : dlagol1
        );

        playerFormationsSum = getPlayerFormationsSum(clubRef);
        rivalFormationsSum = getRivalFormationsSum(clubRef);

        OnaA = playerFormationsSum.sumDef - rivalFormationsSum.sumAtt;
        PnaP = playerFormationsSum.sumMid - rivalFormationsSum.sumMid;
        AnaO = playerFormationsSum.sumAtt - rivalFormationsSum.sumDef;
        if (isShotFromDistance) {
            // increase the chance of defending a shot from a distance
            playerFormationsSum.sumGol += 50;
            rivalFormationsSum.sumGol += 50;
            isShotFromDistance = false;
        }

        if (isHome) { // gracz gra w domu
            drawBoard(
                true,
                str[0], str2[0],
                str[1], str2[1],
                str[2], str2[2],
                strefa[3], strefa[4], strefa[5],
                str[3], str2[3],
                str[7], str2[7],
                str[6], str2[6],
                str[4], str2[4],
                str[5], str2[5],
                pos1, pos2,
                OnaA, PnaP, AnaO,
                whereIsAction,
                isPlayerBall
            );
        }
        else { // gracz gra na wyjezdzie
            drawBoard(
                false,
                str2[0], str[0],
                str2[1], str[1],
                str2[2], str[2],
                strefa[5], strefa[4], strefa[3],
                str2[3], str[3],
                str2[7], str[7],
                str2[6], str[6],
                str2[4], str[4],
                str2[5], str[5],
                pos2, pos1,
                OnaA, PnaP, AnaO,
                whereIsAction,
                isPlayerBall
            );
        }

        if (start == 1 && !isOpenTacticsMenu && koniec != 4 && koniec != 1 && clubRef.isMatchAutoMsg) {
            pColors->textcolor(GREEN);
            wcout << endl;
            wprintf(
                L"A Taktyka-%ls  P Taktyka-%ls  C Szybkość-%ls\n\r",
                pClub->getClubName(clubRef.clubId - 1).c_str(),
                pClub->getClubName(clubRef.rivalData[0] - 1).c_str(),
                speed[czas].c_str()
            );
        }
        if (start == 1 && koniec < 3) { //mecz się rozpoczął
            co = whatHappened(
                isPlayerBall,
                PnaP,
                OnaA,
                AnaO,
                playerFormationsSum.sumGol,
                rivalFormationsSum.sumGol,
                whereIsAction,
                clubRef
            );
            //************ whereIsAction = ACTION_IN_MIDDLEFIELD *******************
            if (co == 9) {
                isShotFromDistance = true;
            }

            if (whereIsAction == ACTION_IN_MIDDLEFIELD) {
                los = (rand() % 3) + 2;
                msgFootballers[0] = getFootballerSurname(isPlayerBall, los - 1);
                msgFootballers[1] = getFootballerSurname(isPlayerBall, 6); //czyli 7

                if (co == 1) {
                    wiado[0] = 8;
                    los = (rand() % 3); //blokada=1;
                    msgWhoBall[0] = isPlayerBall;
                    msgWhoBall[1] = isPlayerBall;
                    int teamSetting = isPlayerBall ? clubRef.teamSetting : clubRef.rivalData[2];
                    msgFootballers[2] = getFootballerSurname(isPlayerBall, 6);

                    if (los == 0) { //na lewe skrzydło
                        wiado[1] = 9;
                        whereIsAction = ACTION_IN_WINGER;
                        footballerMemory = getLefttWingerFootballerId(teamSetting);
                        msgFootballers[3] = getFootballerSurname(isPlayerBall, footballerMemory);
                    }
                    else if (los == 1) { // podanie na prawe skrzydło
                        wiado[1] = 10;
                        whereIsAction = ACTION_IN_WINGER;
                        footballerMemory = getRightWingerFootballerId(teamSetting);
                        msgFootballers[3] = getFootballerSurname(isPlayerBall, footballerMemory);
                    }
                    else if (los == 2) {
                        wiado[1] = 11; // %ls podaje do przodu.
                        whereIsAction = ACTION_IN_PANELTY_AREA;
                        msgFootballers[2] = getFootballerSurname(isPlayerBall, 6);
                    }
                }//co=1
                else if (co == 2) {
                    wiado[0] = 12;
                    whereIsAction = ACTION_IN_PANELTY_AREA;
                    msgWhoBall[0] = isPlayerBall;
                }//co=2
                else if (co == 3 || co == 4) {
                    wiado[0] = 12;
                    msgWhoBall[0] = isPlayerBall;
                    if (co == 4) {
                        wiado[0] = 8;
                    }
                    wiado[1] = 13;
                    if (co == 4) {
                        wiado[1] = 14;
                    }

                    // zamiana pilki
                    isPlayerBall = !isPlayerBall;
                    msgWhoBall[1] = isPlayerBall;
                    msgFootballers[2] = getFootballerSurname(isPlayerBall, 6);
                    msgFootballers[3] = pClub->getClubName((isPlayerBall ? clubRef.clubId : clubRef.rivalData[0]) - 1);

                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }//co=3||co=4
                else if (co == 5) {
                    wiado[0] = 8;
                    msgWhoBall[0] = isPlayerBall;
                    wiado[1] = 15;
                    msgWhoBall[1] = isPlayerBall;
                    wiado[2] = 16;

                    // zamiana pilki
                    isPlayerBall = !isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    msgFootballers[4] = getFootballerSurname(isPlayerBall, 6);

                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }//co=5
                else if (co == 6) {
                    wiado[0] = 8;
                    msgWhoBall[0] = isPlayerBall;
                    wiado[1] = 17;
                    wiado[2] = 18;

                    // zamiana pilki
                    isPlayerBall = !isPlayerBall;
                    msgWhoBall[1] = isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    msgFootballers[2] = getFootballerSurname(isPlayerBall, 6);
                    msgFootballers[3] = msgFootballers[1];
                    msgFootballers[4] = getFootballerSurname(isPlayerBall, 6);

                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }//co=6
                else if (co == 7) { //faul przeciwnika
                    wiado[0] = 8;
                    msgWhoBall[0] = isPlayerBall;
                    isPossibleGoToTactics = true;
                    wiado[1] = 14;
                    wiado[2] = 19;
                    int instrTreatment = isPlayerBall ? clubRef.rivalInst[1] : clubRef.inst[1];
                    int teamSetting = isPlayerBall ? clubRef.rivalData[2] : clubRef.teamSetting;
                    isPlayerBall ? str2[7]++ : str[7]++;

                    co = getArbiterDecision(instrTreatment);

                    // losuje pomocnika, ktory otrzyma zolta kartke
                    los = getMiddlefieldFootballerIdWhoFouled(teamSetting);

                    // wez na odrot niz pilka wskazuje, ten kto nie ma pilki
                    msgFootballers[2] = getFootballerSurname(!isPlayerBall, los - 1);
                    msgWhoBall[1] = !isPlayerBall;
                    msgWhoBall[2] = !isPlayerBall;

                    if (co == 3) { //jest żółta
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
                            wiado[3] = 60; // %ls otrzymuje drugą żółtą kartkę i w konsekwencji czerwoną!
                            if (isPlayerBall) {
                                str2[5]++;
                                str2[4]++;
                            }
                            else {
                                str[5]++;
                                str[4]++;
                            }
                        }
                        else {
                            wiado[3] = 20; // %ls otrzymuje żółtą kartkę!
                            if (isPlayerBall) {
                                str2[4]++;
                            }
                            else {
                                str[4]++;
                            }
                        }

                        msgFootballers[6] = getFootballerSurname(!isPlayerBall, los - 1);
                        msgWhoBall[3] = !isPlayerBall;
                    }// jest żółta end
                    else if (co == 2) { // upomnienie slowne
                        wiado[3] = 21;
                        msgFootballers[6] = getFootballerSurname(!isPlayerBall, los - 1);
                        msgWhoBall[3] = !isPlayerBall;
                    }

                    wiado[4] = 22; // %ls wznawia grę.
                    msgWhoBall[4] = isPlayerBall;
                    msgWhoBall[5] = isPlayerBall;

                    teamSetting = isPlayerBall ? clubRef.teamSetting : clubRef.rivalData[2];
                    msgFootballers[8] = getFootballerSurname(isPlayerBall, 6);
                    msgFootballers[10] = getFootballerSurname(isPlayerBall, 6);

                    los = (rand() % 3);
                    if (los == 0) { //na lewe skrzydło
                        wiado[5] = 9; // %ls podaje na lewe skrzydło do %ls.
                        whereIsAction = ACTION_IN_WINGER;
                        footballerMemory = getLefttWingerFootballerId(teamSetting);
                        msgFootballers[11] = getFootballerSurname(isPlayerBall, footballerMemory);
                    }
                    else if (los == 1) { //podanie na prawe skrzydło
                        wiado[5] = 10; // %ls podaje na prawe skrzydło do %ls.
                        whereIsAction = ACTION_IN_WINGER;
                        footballerMemory = getRightWingerFootballerId(teamSetting);
                        msgFootballers[11] = getFootballerSurname(isPlayerBall, footballerMemory);
                    }
                    else if (los == 2) {
                        wiado[5] = 11; // %ls podaje do przodu.
                        whereIsAction = ACTION_IN_PANELTY_AREA;
                    }
                }//co=7
                else if (co == 8) { //aut dla przeciwnika
                    wiado[0] = 8;
                    msgWhoBall[0] = isPlayerBall;
                    isPossibleGoToTactics = true;
                    wiado[1] = 23;
                    msgWhoBall[1] = isPlayerBall;
                    wiado[2] = (rand() % 2) == 0 ? 24 : 25;

                    los = (rand() % 3) + 2;

                    // zmiana pilki
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[4] = getFootballerSurname(isPlayerBall, los - 1);
                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                    msgWhoBall[2] = isPlayerBall;
                }//co=8
                else if (co == 9) { //strzał z dystansu
                    wiado[0] = 8;
                    msgWhoBall[0] = isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    wiado[1] = 26;
                    msgWhoBall[1] = isPlayerBall;
                    wiado[2] = (rand() % 2) == 0 ? 27 : 28;

                    isPlayerBall ? str[0]++ : str2[0]++;

                    int teamSetting = isPlayerBall ? clubRef.teamSetting : clubRef.rivalData[2];
                    los = getFootballerIdWhoShootDistance(teamSetting);

                    msgFootballers[1] = getFootballerSurname(isPlayerBall, los - 1);
                    msgFootballers[2] = msgFootballers[1];
                    msgFootballers[4] = msgFootballers[2];
                    whereIsAction = ACTION_IN_GOAL_SITUATION;
                    footballerMemory = los - 1;
                }//co=9
            } // dla blokada==1 whereIsAction == ACTION_IN_MIDDLEFIELD
            else if (whereIsAction == ACTION_IN_WINGER) {
                //************** Skrzydłowy przy piłce ******************
                wiado[0] = 29; // %ls biegnie wzdłuż bocznej linii boiska.
                msgWhoBall[0] = isPlayerBall;
                msgFootballers[0] = getFootballerSurname(isPlayerBall, footballerMemory);

                if (co == 10) { //udane dośrodkowanie
                    wiado[1] = 30; //  %ls mija %ls i biegnie dalej...
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

                    wiado[2] = (rand() % 2) == 0 ? 31 : 32;

                    msgWhoBall[2] = isPlayerBall;
                    whereIsAction = ACTION_IN_PANELTY_AREA;
                }
                else if (co == 11) { //faul
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

                    wiado[1] = 30; //  %ls mija %ls i biegnie dalej...
                    msgWhoBall[1] = isPlayerBall;
                    msgFootballers[2] = msgFootballers[0];
                    isPossibleGoToTactics = true;

                    int teamSetting = isPlayerBall ? clubRef.rivalData[2] : clubRef.teamSetting;
                    isPlayerBall ? str2[7]++ : str[7]++;

                    los = getDefFootballerId(teamSetting);

                    msgFootballers[3] = getFootballerSurname(!isPlayerBall, los - 1);

                    wiado[2] = 33;
                    msgFootballers[4] = msgFootballers[3];
                    msgFootballers[5] = msgFootballers[2];
                    msgWhoBall[2] = !isPlayerBall;
                    msgWhoBall[3] = !isPlayerBall;
                    int instrTreatment = isPlayerBall ? clubRef.rivalInst[1] : clubRef.inst[1];
                    co = getArbiterDecision(instrTreatment);

                    if (co == 3) { //jest żółta
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
                            wiado[3] = 60; // %ls otrzymuje drugą żółtą kartkę i w konsekwencji czerwoną!
                            if (isPlayerBall) {
                                str2[5]++;
                                str2[4]++;
                            }
                            else {
                                str[5]++;
                                str[4]++;
                            }
                        }
                        else {
                            wiado[3] = 20; // %ls otrzymuje żółtą kartkę!
                            if (isPlayerBall) {
                                str2[4]++;
                            }
                            else {
                                str[4]++;
                            }
                        }
                    }// jest żółta end
                    else if (co == 2) { //słownie
                        wiado[3] = 21;
                        msgFootballers[6] = msgFootballers[3];
                        msgWhoBall[3] = !isPlayerBall;
                    }//słownie end

                    wiado[4] = 85; // %ls będzie uderzał z wolnego.
                    msgWhoBall[4] = isPlayerBall;

                    footballerMemory = ((rand() % 10) + 2) - 1;
                    msgFootballers[8] = getFootballerSurname(isPlayerBall, footballerMemory);
                    isPlayerBall ? str[0]++ : str2[0]++;

                    wiado[5] = 63; // Sędzia jeszcze ustawia mur.
                    msgWhoBall[5] = isPlayerBall;

                    wiado[6] = 64; // %ls uderza z wolnego...
                    msgWhoBall[6] = isPlayerBall;
                    msgFootballers[12] = msgFootballers[8];
                    whereIsAction = ACTION_IN_DIRECT_FREE_KICK;
                }//end faul
                else if (co == 12) { //aut
                    wiado[1] = 34;
                    msgWhoBall[1] = isPlayerBall;
                    isPossibleGoToTactics = true;
                    wiado[2] = (rand() % 2) == 0 ? 24 : 25;

                    los = (rand() % 3) + 2;
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[4] = getFootballerSurname(isPlayerBall, los - 1);
                    msgWhoBall[2] = isPlayerBall;
                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }
                else if (co == 13) { //nie dośrodkował
                    wiado[1] = 35;
                    los = (rand() % 3) + 2;
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
                    los = (rand() % 6);
                    switch (instrPasses) {
                        case INSTR_PASSES_LONG: { //długie podania
                            if (los == 1 || los == 2 || los == 3 || los == 4 || los == 5) {
                                // %ls wykonuje długie podanie do przodu.
                                wiado[2] = 36; //whereIsAction = ACTION_IN_PANELTY_AREA;
                            }
                            else {
                                // %ls podaje do przodu.
                                wiado[2] = 11; //whereIsAction = ACTION_IN_MIDDLEFIELD;
                            }
                            break;
                        }
                        case INSTR_PASSES_MIXES:
                        case INSTR_PASSES_MIDDLE: { //mieszane, średnie
                            if (los == 1 || los == 2 || los == 5) {
                                wiado[2] = 36; //whereIsAction = ACTION_IN_PANELTY_AREA;
                            }
                            else {
                                wiado[2] = 11; //whereIsAction = ACTION_IN_MIDDLEFIELD;
                            }
                            break;
                        }
                        case INSTR_PASSES_SHORT: //któtkie podania
                        default: {
                            if (los == 1 || los == 2 || los == 3 || los == 4 || los == 5) {
                                wiado[2] = 11; //whereIsAction = ACTION_IN_MIDDLEFIELD;
                            }
                            else {
                                wiado[2] = 36; //whereIsAction = ACTION_IN_PANELTY_AREA;
                            }
                            break;
                        }
                    }

                    whereIsAction = wiado[2] == 11 ? ACTION_IN_MIDDLEFIELD : ACTION_IN_PANELTY_AREA;
                }//nie dośrodkował
            }//dla skrzydłowy przy piłce
            else if (whereIsAction == ACTION_IN_PANELTY_AREA) { //****************** Pole karne ********
                if (co == 14) { //spalony
                    wiado[0] = 37;
                    msgWhoBall[0] = isPlayerBall;
                    isPossibleGoToTactics = true;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, 10);

                    los = (rand() % 3);
                    if (los == 0) {
                        wiado[1] = 38; // %ls na spalonym!
                    }
                    else if (los == 1) {
                        wiado[1] = 39; // %ls ruszył za wcześnie. Spalony.
                    }
                    else {
                        wiado[1] = 40; // Chorągiewka w górze! %ls na spalonym.
                    }

                    msgFootballers[2] = msgFootballers[0];
                    msgWhoBall[1] = isPlayerBall;
                    wiado[2] = 41;
                    isPlayerBall ? str[3]++ : str2[3]++;
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[4] = getFootballerSurname(isPlayerBall, 0);
                    msgWhoBall[2] = isPlayerBall;
                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }//spalony
                else if (co == 15) { //spalony nieudany
                    wiado[0] = 37;
                    msgWhoBall[0] = isPlayerBall;
                    wiado[1] = 42;
                    msgWhoBall[1] = isPlayerBall;
                    wiado[2] = 43;
                    msgWhoBall[2] = isPlayerBall;

                    int teamSetting = isPlayerBall ? clubRef.teamSetting : clubRef.rivalData[2];
                    switch (teamSetting) {
                        case T4_5_1: {
                            los = 11;
                            break;
                        }
                        case T3_4_3:
                        case T4_3_3: {
                            los = (rand() % 3) + 9;
                            break;
                        }
                        default: {
                            los = (rand() % 2) + 10;
                            break;
                        }
                    }

                    footballerMemory = los - 1;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, footballerMemory);
                    isPlayerBall ? str[0]++ : str2[0]++;

                    msgFootballers[2] = msgFootballers[0];
                    msgFootballers[4] = msgFootballers[0];
                    whereIsAction = ACTION_IN_PANELTY_OR_1ON1;
                }//sam na sam
                else if (co == 16) { //obrona wykopuje
                    int instrPasses = isPlayerBall ? clubRef.rivalInst[0] : clubRef.inst[0];

                    los = (rand() % 6);
                    switch (instrPasses) {
                        case INSTR_PASSES_LONG: { //długie
                            if (los == 0) {
                                whereIsAction = ACTION_IN_MIDDLEFIELD;
                                wiado[0] = (rand() % 2) == 0 ? 44 : 45;
                            }
                            else {
                                whereIsAction = ACTION_IN_PANELTY_AREA;
                                wiado[0] = (rand() % 2) == 0 ? 46 : 47;
                            }
                            break;
                        }
                        case INSTR_PASSES_SHORT: { //krótkie
                            if (los == 0) {
                                whereIsAction = ACTION_IN_PANELTY_AREA;
                                wiado[0] = (rand() % 2) == 0 ? 46 : 47;
                            }
                            else {
                                whereIsAction = ACTION_IN_MIDDLEFIELD;
                                wiado[0] = (rand() % 2) == 0 ? 44 : 45;
                            }
                            break;
                        }
                        default: { //pozostałe
                            if (los == 0 || los == 1 || los == 2) {
                                whereIsAction = ACTION_IN_PANELTY_AREA;
                                wiado[0] = (rand() % 2) == 0 ? 46 : 47;
                            }
                            else {
                                whereIsAction = ACTION_IN_MIDDLEFIELD;
                                wiado[0] = (rand() % 2) == 0 ? 44 : 45;
                            }
                            break;
                        }
                    }
                    los = (rand() % 3) + 2;
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, los - 1);
                    msgWhoBall[0] = isPlayerBall;
                }//obrona wykopuje
                else if (co == 17) { //B łapie
                    wiado[0] = 48;
                    wiado[1] = 49;
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, 0);
                    msgWhoBall[0] = isPlayerBall;
                    msgWhoBall[1] = isPlayerBall;
                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }//B łapie
                else if (co == 18) { //karny
                    wiado[0] = 37;
                    msgWhoBall[0] = isPlayerBall;
                    wiado[1] = 50;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, 10);
                    msgWhoBall[1] = !isPlayerBall;
                    int teamSetting = isPlayerBall ? clubRef.rivalData[2] : clubRef.teamSetting;
                    if (isPlayerBall) {
                        str2[7]++;
                        str[6]++;
                    }
                    else {
                        str[7]++;
                        str2[6]++;
                    }

                    los = getDefFootballerId(teamSetting);

                    msgFootballers[3] = msgFootballers[0];
                    msgFootballers[2] = getFootballerSurname(!isPlayerBall, los - 1);

                    wiado[2] = (rand() % 2) == 0 ? 51 : 52;

                    int instrTreatment = isPlayerBall ? clubRef.rivalInst[1] : clubRef.inst[1];
                    msgWhoBall[2] = !isPlayerBall;
                    msgWhoBall[3] = !isPlayerBall;
                    isPlayerBall ? str[0]++ : str2[0]++;

                    co = getArbiterDecision(instrTreatment);

                    if (co == 3) { //jest żółta
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
                            wiado[3] = 60; // %ls otrzymuje drugą żółtą kartkę i w konsekwencji czerwoną!
                            if (isPlayerBall) {
                                str2[5]++;
                                str2[4]++;
                            }
                            else {
                                str[5]++;
                                str[4]++;
                            }
                        }
                        else {
                            wiado[3] = 20; // %ls otrzymuje żółtą kartkę!
                            isPlayerBall ? str2[4]++ : str[4]++;
                        }
                    }// jest żółta end
                    else if (co == 2) { //słownie
                        wiado[3] = 21;
                        msgFootballers[6] = msgFootballers[2];
                        msgWhoBall[3] = !isPlayerBall;
                    }

                    wiado[4] = (rand() % 2) == 0 ? 53 : 54;
                    footballerMemory = ((rand() % 4) + 8) - 1;
                    msgFootballers[8] = getFootballerSurname(isPlayerBall, footballerMemory);
                    whereIsAction = ACTION_IN_PANELTY_OR_1ON1;
                    msgWhoBall[4] = isPlayerBall;
                }//karny
                else if (co == 19) { //symulowany
                    wiado[0] = 37; // %ls przyjmuje piłkę w polu karnym...
                    msgWhoBall[0] = isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, 10);
                    msgWhoBall[1] = !isPlayerBall;
                    msgWhoBall[3] = !isPlayerBall;
                    int teamSetting = isPlayerBall ? clubRef.rivalData[2] : clubRef.teamSetting;

                    los = getDefFootballerId(teamSetting);

                    msgFootballers[3] = msgFootballers[0];
                    msgFootballers[2] = getFootballerSurname(!isPlayerBall, los - 1);
                    wiado[1] = 50; // %ls wślizgiem atakuje %ls...
                    wiado[2] = 55; // %ls pada na murawę, ale sędzia nie dał się nabrać.
                    msgWhoBall[2] = isPlayerBall;
                    msgFootballers[4] = msgFootballers[0];
                    wiado[3] = ((rand() % 2) == 0) ? 44 : 45;
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[6] = msgFootballers[2];
                    msgWhoBall[3] = isPlayerBall;
                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }//faul symulowany
                else if (co == 20) { //róg
                    wiado[0] = 37;
                    msgWhoBall[0] = isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    msgWhoBall[3] = isPlayerBall;
                    isPossibleGoToTactics = true;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, 10);
                    msgWhoBall[1] = !isPlayerBall;
                    int teamSetting = isPlayerBall ? clubRef.rivalData[2] : clubRef.teamSetting;
                    isPlayerBall ? str[2]++ : str2[2]++;

                    los = getDefFootballerId(teamSetting);

                    msgFootballers[3] = msgFootballers[0];
                    msgFootballers[2] = getFootballerSurname(!isPlayerBall, los - 1);
                    wiado[1] = 50;
                    wiado[2] = 56;
                    wiado[3] = 78;
                    msgFootballers[6] = getFootballerSurname(isPlayerBall, 6);
                    whereIsAction = ACTION_IN_PANELTY_AREA;
                }//róg
                else if (co == 21) { //strzał
                    wiado[0] = 37;
                    msgWhoBall[0] = isPlayerBall;
                    msgWhoBall[1] = isPlayerBall;
                    los = (rand() % 2);
                    wiado[1] = (los == 0) ? 43 : 57;
                    int teamSetting = isPlayerBall ? clubRef.teamSetting : clubRef.rivalData[2];
                    if (teamSetting == T3_4_3 || teamSetting == T4_3_3) {
                        los = (rand() % 3) + 9;
                    }
                    else if (teamSetting == T4_5_1) {
                        los = 11;
                    }
                    else {
                        los = (rand() % 2) + 10;
                    }

                    msgFootballers[0] = getFootballerSurname(isPlayerBall, los - 1);
                    isPlayerBall ? str[0]++ : str2[0]++;
                    msgFootballers[2] = msgFootballers[0];
                    footballerMemory = los - 1;
                    whereIsAction = ACTION_IN_GOAL_SITUATION;
                }//strzał
                else if (co == 22) { //podanie i strzał
                    wiado[0] = 37;
                    msgWhoBall[0] = isPlayerBall;
                    wiado[1] = 58;
                    msgWhoBall[1] = isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    los = (rand() % 2);
                    wiado[2] = (los == 0) ? 59 : 57;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, 10);
                    los = (rand() % 4) + 7;
                    msgFootballers[2] = getFootballerSurname(isPlayerBall, los - 1);
                    isPlayerBall ? str[0]++ : str2[0]++;
                    msgFootballers[4] = msgFootballers[2];
                    footballerMemory = los - 1;
                    whereIsAction = ACTION_IN_GOAL_SITUATION;
                }//podanie i strzał
                else if (co == 23) { // faul napastnika
                    wiado[0] = 37;
                    msgWhoBall[0] = isPlayerBall;
                    isPossibleGoToTactics = true;
                    msgFootballers[2] = getFootballerSurname(!isPlayerBall, 1);
                    msgWhoBall[1] = !isPlayerBall;
                    msgWhoBall[2] = !isPlayerBall;
                    int instrTreatment = isPlayerBall ? clubRef.inst[1] : clubRef.rivalInst[1];
                    int teamSetting = isPlayerBall ? clubRef.teamSetting : clubRef.rivalData[2];

                    isPlayerBall ? str[7]++ : str2[7]++;

                    if (teamSetting == T3_4_3 || teamSetting == T4_3_3) {
                        los = (rand() % 3) + 9;
                    }
                    else if (teamSetting == T4_5_1) {
                        los = 11;
                    }
                    else {
                        los = (rand() % 2) + 10;
                    }

                    msgFootballers[0] = getFootballerSurname(isPlayerBall, los - 1);
                    msgFootballers[3] = msgFootballers[0];
                    wiado[1] = 50;
                    wiado[2] = 61;
                    wiado[3] = 62;
                    msgWhoBall[3] = isPlayerBall;
                    msgFootballers[6] = msgFootballers[0];
                    wiado[4] = 19;
                    msgWhoBall[4] = isPlayerBall;

                    co = getArbiterDecision(instrTreatment);

                    if (co == 3) { //jest żółta
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
                            wiado[5] = 60;
                            if (isPlayerBall) {
                                str[4]++;
                                str[5]++;
                            }
                            else {
                                str2[4]++;
                                str2[5]++;
                            }
                        }
                        else {
                            wiado[5] = 20;
                            if (isPlayerBall) {
                                str[4]++;
                            }
                            else {
                                str2[4]++;
                            }
                        }
                    }// jest żółta end
                    else if (co == 2) { //słownie
                        wiado[5] = 21;
                        msgFootballers[10] = msgFootballers[3];
                        msgWhoBall[5] = isPlayerBall;
                    }
                    wiado[6] = 41;
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[12] = getFootballerSurname(isPlayerBall, 0);
                    msgWhoBall[6] = isPlayerBall;
                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }//faul napastnika
            }//whereIsAction = ACTION_IN_PANELTY_AREA, blokada=1, pole karne
            //*********************** obron B ***********************************
            else if (whereIsAction == ACTION_IN_GOAL_SITUATION ||
                     whereIsAction == ACTION_IN_PANELTY_OR_1ON1 ||
                     whereIsAction == ACTION_IN_DIRECT_FREE_KICK
            ) { //obrona B
                if (co == 24) { // udana
                    wiado[0] = (rand() % 2) == 0 ? 65 : 66;
                    wiado[1] = 67; // I łapie piłkę! Dobra obrona.
                    wiado[2] = 49; // Bramkarz wykopuje piłkę.
                    isPlayerBall ? str[1]++ : str2[1]++;
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
                }//udana
                else if (co == 25) { //no problem
                    //68. Dobrze ustawiony %ls, bez trudu łapie piłkę.
                    //69. %ls spokojnie łapie piłkę.
                    wiado[0] = (rand() % 2 == 0) ? 68 : 69;
                    wiado[1] = 49; // Bramkarz wykopuje piłkę.

                    isPlayerBall ? str[1]++ : str2[1]++;
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
                }//no problem
                else if (co == 26) { //niepewnie
                    los = (rand() % 4);
                    if (los == 0) {
                        wiado[0] = 70; // %ls z trudem broni, ale nie zdołał złapać piłki...
                        msgWhoBall[0] = 1;
                        if (isPlayerBall) {
                            msgWhoBall[0] = 2;
                            str[1]++;
                        }
                        else {
                            str2[1]++;
                        }
                    }
                    else if (los == 1) {
                        wiado[0] = 71; // Piłka uderza w słupek!!
                        msgWhoBall[0] = isPlayerBall;
                    }
                    else if (los == 2) {
                        wiado[0] = 72; // Piłka uderza w poprzeczkę!!
                        msgWhoBall[0] = isPlayerBall;
                    }
                    else {
                        wiado[0] = 73; // Piłka uderza w któregoś z obrońców!
                        msgWhoBall[0] = 1;
                        if (isPlayerBall) {
                            msgWhoBall[0] = 2;
                        }
                    }
                    msgFootballers[0] = getFootballerSurname(!isPlayerBall, 0);
                    whereIsAction = ACTION_IN_PANELTY_AREA;
                }//niepewnie
                else if (co == 27) { //róg
                    isPossibleGoToTactics = true;
                    if (rand() % 2 == 0) {
                        wiado[0] = 70;
                        isPlayerBall ? str[1]++ : str2[1]++;
                    }
                    else {
                        wiado[0] = 73;
                    }

                    msgWhoBall[0] = !isPlayerBall;
                    msgFootballers[0] = getFootballerSurname(!isPlayerBall, 0);
                    msgFootballers[4] = getFootballerSurname(isPlayerBall, 6);
                    isPlayerBall ? str[2]++ : str2[2]++;
                    wiado[1] = 56;
                    msgWhoBall[1] = isPlayerBall;
                    wiado[2] = 78;
                    msgWhoBall[2] = isPlayerBall;
                    whereIsAction = ACTION_IN_PANELTY_AREA;
                }//róg
                else if (co == 28) { // GOOL
                    isPossibleGoToTactics = true;
                    wiado[0] = (rand() % 2 == 0) ? 65 : 66;
                    wiado[1] = 74;
                    msgWhoBall[1] = isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    wiado[2] = (rand() % 2 == 0) ? 75 : 76;
                    wiado[3] = 22;
                    los = (rand() % 11) + 1;

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
                        if (footballer.data[0] == los && clubId == footballer.data[22]) {
                            footballer.data[20]++; // losowy gracz dosatje formę
                        }
                    }

                    if (isPlayerBall) {
                        pFootballers->savePlayerTeam();
                    }
                    else {
                        pFootballers->saveRivals();
                    }

                    if (isPlayerBall) {
                        str[1]++;
                        playerGoals++;
                        dlagol1[playerGoals - 1] = msgPlayerSurnames[footballerMemory];
                        mingol1[playerGoals - 1] = getGoooalMinute(minuta, koniec);
                    }
                    else {
                        str2[1]++;
                        rivalGoals++;
                        dlagol2[rivalGoals - 1] = msgRivalSurnames[footballerMemory];
                        mingol2[rivalGoals - 1] = getGoooalMinute(minuta, koniec);
                    }

                    msgFootballers[0] = getFootballerSurname(!isPlayerBall, 0);
                    msgFootballers[2] = pClub->getClubName((isPlayerBall ? clubRef.clubId : clubRef.rivalData[0]) - 1);
                    msgFootballers[4] = getFootballerSurname(isPlayerBall, footballerMemory);
                    msgFootballers[6] = pClub->getClubName((isPlayerBall ? clubRef.rivalData[0] : clubRef.clubId) - 1);
                    isPlayerBall = !isPlayerBall;
                    msgWhoBall[3] = isPlayerBall;
                    msgWhoBall[0] = isPlayerBall;

                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                    los = (rand() % 11) + 1;

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

                        if (footballer.data[0] == los && clubId == footballer.data[22]) {
                            footballer.data[20]--; // losowy zawodnik dostaje tez minus forma
                        }
                    }

                    if (isPlayerBall) {
                        pFootballers->savePlayerTeam();
                    }
                    else {
                        pFootballers->saveRivals();
                    }
                }//gool
                else if (co == 29) { //nieuznany
                    isPossibleGoToTactics = true;
                    wiado[0] = (rand() % 2 == 0) ? 65 : 66;
                    wiado[1] = 74;
                    msgWhoBall[1] = isPlayerBall;
                    msgWhoBall[2] = isPlayerBall;
                    wiado[2] = (rand() % 2 == 0) ? 75 : 76;
                    wiado[3] = 77;
                    msgWhoBall[3] = isPlayerBall;
                    wiado[4] = 41;

                    isPlayerBall ? str[0]-- : str2[0]--;
                    msgFootballers[0] = getFootballerSurname(!isPlayerBall, 0);
                    msgFootballers[2] = pClub->getClubName((isPlayerBall ? clubRef.clubId : clubRef.rivalData[0]) - 1);
                    msgFootballers[4] = getFootballerSurname(isPlayerBall, footballerMemory);
                    msgFootballers[8] = msgFootballers[0];
                    isPlayerBall = !isPlayerBall;
                    msgWhoBall[4] = isPlayerBall;
                    msgWhoBall[0] = isPlayerBall;

                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }//gool
                else if (co == 30) { //niecelnie
                    los = (rand() % 4);
                    isPossibleGoToTactics = true;
                    if (los == 0) {
                        wiado[0] = 79; // %ls walnął nad poprzeczką!
                    }
                    else if (los == 1) {
                        wiado[0] = 80; // Ale strzelił fatalnie!
                    }
                    else if (los == 2) {
                        wiado[0] = 81; // %ls minimalnie chybił!
                    }
                    else {
                        wiado[0] = 82; // Poszło w trybuny! Fatalny strzał.
                    }

                    msgWhoBall[0] = isPlayerBall;
                    msgFootballers[0] = getFootballerSurname(isPlayerBall, footballerMemory);
                    isPlayerBall = !isPlayerBall;
                    msgFootballers[2] = getFootballerSurname(!isPlayerBall, 0);
                    wiado[1] = 41;
                    msgWhoBall[1] = isPlayerBall;
                    whereIsAction = ACTION_IN_MIDDLEFIELD;
                }//niecelnie
            }//obrona B whereIsAction = ACTION_IN_GOAL_SITUATION;

            pColors->textcolor(LIGHTGRAY);
            if (minuta == 0 && koniec == 0) { //początek meczu
                memset(wiado, 0, 10 * sizeof(int));
                wiado[0] = 1; //mecz się rozpoczął
                ktoZacz = (rand() % 2) + 1;
                if (ktoZacz == 1) { //zaczyna gracz
                    msgFootballers[0] = pClub->getClubName(clubRef.clubId - 1);
                    isPlayerBall = true;
                }
                else { //zaczyna przeciwnik
                    msgFootballers[0] = pClub->getClubName(clubRef.rivalData[0] - 1);
                    isPlayerBall = false;
                }
                whereIsAction = ACTION_IN_MIDDLEFIELD;
                msgWhoBall[0] = isPlayerBall;
            }//dla minuta=0 początek meczu
            else if (minuta == 45 && koniec == 2) { //początek 2 połowy
                if (ktoZacz == 1) { //zaczyna rywal
                    msgFootballers[0] = pClub->getClubName(clubRef.rivalData[0] - 1);
                    isPlayerBall = false;
                }
                else { // zaczyna gracz
                    msgFootballers[0] = pClub->getClubName(clubRef.clubId - 1);
                    isPlayerBall = true;
                }
                whereIsAction = ACTION_IN_MIDDLEFIELD;
                msgWhoBall[0] = isPlayerBall;
                memset(wiado, 0, 10 * sizeof(int));
                wiado[0] = 2;
            }//dla początek 2 połowy
            else if (koniec == 1) { //koniec 1 połowy
                isPossibleGoToTactics = true;
                minuta = 43;
                koniec = 2; // poczatek 2 polowy
                start = 0;
                whereIsAction = ACTION_IN_MIDDLEFIELD;
                memset(wiado, 0, 10 * sizeof(int));
                wiado[0] = 3; // "Koniec pierwszej połowy."
            }

            //************************ kontuzja *************************
            los = (rand() % 100); // losuj czy kontuzja 1/100
            if (los == 0) {
                x1 = (rand() % 2); // losuj ktory zespol 50/50
                los = (rand() % 11) + 1; // losuj ktory zawodnik

                vector<SFootballer> &tmpFootballers = (x1 == 0)
                    ? pFootballers->getPlayersTeam()
                    : pFootballers->getRivals();
                int clubId = (x1 == 0) ? clubRef.clubId : clubRef.rivalData[0];

                for (size_t index = 0; index < tmpFootballers.size(); index++) {
                    SFootballer &footballer = tmpFootballers[index];
                    if (footballer.data[0] == los && footballer.data[22] == clubId && footballer.data[12] < 2) {
                        footballer.data[11] = 0;
                        footballer.data[19] = 1;
                        for (int i = MAX_MESSAGES; i > 0; i--) {
                            if (wiado[i] == 0) {
                                k = i;
                            }
                        }
                        wiado[k] = (rand() % 2 == 0) ? 83 : 84; // %ls doznaje kontuzji! / %ls kontuzjowany!

                        if (x1 == 0) {
                            msgWhoBall[k] = 1;
                            msgFootballers[k * 2] = msgPlayerSurnames[los - 1];
                        }
                        else {
                            msgWhoBall[k] = 2;
                            msgFootballers[k * 2] = msgRivalSurnames[los - 1];
                        }

                        break;
                    }
                }

                if (x1 == 0) {
                    pFootballers->savePlayerTeam();
                }
                else {
                    pFootballers->saveRivals();
                }
            }
            //************************ kontuzja end *************************

            //******************** rywal - zamina zawonika kontuzjowanego ***********
            if (isPossibleGoToTactics) {
                x1 = 0;
                x3 = 0;
                for (size_t index = 0; index < pFootballers->getSizeRivals(); index++) {
                    SFootballer &footballer = pFootballers->getRival(index);
                    if (footballer.data[11] == 0 &&
                        clubRef.rivalData[0] == footballer.data[22] &&
                        footballer.data[0] < 12
                    ) {
                        x1 = footballer.data[0];
                        x2 = footballer.data[2]; // formacja na jakiej gra
                    }
                }

                for (int i = 0; i < 3; i++) {
                    if (juzzmienil[i] == 0) {
                        juzzmienil[i] = x1;
                        break;
                    }
                }

                for (int i = 0; i < 3; i++) {
                    if (juzzmienil[i] == x1) {
                        x3++;
                    }
                }

                if (x3 > 1) {
                    for (int i = 2; i >= 0; i--) {
                        if (juzzmienil[i] == x1) {
                            juzzmienil[i] = 0;
                            break;
                        }
                    }
                    x1 = 0;
                }

                if (x1 > 0) {
                    for (size_t index = 0; index < pFootballers->getSizeRivals(); index++) {
                        SFootballer &footballer = pFootballers->getRival(index);
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
            }//dla if isPossibleGoToTactics
            //************************************************
            if (whoPlayerChanges > 0) { //zmiana zawodnika**************
                for (int i = MAX_MESSAGES; i > 0; i--) {
                    if (wiado[i] == 0) {
                        k = i;
                    }
                }
                wiado[k] = 5; // %ls dokonuje zmiany...
                if (whoPlayerChanges == 1) { // zmiana gracza
                    msgWhoBall[k] = 1;
                    msgFootballers[k * 2] = pClub->getClubName(clubRef.clubId - 1);
                }
                else { // zmiana dla rywala
                    msgWhoBall[k] = 2;
                    msgFootballers[k * 2] = pClub->getClubName(clubRef.rivalData[0] - 1);
                    wiado[k + 1] = 7;
                    msgWhoBall[k + 1] = 2;
                    msgFootballers[(k + 1) * 2] = msgRivalSurnames[x3 - 1];
                    msgFootballers[(k + 1) * 2 + 1] = msgRivalSurnames[x1 - 1];
                }
                whoPlayerChanges = 0;
            }
            //*************** zadyma **********************
            x1 = (rand() % 100);
            if (x1 == 1 && whereIsAction == ACTION_IN_MIDDLEFIELD) {
                for (int i = MAX_MESSAGES; i > 0; i--) {
                    if (wiado[i] == 0) {
                        k = i;
                    }
                }
                wiado[k] = 86;
                wiado[k + 1] = 87;
                wiado[k + 2] = 88;
                wiado[k + 3] = 89;
                if (clubRef.rivalData[1] == 0) { // home
                    msgWhoBall[k] = 1;
                    msgWhoBall[k + 1] = 1;
                    msgWhoBall[k + 2] = 1;
                    msgWhoBall[k + 3] = 1;
                    clubRef.isRiot = 1;

                    pClub->save();
                }
                else {
                    msgWhoBall[k] = 2;
                    msgWhoBall[k + 1] = 2;
                    msgWhoBall[k + 2] = 2;
                    msgWhoBall[k + 3] = 2;
                }
            }
            //*********************************
            for (int i = 0; i < MAX_MESSAGES; i++) {
                k = i * 2;
                if (wiado[i] != 0) {
                    wcout << endl;
                }

                for (int index = 0; index < matchMsgs.size(); index++) {
                    const SNews &meczs = matchMsgs[index];
                    if (wiado[i] == meczs.num) {
                        int color = 0;
                        if (wiado[i] == 74) { // GOOOL dla %ls!!!
                            color = 143;
                        }
                        else if (wiado[i] == 20) { // %ls otrzymuje żółtą kartkę!
                            color = YELLOW;
                        }
                        else if (wiado[i] == 60) { //  %ls otrzymuje drugą żółtą kartkę i w konsekwencji czerwoną!
                            color = LIGHTRED;
                        }
                        else if (wiado[i] == 83 || wiado[i] == 84 ||
                                 wiado[i] == 86 || wiado[i] == 87 || wiado[i] == 88 || wiado[i] == 89
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

                        int bkgcolor = msgWhoBall[i] == 1 ? BLUE : RED;

                        pColors->textcolor(color);
                        pColors->textbackground(bkgcolor);

                        swprintf(
                            tmpMessage,
                            MAX_NEWS_LENGTH,
                            meczs.message.c_str(),
                            msgFootballers[k].c_str(),
                            msgFootballers[k + 1].c_str()
                        );

                        wcout << tmpMessage;

                        saveMatchMessageToFile(bkgcolor, minuta, tmpMessage);

                        break;
                    }
                }

                if (wiado[i] != 0) { //&&wiado[i+1]!=0)
                    /*if (clubRef.isMatchAutoMsg) {
                        //gettime(&t);
                        time_t tmptime = time(NULL);
                        t = *localtime(&tmptime);
                        k = t.tm_sec;
                        if (czas == 1) {
                            if (k == 59) k = 0;
                            else k += czas;
                        }
                        else if (czas == 2) {
                            if (k == 58) k = 0;
                            else if (k == 59) k = 1;
                            else k += czas;
                        }
                        else if (czas == 3) {
                            if (k == 57) k = 0;
                            else if (k == 58) k = 1;
                            else if (k == 59) k = 2;
                            else k += czas;
                        }
                        else if (czas == 4) {
                            if (k == 56) k = 0;
                            else if (k == 57) k = 1;
                            else if (k == 58) k = 2;
                            else if (k == 59) k = 3;
                            else k += czas;
                        }

                        while (t.tm_sec != k) {
                            time_t tmptime = time(NULL);
                            t = *localtime(&tmptime);

                            // UWAGA kbhit sprawdza czy nacisnieto klawisz, ale nie wstrzymuje programu
                            if (kbhit() && isPossibleGoToTactics) {
                                isOpenTacticsMenu = true; // wcisnieto klawisz, i mozna wejsc w taktykę, wtedy isOpenTacticsMenu = 1
                            }
                        }
                    }
                    else {*/
                    wchar_t key = pInput->getKeyBoardPressed();
                    if (clubRef.isMatchAutoMsg) {
                        if (isPossibleGoToTactics && (key == 'A' || key == 'P')) {
                            isOpenTacticsMenu = true; // user chce wejsc w taktyke swoja albo rywala
                        }
                    }
                    /*}*/
                }
            }
            pColors->textbackground(BLACK);
            memset(wiado, 0, MAX_MESSAGES * sizeof(int));
            minuta += 2;
            minuta2 += 2;
            //****************** posiadanie piłki i strefy **********************
            if (isPlayerBall) {
                pos++;
            }
            pos1 = (pos * 100) / (minuta2 / 2);
            pos2 = 100 - pos1;

            if (whereIsAction == ACTION_IN_MIDDLEFIELD) {
                strefa[1]++;
            }
            else {
                isPlayerBall ? strefa[2]++ : strefa[0]++;
            }
            strefa[3] = (strefa[0] * 100) / (minuta2 / 2);
            strefa[4] = (strefa[1] * 100) / (minuta2 / 2);
            strefa[5] = (strefa[2] * 100) / (minuta2 / 2);
            //***************** posiadanie i strefy end *********************
            //***************** kondycja ***************************
            k = (clubRef.inst[2] == 1) ? 2 : 3;

            for (size_t index = 0; index < pFootballers->getSizePlayerTeam(); index++) {
                SFootballer &footballer = pFootballers->getPlayerTeam(index);
                los = (rand() % k) + 1;
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
                los = (rand() % k) + 1;
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

        if (minuta > 45 && whereIsAction == ACTION_IN_MIDDLEFIELD && koniec == 0 && !endHalfMatch) {
            koniec = 1; // koniec 1 polowy
            minuta = 45;
            endHalfMatch = true;
            whereIsAction = ACTION_IN_MIDDLEFIELD;
        }//koniec1 połowy

        if (minuta > 90 && whereIsAction == ACTION_IN_MIDDLEFIELD && koniec == 2) {
            koniec = 3; // koniec meczu
        }

        if (koniec == 4) {
            wcout << endl << L"KONIEC MECZU";
        }
        pColors->textcolor(GREEN);
        wcout << endl;
        if (start == 0) {
            wcout << endl << L"S Start mecz" << endl;
        }

        if (clubRef.isMatchAutoMsg && (koniec == 0 || koniec == 2 || koniec == 1 || koniec == 4)) {
            wcout << endl;
            wprintf(
                L"A Taktyka-%ls  P Taktyka-%ls  C Szybkość-%ls\n\r",
                pClub->getClubName(clubRef.clubId - 1).c_str(),
                pClub->getClubName(clubRef.rivalData[0] - 1).c_str(),
                speed[czas].c_str()
            );
        }

        if (!clubRef.isMatchAutoMsg) {
            if (start && koniec < 4) {
                wcout << L"S Kontynuuj    ";
            }
            wprintf(
                L"A Taktyka - %ls    P Taktyka - %ls\n\r",
                pClub->getClubName(clubRef.clubId - 1).c_str(),
                pClub->getClubName(clubRef.rivalData[0] - 1).c_str()
            );
        }

        if (koniec == 4) {
            pColors->textcolor(RED);
            wcout << endl << L"Q Wyjście";
        }

        wchar_t menu = L'y';
        if (clubRef.isMatchAutoMsg && (start == 0 || isOpenTacticsMenu || koniec == 4)) {
            menu = pInput->getKeyBoardPressed();
        }
        else if (!clubRef.isMatchAutoMsg && (start == 0 || isPossibleGoToTactics || koniec == 1 || koniec == 4)) {
            menu = pInput->getKeyBoardPressed();
        }

        if (koniec == 3) {
            koniec = 4; // wyjscie z meczu
        }

        isPossibleGoToTactics = false;
        isOpenTacticsMenu = false;
        k = 0;

        if (walkower < 11) {
            endMenuMatch = L'Q';
            pColors->textcolor(LIGHTRED);
            wcout << endl << L"Przegrywasz walkowerem 0-3";
            playerGoals = 0;
            rivalGoals = 3;
        }

        switch (menu) {
            case 'C': { // szybkosc
                pInput->clrscr();
                pColors->textcolor(GREEN);
                wcout << endl <<
                    L"Podaj szybkość wyświetlania napisów:" << endl <<
                    L"0. Bardzo szybko" << endl <<
                    L"1. Szybko" << endl <<
                    L"2. Średnio" << endl <<
                    L"3. Wolno" << endl <<
                    L"4. Bardzo wolno" << endl;
                czas = pInput->getNumber();
                if (czas < 0 || czas > 4) {
                    czas = 2;
                }
                break;
            }
            case 'Q': { // wyjscie
                if (koniec == 4) {
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
                if (koniec < 3) {
                    start = 0;
                }

                playerTactics();
                break;
            }
            case 'P': { // taktyka przeciwnika
                if (koniec < 3) {
                    start = 0;
                }

                rivalTactics();
                break;
            }
        }
    }
    while (endMenuMatch != 'Q');

    pColors->textcolor(LIGHTGRAY);
    wcout << endl << L"Proszę czekać...";
    if (clubRef.rivalData[1] == 0) { // player home
        float tickets = ((rand() % 30) + 30) * 5000.0;
        clubRef.finances[0] += tickets; //bilety

        float tv = ((rand() % 30) + 30) * 5000.0;
        clubRef.finances[1] += tv; //Tv

        clubRef.finances[2] += ((rand() % 10) + 1) * 1000; //handel

        clubRef.finances[4] += 300000;
    }
    else {
        clubRef.finances[4] += 50000;
    }

    clubRef.finances[5] = clubRef.finances[0] + clubRef.finances[1] + clubRef.finances[2] + clubRef.finances[3] + clubRef.finances[4];
    clubRef.finances[10] = clubRef.finances[6] + clubRef.finances[7] + clubRef.finances[8] + clubRef.finances[9];
    clubRef.finances[11] = clubRef.finances[5] - clubRef.finances[10];
    clubRef.isMatch = 0; //bylM=1;
    clubRef.playerGoals = playerGoals;
    clubRef.rivalGoals = rivalGoals;
    clubRef.isAssistantMsg = 1;
    //******** manager stats ************
    clubRef.managerStats[4]++;
    if (playerGoals > rivalGoals) {
        clubRef.managerStats[5]++;
    }
    else if (playerGoals == rivalGoals) {
        clubRef.managerStats[6]++;
    }
    else {
        clubRef.managerStats[7]++;
    }
    clubRef.managerStats[8] += playerGoals;
    clubRef.managerStats[9] += rivalGoals;
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
        k = clubRef.roundNumber + 4;
        clubRef.goalsControls[k * 2] = playerGoals;
        clubRef.goalsControls[k * 2 + 1] = rivalGoals;
    }
    else if (clubRef.roundNumber > 0) {
        k = -2;
        while (k != 14) { //wszystkie mecze kolejki, losujemy wyniki meczy
            k += 2;
            int round = clubRef.roundNumber * 16;
            int homeGoals = (rand() % 5);
            int awayGoals = (rand() % 4);
            clubRef.goalsLeague[round - 16 + k] = homeGoals;
            clubRef.goalsLeague[round - 16 + k + 1] = awayGoals;
        }

        int clubIndexInRound = pRounds->getClubIndexInRoundByClubId(clubRef.roundNumber, clubRef.clubId);

        int roundIndex = clubRef.roundNumber * 16;
        if (clubIndexInRound % 2 == 0) {
            clubRef.goalsLeague[roundIndex - 16 + clubIndexInRound] = playerGoals;
            clubRef.goalsLeague[roundIndex - 16 + clubIndexInRound + 1] = rivalGoals;
        }
        else {
            clubRef.goalsLeague[roundIndex - 16 + clubIndexInRound] = playerGoals;
            clubRef.goalsLeague[roundIndex - 16 + clubIndexInRound - 1] = rivalGoals;
        }

        for (size_t index = 0; pRounds->getSize(); index++) {
            SRound &round = pRounds->get(index);

            if (clubRef.roundNumber == round.number) {
                pTable->updateAfterMatch(round, roundIndex, clubRef);
                break;
            }
        }

        pTable->save();
    }//else kolejka ligowa
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

        int k = 0;
        for (size_t index = 0; index < pTable->getSize(); index++) {
            STable &tableRef = pTable->get(index);
            pkt[k] = tableRef.data[7]; //zapisuje punkty do pkt
            clubId[k] = tableRef.num; // numer klubu
            golr[k] = tableRef.data[6]; // roznica goli
            gol[k] = tableRef.data[4]; //gole zdobyte
            blokada[k] = 0;
            k++;
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
                        if (clubId[i] == tableRef.num &&
                            pkt[i] == tableRef.data[7] &&
                            golr[k] == tableRef.data[6] &&
                            gol[v] == tableRef.data[4] &&
                            blokada[i] == 0
                        ) {
                            tableRef.data[8] = i + 1;
                            blokada[i] = tableRef.num;
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

        if (footballer.data[0] <= 11) { //zwiększam morale jedenastce za 1 skład
            footballer.data[8] += 3;
            if (footballer.data[8] > 5) {
                footballer.data[7]++;
                footballer.data[8] = 0;
                if (footballer.data[7] > 3) {
                    footballer.data[7] = 3;
                }
            }
        }

        if (footballer.data[0] > 16) { // zmniejszam morale tym co nie pojechali na mecz
            footballer.data[8] -= 2;
            if (footballer.data[8] < -5) {
                footballer.data[7]--;
                footballer.data[8] = 0;
                if (footballer.data[7] < -3) {
                    footballer.data[7] = -3;
                }
            }
        }

        footballer.data[20] = 0; // reset forma podczas meczu
        footballer.data[21] = 0; // reset gole podczas meczu
    }
    pFootballers->savePlayerTeam();

    for (size_t index = 0; index < pFootballers->getSizeRivals(); index++) {
        SFootballer &footballer = pFootballers->getRival(index);

        footballer.data[20] = 0; // reset forma podczas meczu
        footballer.data[21] = 0; // reset gole podczas meczu
        footballer.data[12] = 0; // zolte kartki podczas meczu
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

void Match::drawTeam(int usta, int tryb, int kto)
{
    TeamComposition teamComposition(pColors);
    int i = 0;

    pColors->textcolor(GREEN);
    wcout << endl << L"Lp.   Zawodnik         Po.  ";

    pColors->textcolor(LIGHTBLUE);
    wcout << L"B  ";

    pColors->textcolor(MAGENTA);
    wcout << L"O  ";

    pColors->textcolor(LIGHTCYAN);
    wcout << L"P  ";
    pColors->textcolor(LIGHTGREEN);

    wcout << L"N  ";
    pColors->textcolor(GREEN);

    wcout << L"Morale  For. ForM Kon. Gole";
    pColors->textcolor(LIGHTBLUE);
    int color = LIGHTBLUE;
    if (tryb == 16) i = 11; //dla pokazania rezerwowych w taktyce
    if (tryb == 40) i = 20; //dla dalej w Składzie

    while (i != tryb) {
        i++;
        if (tryb == 16 || i == 12) {
            color = YELLOW;
            pColors->textcolor(color);//BROWN);
        }
        if (tryb == 40) {
            color = LIGHTGRAY;
            pColors->textcolor(color);
        }

        vector<SFootballer> &tmpFoorballers = (kto > 0) ? pFootballers->getRivals() : pFootballers->getPlayersTeam();
        int clubId = (kto > 0) ? kto : pClub->get().clubId;

        for (int index = 0; index < tmpFoorballers.size(); index++) {
            SFootballer &footballer = tmpFoorballers[index];

            if (i == footballer.data[0] && clubId == footballer.data[22]) {
                wprintf(L"\n\r%2d.", footballer.data[0]);

                if (footballer.data[15] > 0 || footballer.data[19]) {
                    pColors->textbackground(RED);
                    pColors->textcolor(LIGHTGRAY);
                    wcout << L"Ko";
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
                    wprintf(L"  ");
                }

                wprintf(
                    L"%3ls%-15ls %lc  %2d %2d %2d %2d  %-7ls  %2d",
                    footballer.name,
                    footballer.surname,
                    teamComposition.getFootballerPosition(footballer.data[2]),
                    footballer.data[3],
                    footballer.data[4],
                    footballer.data[5],
                    footballer.data[6],
                    teamComposition.getMorale(footballer.data[7]).c_str(),
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

        if (kto > -1) {
            if (i == 1) {
                color = MAGENTA;
                pColors->textcolor(color);
            }

            if ((i == 5) &&
                (usta == 1 || usta == 2 || usta == 3 || usta == 4 || usta == 9 || usta == 10 || usta == 11)
            ) {
                color = LIGHTCYAN;
                pColors->textcolor(color);
            }
            else if ((i == 4) && (usta == 5 || usta == 6 || usta == 7 || usta == 8)) {
                color = LIGHTCYAN;
                pColors->textcolor(color);
            }
            else if ((i == 6) && (usta == 12 || usta == 13 || usta == 14)) {
                color = LIGHTCYAN;
                pColors->textcolor(color);
            }

            if ((i == 7) && (usta == 9)) {
                color = LIGHTGREEN;
                pColors->textcolor(color);
            }
            else if ((i == 8) && (usta == 10 || usta == 5)) {
                color = LIGHTGREEN;
                pColors->textcolor(color);
            }
            else if ((i == 10) && (usta == 11)) {
                color = LIGHTGREEN;
                pColors->textcolor(color);
            }
            else if ((i == 9) &&
                (
                    usta == 1 || usta == 2 || usta == 3 || usta == 4 || usta == 6 ||
                    usta == 7 || usta == 8 || usta == 12 || usta == 13 || usta == 14
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
}

int Match::whatHappened(
    bool isPlayerBall,
    int PnaP,
    int OnaA,
    int AnaO,
    int playerGoalkeeperSkills,
    int rivalGoalkeeperSkills,
    int whereIsAction,
    const SClub &clubRef
) {
    int x1, x2, x3, los, co;
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
        los = (rand() % 27) + 1;
        if (los < 10) {
            co = los;
        }
        else {
            if ((PnaP > 20 && isPlayerBall) || (PnaP < -20 && !isPlayerBall)) { // przewaga duża
                if (x1 == 0 && x2 >= 2 && x3 == 0) {
                    co = 1;
                }
                else if (x1 == 0 && x2 >= 2 && x3 == 1) {
                    los = (rand() % 18) + 1;
                    if (los == 1 || los == 2 || los == 3) co = 6;
                    else co = 1;
                }
                else if (x1 == 0 && x2 < 2 && x3 == 0) {
                    los = (rand() % 18) + 1;
                    if (los == 1 || los == 2) co = 5;
                    else co = 1;
                }
                else if (x1 == 0 && x2 < 2 && x3 == 1) {
                    los = (rand() % 18) + 1;
                    if (los == 1 || los == 2) co = 6;
                    else if (los == 3 || los == 4) co = 5;
                    else co = 1;
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 0) {
                    los = (rand() % 18) + 1;
                    if (los == 1 || los == 2 || los == 3) co = 1; //udana normal
                    else co = 2; //udana z kontry
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 1) {
                    los = (rand() % 18) + 1;
                    if (los == 1 || los == 2 || los == 3) co = 1; //udana normal
                    else if (los == 4 || los == 5 || los == 6) co = 6; //pressing rywala
                    else co = 2; //udana z kontry
                }
                else if (x1 == 1 && x2 < 2 && x3 == 0) {
                    los = (rand() % 18) + 1;
                    if (los == 1 || los == 2 || los == 3) co = 1; //udana normal
                    else if (los == 4 || los == 5) co = 5; //słabe podania
                    else co = 2; //udana z kontry
                }
                else if (x1 == 1 && x2 < 2 && x3 == 1) {
                    los = (rand() % 18) + 1;
                    if (los == 1 || los == 2 || los == 3) co = 1; //udana normal
                    else if (los == 4 || los == 5) co = 5; //słabe podania
                    else if (los == 6 || los == 7) co = 6; //pressing rywala
                    else co = 2; //udana z kontry
                }
            }
            else if ((PnaP > 10 && PnaP < 21 && isPlayerBall) || (PnaP < -10 && PnaP > -21 && !isPlayerBall))//przewaga mała
            {
                if (x1 == 0 && x2 >= 2 && x3 == 0) {
                    if (los == 14 || los == 22) co = 4;
                    else if (los == 15 || los == 23 || los == 26) co = 7;
                    else if (los == 16 || los == 24 || los == 27) co = 8;
                    else if (los == 17 || los == 18) co = 9;
                    else co = 1; //if (los==10||los==11||los==12||los==13||los==19||los==20||los==21||los==25)
                }
                else if (x1 == 0 && x2 >= 2 && x3 == 1) {
                    if (los == 13 || los == 21 || los == 22 || los == 26) co = 6; //pressing przeciwnika
                    else if (los == 14 || los == 23) co = 4;
                    else if (los == 15 || los == 24) co = 7;
                    else if (los == 16) co = 8;
                    else if (los == 17 || los == 18) co = 9;
                    else co = 1; //if (los==10||los==11||los==12||los==19||los==20||los==25||los==27)
                }
                else if (x1 == 0 && x2 < 2 && x3 == 0) {
                    if (los == 13 || los == 23 || los == 24 || los == 27) co = 5; //słabe podania
                    else if (los == 14) co = 4;
                    else if (los == 15 || los == 25 || los == 26) co = 7;
                    else if (los == 16) co = 8;
                    else if (los == 17 || los == 18) co = 9;
                    else co = 1; //if (los==10||los==11||los==12||los==19||los==20||los==21||los==22)
                }
                else if (x1 == 0 && x2 < 2 && x3 == 1) {
                    if (los == 12 || los == 22 || los == 24) co = 6; //pressing przeciwnika
                    else if (los == 13 || los == 23 || los == 25) co = 5; //słabe podania
                    else if (los == 14) co = 4;
                    else if (los == 15 || los == 26) co = 7;
                    else if (los == 16) co = 8;
                    else if (los == 17 || los == 18) co = 9;
                    else co = 1; //if (los==10||los==11||los==19||los==20||los==21||los==27)
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 0) {
                    if (los == 13 || los == 24 || los == 25 || los == 26) co = 3; //nie udana z kontry
                    else if (los == 14) co = 4;
                    else if (los == 15 || los == 27) co = 7;
                    else if (los == 16) co = 8;
                    else if (los == 17 || los == 18) co = 9;
                    else co = 2; //udana z kontryif (los==10||los==11||los==12||los==19||los==20||los==21||los==22||los==23)
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 1) {
                    if (los == 12 || los == 23 || los == 25) co = 6; //pressing przeciwnika
                    else if (los == 13 || los == 24 || los == 14) co = 3; //nie udana z kontry
                    else if (los == 15 || los == 26) co = 7;
                    else if (los == 16) co = 8;
                    else if (los == 17 || los == 18) co = 9;
                    else co = 2; //udana z kontry if (los==10||los==11||los==19||los==20||los==21||los==22||los==27)
                }
                else if (x1 == 1 && x2 < 2 && x3 == 0) {
                    if (los == 12 || los == 23 || los == 24) co = 5; //słaby trening podaä
                    else if (los == 13 || los == 26 || los == 14) co = 3; //nie udana z kontry
                    else if (los == 15 || los == 27) co = 7;
                    else if (los == 16) co = 8;
                    else if (los == 17 || los == 18) co = 9;
                    else co = 2; //udana z kontry if (los==10||los==11||los==19||los==20||los==21||los==22||los==25)
                }
                else { //if (x1==1&&x2<2&&x3==1)
                    if (los == 13 || los == 22 || los == 26) co = 5; //słaby trening podaä
                    else if (los == 14 || los == 23 || los == 27) co = 3; //nie udana z kontry
                    else if (los == 16) co = 7;
                        //else if (los==7) co=8;//aut
                    else if (los == 17) co = 9;
                    else if (los == 15 || los == 18 || los == 24) co = 6; //pressing rywala3x
                    else co = 2; //udana z kontryif (los==10||los==11||los==12||los==19||los==20||los==21||los==25)
                }
            }
            else if ((PnaP > 20 && !isPlayerBall) || (PnaP < -20 && isPlayerBall)) { //osłabienie duże
                if (x1 == 0 && x2 >= 2 && x3 == 0){
                    co = 4;
                }
                else if (x1 == 0 && x2 >= 2 && x3 == 1) {
                    los = (rand() % 18) + 1;
                    if (los == 1 || los == 2 || los == 3) co = 4;
                    else co = 6;
                }
                else if (x1 == 0 && x2 < 2 && x3 == 0) {
                    los = (rand() % 18) + 1;
                    if (los == 1 || los == 2) co = 4;
                    else co = 5;
                }
                else if (x1 == 0 && x2 < 2 && x3 == 1) {
                    los = (rand() % 18) + 1;
                    if (los == 1 || los == 2) co = 5;
                    else if (los == 3 || los == 4) co = 4;
                    else co = 6;
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 0) {
                    los = (rand() % 18) + 1;
                    if (los == 1 || los == 2 || los == 3) co = 4; //nieudana normal
                    else co = 3; //nieudana z kontry
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 1) {
                    los = (rand() % 18) + 1;
                    if (los == 1 || los == 2 || los == 3) co = 4; //nieudana normal
                    else if (los == 4 || los == 5 || los == 6) co = 6; //pressing rywala
                    else co = 3; //nieudana z kontry
                }
                else if (x1 == 1 && x2 < 2 && x3 == 0) {
                    los = (rand() % 18) + 1;
                    if (los == 1 || los == 2 || los == 3) co = 4; //nieudana normal
                    else if (los == 4 || los == 5) co = 5; //słabe podania
                    else co = 3; //nieudana z kontry
                }
                else if (x1 == 1 && x2 < 2 && x3 == 1) {
                    los = (rand() % 18) + 1;
                    if (los == 1 || los == 2 || los == 3) co = 4; //nieudana normal
                    else if (los == 4 || los == 5) co = 5; //słabe podania
                    else if (los == 6 || los == 7) co = 6; //pressing rywala
                    else co = 3; //nieudana z kontry
                }
            }
            else if ((PnaP < -10 && PnaP > -21 && isPlayerBall) || (PnaP > 10 && PnaP < 21 && !isPlayerBall)) { //osłabienie
                if (x1 == 0 && x2 >= 2 && x3 == 0) {
                    if (los == 14 || los == 22 || los == 23 || los == 27) co = 1;
                    else if (los == 15 || los == 24) co = 7;
                    else if (los == 16 || los == 25) co = 8;
                    else if (los == 17 || los == 18) co = 9;
                    else co = 4; //if (los==10||los==11||los==12||los==13||los==19||los==20||los==21||los==26)
                }
                else if (x1 == 0 && x2 >= 2 && x3 == 1) {
                    if (los == 10 || los == 26) co = 4;
                    else if (los == 14 || los == 23 || los == 25 || los == 27) co = 1;
                    else if (los == 15 || los == 24) co = 7;
                    else if (los == 16) co = 8;
                    else if (los == 17 || los == 18) co = 9;
                    else co = 6; //pressing przeciwnikaif (los==11||los==12||los==13||los==19||los==20||los==21||los==22)
                }
                else if (x1 == 0 && x2 < 2 && x3 == 0) {
                    if (los == 10 || los == 23) co = 4;
                    else if (los == 14 || los == 25 || los == 26 || los == 27) co = 1;
                    else if (los == 15 || los == 24) co = 7;
                    else if (los == 16) co = 8;
                    else if (los == 17 || los == 18) co = 9;
                    else co = 5; //słabe podania if (los==11||los==12||los==13||los==19||los==20||los==21||los==22)
                }
                else if (x1 == 0 && x2 < 2 && x3 == 1) {
                    if (los == 10) co = 4;
                    else if (los == 11 || los == 12 || los == 19 || los == 20) co = 6; //pressing przeciwnika
                    else if (los == 13 || los == 22 || los == 23 || los == 21) co = 5; //słabe podania
                    else if (los == 14 || los == 25 || los == 26 || los == 27) co = 1;
                    else if (los == 15 || los == 24) co = 7;
                    else if (los == 16) co = 8;
                    else co = 9;
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 0) {
                    //if (los==10||los==11||los==12||los==19||los==20||los==21||los==27) co=3;//nie udana z kontry
                    if (los == 13 || los == 22 || los == 23 || los == 24) co = 2; //udana z kontry
                    else if (los == 14 || los == 25) co = 4;
                    else if (los == 15 || los == 26) co = 7;
                    else if (los == 16) co = 8;
                    else if (los == 17 || los == 18) co = 9;
                    else co = 3; //nie udana z kontry if (los==10||los==11||los==12||los==19||los==20||los==21||los==27)
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 1) {
                    if (los == 10 || los == 11 || los == 20 || los == 21) co = 3; //nieudana z kontry
                    else if (los == 12 || los == 22 || los == 23 || los == 27) co = 6; //pressing przeciwnika
                    else if (los == 13 || los == 24 || los == 25 || los == 19) co = 2; //udana z kontry
                    else if (los == 14) co = 4;
                    else if (los == 15 || los == 26) co = 7;
                    else if (los == 16) co = 8;
                    else co = 9;
                }
                else if (x1 == 1 && x2 < 2 && x3 == 0) {
                    if (los == 10 || los == 11 || los == 19 || los == 20) co = 3; //nieudana z kontry
                    else if (los == 12 || los == 21 || los == 22 || los == 23) co = 5; //słaby trening podaä
                    else if (los == 13 || los == 24 || los == 25 || los == 26) co = 2; //udana z kontry
                    else if (los == 14) co = 4;
                    else if (los == 15 || los == 27) co = 7;
                    else if (los == 16) co = 8;
                    else co = 9;
                }
                else { //if (x1==1&&x2<2&&x3==1)
                    if (los == 10 || los == 11 || los == 12 || los == 19) co = 3; //nieudana z kontry
                    else if (los == 13 || los == 22 || los == 23) co = 5; //słaby trening podaä
                    else if (los == 14 || los == 24 || los == 25 || los == 26) co = 2; // udana z kontry
                    else if (los == 15) co = 4;
                    else if (los == 16 || los == 27) co = 7;
                        //else if (los==7) co=8;//aut
                    else if (los == 17) co = 9;
                    else co = 6; //pressing rywala3x
                }
            }
            else { //if (PnaP>=-10&&PnaP<=10)//siły wyrównane
                if (x1 == 0 && x2 >= 2 && x3 == 0) {
                    if (los == 10 || los == 11 || los == 19 || los == 24 || los == 25) co = 1;
                    else if (los == 12 || los == 13 || los == 20 || los == 26) co = 4;
                    else if (los == 14 || los == 15 || los == 21) co = 7;
                    else if (los == 16 || los == 17 || los == 22 || los == 27) co = 8;
                    else co = 9;
                }
                else if (x1 == 0 && x2 >= 2 && x3 == 1) {
                    if (los == 10 || los == 11 || los == 19 || los == 20 || los == 24) co = 1;
                    else if (los == 12 || los == 21) co = 4;
                    else if (los == 13 || los == 22) co = 6; //pressing rywala
                    else if (los == 14 || los == 15 || los == 23) co = 7;
                    else if (los == 16 || los == 17 || los == 25 || los == 26) co = 8;
                    else co = 9;
                }
                else if (x1 == 0 && x2 < 2 && x3 == 0) {
                    if (los == 10 || los == 11 || los == 25 || los == 26 || los == 27) co = 1;
                    else if (los == 12 || los == 24) co = 4;
                    else if (los == 13 || los == 23) co = 5; //słabe podania
                    else if (los == 14 || los == 15 || los == 22) co = 7;
                    else if (los == 16 || los == 17 || los == 20 || los == 21) co = 8;
                    else co = 9;
                }
                else if (x1 == 0 && x2 < 2 && x3 == 1) {
                    if (los == 10 || los == 11 || los == 25 || los == 26 || los == 27) co = 1;
                    else if (los == 12 || los == 24) co = 6; //pressing przeciwnika
                    else if (los == 13 || los == 23) co = 5; //słabe podania
                    else if (los == 14 || los == 15 || los == 22) co = 7;
                    else if (los == 16 || los == 17 || los == 20 || los == 21) co = 8;
                    else co = 9;
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 0) {
                    if (los == 10 || los == 11 || los == 25 || los == 26 || los == 27) co = 2; //unanie z kontry
                    else if (los == 12 || los == 13 || los == 23 || los == 24) co = 3; //nieudana kontra
                    else if (los == 14 || los == 15 || los == 22) co = 7;
                    else if (los == 16 || los == 17 || los == 20 || los == 21) co = 8;
                    else co = 9;
                }
                else if (x1 == 1 && x2 >= 2 && x3 == 1) {
                    if (los == 10 || los == 11 || los == 25 || los == 26 || los == 27) co = 2; //unanie z kontry
                    else if (los == 12 || los == 24) co = 3; //nieudana kontra
                    else if (los == 13 || los == 23) co = 6; //pressing przeciwnika
                    else if (los == 14 || los == 15 || los == 22) co = 7;
                    else if (los == 16 || los == 17 || los == 20 || los == 21) co = 8;
                    else co = 9;
                }
                else if (x1 == 1 && x2 < 2 && x3 == 0) {
                    if (los == 10 || los == 11 || los == 25 || los == 26 || los == 27) co = 2; //unanie z kontry
                    else if (los == 12 || los == 24) co = 3; //nieudana kontra
                    else if (los == 13 || los == 23) co = 5; //słaby trening
                    else if (los == 14 || los == 15 || los == 22) co = 7;
                    else if (los == 16 || los == 17 || los == 20 || los == 21) co = 8;
                    else co = 9;
                }
                else { //if (x1==1&&x2<2&&x3==1)
                    if (los == 10 || los == 11 || los == 25 || los == 26 || los == 27) co = 2; //unanie z kontry
                    else if (los == 12 || los == 24) co = 3; //nieudana kontra
                    else if (los == 13 || los == 23) co = 5; //słaby trening
                    else if (los == 14 || los == 15 || los == 22) co = 7;
                    else if (los == 16 || los == 21) co = 8;
                    else if (los == 17 || los == 20) co = 6; //pressing rywala
                    else co = 9;
                }
            }
        }
    }//whereIsAction = ACTION_IN_MIDDLEFIELD  P/P
    else if (whereIsAction == ACTION_IN_WINGER) { //skrzydłowy przy piłce, A/O
        los = (rand() % 12) + 1;
        if (los < 5) {
            co = los + 9;
        }
        else {
            if ((AnaO > 10 && isPlayerBall) || (OnaA < -10 && !isPlayerBall)) { //duża przewaga ataku
                los = (rand() % 10) + 1;
                if (los == 1 || los == 2) co = 11;
                else co = 10;
            }
            else if ((AnaO > 0 && AnaO < 11 && isPlayerBall) || (OnaA > -11 && OnaA < 0 && !isPlayerBall)) { //przewaga ataku
                if (los == 6 || los == 7 || los == 8 || los == 9) co = 10;
                else if (los == 10 || los == 11 || los == 12) co = 11;
                else co = 13;
            }//przewaga w ataku
            else if ((AnaO < -40 && isPlayerBall) || (OnaA > 40 && !isPlayerBall)) { //duża przewaga obrony
                los = (rand() % 10) + 1;
                if (los == 1 || los == 2) co = 12;
                else co = 13;
            }
            else if ((AnaO > -41 && AnaO < -20 && isPlayerBall) || (OnaA > 20 && OnaA < 41 && !isPlayerBall)) { //przewaga obrony
                if (los == 5 || los == 6 || los == 7 || los == 8 || los == 9) co = 13;
                else if (los == 10 || los == 11) co = 10;
                else co = 11;
            }//przewaga w obronie
            else { //if ((AnaO > -21&&AnaO<1&& isPlayerBall)||(OnaA<21&&OnaA > -1&& !isPlayerBall))
                if (los > 4 && los < 9) co = los + 5;
                else co = los + 1;
            }
        }//else
    }
    else if (whereIsAction == ACTION_IN_PANELTY_AREA) { //pole karne A/O
        los = (rand() % 30) + 1;
        if (isPlayerBall) {
            x1 = clubRef.rivalInst[3]; //pułapki ofsajdowe rywala
        }
        else {
            x1 = clubRef.inst[3];
        }

        if (los < 11) {
            co = los + 13;
        }
        else {
            if ((AnaO > 10 && isPlayerBall) || (OnaA < -10 && !isPlayerBall)) { //duża przewaga ataku
                if (x1 == 0) {
                    los = (rand() % 10) + 1;
                    if (los == 1 || los == 2 || los == 3) co = 21;
                    else co = 22;
                }
                else if (x1 == 1) {
                    los = (rand() % 15) + 1;
                    if (los == 1 || los == 2 || los == 3) co = 21;
                    else if (los == 4 || los == 5 || los == 6) co = 14;
                    else if (los == 7) co = 15;
                    else co = 22;
                }
            }
            else if ((AnaO > 0 && AnaO < 11 && isPlayerBall) || (OnaA > -11 && OnaA < 0 && !isPlayerBall)) { //przewaga ataku
                if (x1 == 0) {
                    if (los == 11 || los == 27) co = 16;
                    else if (los == 12 || los == 26) co = 17;
                    else if (los == 13 || los == 25) co = 18;
                    else if (los == 14 || los == 15 || los == 24 || los == 30) co = 20;
                    else if (los == 16 || los == 17 || los == 23) co = 21;
                    else if (los == 18 || los == 19 || los == 21 || los == 22 || los == 29) co = 22;
                    else co = 23;
                }
                else if (x1 == 1) {
                    if (los == 11 || los == 23 || los == 27) co = 14;
                    else if (los == 12 || los == 28) co = 15;
                    else if (los == 13) co = 16;
                    else if (los == 14) co = 17;
                    else if (los == 15 || los == 29) co = 18;
                    else if (los == 16 || los == 24) co = 20;
                    else if (los == 17 || los == 22 || los == 26) co = 21;
                    else if (los == 18 || los == 19 || los == 21 || los == 25 || los == 30) co = 22;
                    else co = 23;
                }
            }
            else if ((AnaO < -40 && isPlayerBall) || (OnaA > 40 && !isPlayerBall)) { //duża przewaga obrony
                if (x1 == 0) {
                    los = (rand() % 10) + 1;
                    if (los == 1 || los == 2 || los == 3) co = 17;
                    else co = 16;
                }
                else if (x1 == 1) {
                    los = (rand() % 15) + 1;
                    if (los == 1 || los == 2 || los == 3) co = 17;
                    else if (los == 4 || los == 5 || los == 6) co = 16;
                    else if (los == 7) co = 15;
                    else co = 14;
                }
            }
            else if ((AnaO > -41 && AnaO < -20 && isPlayerBall) || (OnaA > 20 && OnaA < 41 && !isPlayerBall)) { //przewaga obrony
                if (x1 == 0) {
                    if (los == 11 || los == 12 || los == 13 || los == 21 || los == 22 || los == 30) co = 16;
                    else if (los == 14 || los == 23 || los == 24) co = 17;
                    else if (los == 15) co = 18;
                    else if (los == 16 || los == 17 || los == 27) co = 20;
                    else if (los == 18 || los == 25) co = 21;
                    else if (los == 19 || los == 26) co = 22;
                    else co = 23; //3x
                }
                else if (x1 == 1) {
                    if (los == 11 || los == 12 || los == 13 || los == 21 || los == 22 || los == 29) co = 14;
                    else if (los == 30) co = 16;
                    else if (los == 14 || los == 23) co = 15;
                    else if (los == 15 || los == 28) co = 17;
                    else if (los == 16) co = 18;
                    else if (los == 17 || los == 27) co = 20;
                    else if (los == 18 || los == 26) co = 21;
                    else if (los == 19 || los == 25) co = 22;
                    else co = 23;
                }
            }
            else if ((AnaO > -21 && AnaO < 1 && isPlayerBall) || (OnaA < 21 && OnaA > -1 && !isPlayerBall)) {
                if (x1 == 0) {
                    if (los == 11 || los == 12 || los == 13 || los == 21) co = 16;
                    else if (los == 30) co = 14;
                    else if (los == 14 || los == 23) co = 17;
                    else if (los == 15) co = 18;
                    else if (los == 16 || los == 17 || los == 24 || los == 27) co = 20;
                    else if (los == 18 || los == 25 || los == 29) co = 21;
                    else if (los == 19 || los == 26 || los == 22) co = 22;
                    else co = 23;
                }
                else if (x1 == 1) {
                    los = (rand() % 10) + 1;
                    co = los + 13;
                }
            }
        }//else
    }//dla whereIsAction == ACTION_IN_PANELTY_AREA A/O
    else if (whereIsAction == ACTION_IN_GOAL_SITUATION ||
             whereIsAction == ACTION_IN_DIRECT_FREE_KICK
    ) { //obrona B playerGoalkeeperSkills max 43, mini 8, śred 26
        los = (rand() % 12) + 1;
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
            if (los == 1 || los == 2 || los == 3 || los == 4) co = 28; //gol
            else if (los == 5 || los == 6 || los == 7) co = 27; //róg
            else if (los == 8 || los == 9) co = 26; //niepewnia obrona
            else if (los == 10) co = 29; //gol nieuznany
            else if (los == 11) co = 24; //obronił
            else co = 30; //strzał niecelny 1x
        }
        else if (x1 >= 15 && x1 < 25) {
            if (los == 1 || los == 2 || los == 3) co = 28; //gol
            else if (los == 4 || los == 5 || los == 6) co = 27; //róg
            else if (los == 7 || los == 8) co = 26; //niepewnia obrona
            else if (los == 9) co = 29; //gol nieuznany
            else if (los == 10 || los == 11) co = 24; //obronił
            else co = 30; //strzał niecelny
        }
        else if (x1 >= 25 && x1 < 35) {
            if (los == 1 || los == 2 || los == 3) co = 28; //gol
            else if (los == 4 || los == 5) co = 27; //róg
            else if (los == 6 || los == 7) co = 26; //niepewnia obrona
            else if (los == 8) co = 29; //gol nieuznany
            else if (los == 9 || los == 10) co = 24; //obronił
            else if (los == 11) co = 25; //bez problemu
            else co = 30; //strzał niecelny1x
        }
        else if (x1 > 34) {
            if (los == 1 || los == 2) co = 28; //gol
            else if (los == 3 || los == 4) co = 27; //róg
            else if (los == 5) co = 26; //niepewnia obrona
            else if (los == 6) co = 29; //gol nieuznany
            else if (los == 7 || los == 8) co = 24; //obronił
            else if (los == 9 || los == 10) co = 25; //bez problemu
            else co = 30; //strzał niecelny2x
        }
    } // whereIsAction = ACTION_IN_GOAL_SITUATION // obrona B
    else if (whereIsAction == ACTION_IN_PANELTY_OR_1ON1) { //obrona B - karny, sma na sam
        los = (rand() % 9) + 1;
        if (isPlayerBall) {
            x1 = rivalGoalkeeperSkills - clubRef.trained[2]; //um. bramkarza-stałe fragmenty
        }
        else {
            x1 = playerGoalkeeperSkills - 2;
        }

        if (x1 < 15) {
            if (los == 1 || los == 2 || los == 3 || los == 4 || los == 5 || los == 6 || los == 7) co = 28; //gol
            else if (los == 8) co = 24; //obronił
            else co = 30; //strzał niecelny 1x
        }
        else if (x1 >= 15 && x1 < 25) {
            if (los == 1 || los == 2 || los == 3 || los == 4 || los == 5 || los == 6) co = 28; //gol
            else if (los == 8) co = 24; //obronił
            else co = 30; //strzał niecelny 2x
        }
        else if (x1 >= 25 && x1 < 35) {
            if (los == 1 || los == 2 || los == 3 || los == 4 || los == 5) co = 28; //gol
            else if (los == 8 || los == 6) co = 24; //obronił
            else co = 30; //strzał niecelny 2x
        }
        else if (x1 > 34) {
            if (los == 1 || los == 2 || los == 3 || los == 4) co = 28; //gol
            else if (los == 8 || los == 6 || los == 5) co = 24; //obronił
            else co = 30; //strzał niecelny 2x
        }
    }//whereIsAction = ACTION_IN_PANELTY_OR_1ON1 //obrona B

    return co;
}

void Match::drawBoard(
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
    wprintf(L"     %2d     Strzały     %2d", shootHome, shootAway);

    pColors->textbackground(GREEN);
    wcout << endl << BOX_LIGHT_VERTICAL;

    pColors->textcolor(isHome ? BLUE : RED);
    wcout << L"   O";

    pColors->textcolor(WHITE);
    wcout << L"/";

    pColors->textcolor(isHome ? RED : BLUE);
    wcout << L"A   ";

    pColors->textcolor(WHITE);
    wcout << BOX_LIGHT_DASH_VERTICAL;//L"|";

    pColors->textcolor(isHome ? BLUE : RED);
    wcout << L"     P";

    pColors->textcolor(WHITE);
    wcout << L"/";

    pColors->textcolor(isHome ? RED : BLUE);
    wcout << L"P     ";

    pColors->textcolor(WHITE);
    wcout << BOX_LIGHT_DASH_VERTICAL;//L"|";

    pColors->textcolor(isHome ? BLUE : RED);
    wcout << L"   A";

    pColors->textcolor(WHITE);
    wcout << L"/";

    pColors->textcolor(isHome ? RED : BLUE);
    wcout << L"O   ";

    pColors->textcolor(WHITE);
    wcout << BOX_LIGHT_VERTICAL;

    pColors->textbackground(BLACK);
    wprintf(L"     %2d  Strzały celne  %2d", accurateShootHome, accurateShootAway);

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
    wprintf(L"     %2d   Rzuty rożne   %2d", cornersHome, cornersAway);

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
    wprintf(L"     %2d     Spalone     %2d", offsidesHome, offsidesAway);

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
    wprintf(L"     %2d      Faule      %2d", faulsHome, faulsAway);

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
    wprintf(L"     %2d   Rzuty karne   %2d", paneltiesHome, paneltiesAway);

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
    wprintf(L"     %2d   żółte kartki  %2d", yellowsHome, yellowsAway);

    pColors->textcolor(WHITE);
    pColors->textbackground(GREEN);
    wcout << endl << BOX_LIGHT_VERTICAL << L"  ";
    isHome
        ? drawOnaA(isHome, OnaA)
        : drawAnaO(isHome, AnaO);

    pColors->textcolor(WHITE);
    wcout << L"   " << BOX_LIGHT_DASH_VERTICAL << L"    ";
    if (PnaP < -20) {
        pColors->textcolor(RED);
        wcout << BALL_FULL << BALL_FULL << BALL_FULL << BALL_FULL;
    }
    else if (PnaP >= -20 && PnaP < -10) {
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
    else if (PnaP >= -10 && PnaP < 11) {
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
    else if (PnaP >= 11 && PnaP < 21) {
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
    else if (PnaP >= 21) {
        pColors->textcolor(BLUE);
        wcout << BALL_FULL << BALL_FULL << BALL_FULL << BALL_FULL;
    }

    pColors->textcolor(WHITE);
    wcout << L"     " << BOX_LIGHT_DASH_VERTICAL << L"   ";

    isHome
        ? drawAnaO(isHome, AnaO)
        : drawOnaA(isHome, OnaA);

    pColors->textcolor(WHITE);
    wcout << L"  " << BOX_LIGHT_VERTICAL;

    pColors->textcolor(LIGHTRED);
    pColors->textbackground(BLACK);
    wprintf(L"     %2d Czerwone kartki %2d", redsHome, redsAway);

    pColors->textcolor(WHITE);
    pColors->textbackground(GREEN);
    wcout << endl << BOX_LIGHT_UP_RIGHT << BOX_LIGHT_HORIZONTAL;

    pColors->textcolor(BLACK);
    wprintf(L"%4d", isHome ? OnaA : AnaO);

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
    wprintf(L"%4d", PnaP);
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
    wprintf(L"%4d", isHome ? AnaO : OnaA);
    pColors->textcolor(WHITE);
    wcout << BOX_LIGHT_HORIZONTAL << BOX_LIGHT_HORIZONTAL << BOX_LIGHT_UP_LEFT;

    pColors->textbackground(BLACK);
    wprintf(L"   %3d%% Posiadanie piłki %d%%", ballPossHome, ballPossAway);

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

void Match::drawOnaA(bool isHome, int OnaA)
{
    if (OnaA < -10) {
        pColors->textcolor(RED);
        wcout << BALL_FULL << BALL_FULL << BALL_FULL << BALL_FULL;
    }
    else if (OnaA >= -10 && OnaA < 0) {
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
    else if (OnaA >= 0 && OnaA < 21) {
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
    else if (OnaA >= 21 && OnaA < 41) {
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
    else if (OnaA >= 41) {
        pColors->textcolor(BLUE);
        wcout << BALL_FULL << BALL_FULL << BALL_FULL << BALL_FULL;
    }
}

void Match::drawAnaO(bool isHome, int AnaO)
{
    if (AnaO < -40) {
        pColors->textcolor(RED);
        wcout << BALL_FULL << BALL_FULL << BALL_FULL << BALL_FULL;
    }
    else if (AnaO >= -40 && AnaO < -20) {
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
    else if (AnaO >= -20 && AnaO < 1) {
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
    else if (AnaO >= 1 && AnaO < 11) {
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
    else if (AnaO >= 11) {
        pColors->textcolor(BLUE);
        wcout << BALL_FULL << BALL_FULL << BALL_FULL << BALL_FULL;
    }
}

void Match::drawWhoScored(int mingol1[], int mingol2[], wstring dlagol1[], wstring dlagol2[])
{
    int i = 3;
    for (int k = 0; k < 10; k++) {
        if (mingol1[k] != 0) {
            wprintf(L"\n\r%19ls %2d ", dlagol1[k].c_str(), mingol1[k]);
            i--;
        }

        if (mingol2[k] != 0 && mingol1[k] != 0) {
            wprintf(L" %2d %-19ls", mingol2[k], dlagol2[k].c_str());
        }
        else if (mingol2[k] != 0 && mingol1[k] == 0) {
            wprintf(L"\n\r                        %2d %-19ls", mingol2[k], dlagol2[k].c_str());
            i--;
        }
    }

    if (i == 3) {
        wcout << endl << endl << endl;
    }
    else if (i == 2) {
        wcout << endl << endl;
    }
    else if (i == 1) {
        wcout << endl;
    }
}

void Match::saveMatchMessageToFile(int bkgcolor, int minuta, const wchar_t *message)
{
    SLastMatch lastMatch;
    lastMatch.textcolor = bkgcolor == BLUE ? LIGHTBLUE : RED;

    swprintf(lastMatch.text, MAX_NEWS_LENGTH, L"%02d min. %ls", minuta + 2, message);

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
                    wcout << L"Obecne ustawienie: " << settings[clubRef.teamSetting - 1];
                    pColors->textcolor(LIGHTGRAY);
                    wcout << endl << L"Wybierz ustawienie zespołu:" << endl;
                    for (int i = 0; i < 14; i++) {
                        wcout << endl;
                        if (i < 9) {
                            wcout << L" ";
                        }
                        wcout << i + 1 << L". " << settings[i];
                    }
                    wcout << endl << endl << L"Wpisz odpowiednią cyfrę: ";
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
                wcout << endl << L"Podaj numer Lp., spacja, drugi numer i Enter: ";
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
                        msgWhoBall[9] = 1;
                        whoPlayerChanges = 1;
                        wiado[9] = 6;
                        if (footballer1 <= 11 && footballer2 > 11) { // stalenie w wiadomosci kto kogo zastepuje
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
                wcout << L"REZERWOWI - " << pClub->getClubName(clubRef.clubId - 1);

                drawTeam(clubRef.teamSetting, 16, 0);

                pColors->textcolor(LIGHTGRAY);
                wcout << endl << endl << L"Naciśnij dowolny klawisz...";
                pInput->getKeyBoardPressed();
                break;
            }
            case 'I': {
                int belka = 1;
                wchar_t menuInstructions = 0;
                do {
                    pInput->clrscr();
                    pColors->textcolor(WHITE);
                    wcout << L"INSTRUKCJE DLA DRUŻYNY - " << pClub->getClubName(clubRef.clubId - 1);

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
                    wcout << endl << L" Q Powrót" << endl;

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
                wcout << L"REZERWOWI - " << pClub->getClubName(clubRef.rivalData[0] - 1);

                drawTeam(clubRef.rivalData[2], 16, clubRef.rivalData[0]);

                pColors->textcolor(LIGHTGRAY);
                wcout << endl << endl << L"Naciśnij dowolny klawisz...";
                pInput->getKeyBoardPressed();
                break;
            }
            case 'I': {
                pInput->clrscr();
                pColors->textcolor(WHITE);
                wcout << L"INSTRUKCJE DLA DRUŻYNY - " << pClub->getClubName(clubRef.rivalData[0] - 1);

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
                wcout << endl << endl << L"Naciśnij dowolny klawisz...";
                pInput->getKeyBoardPressed();
                break;
            }
        }
    }
    while (menuTeamSetting != 'Q');
}

wstring Match::getFootballerSurname(bool isPlayerBall, int footabllerId)
{
    return isPlayerBall ? msgPlayerSurnames[footabllerId] : msgRivalSurnames[footabllerId];
}

int Match::getArbiterDecision(int instrTreatment)
{
    int chance = (rand() % 6);

    switch (instrTreatment) {
        case INSTR_TREATMENT_SOFT: {
            if (chance == 0 || chance == 1 || chance == 2) {
                return 1; // nie ma kartki 1/2
            }
            else if (chance == 3 || chance == 4) {
                return 2; // słowne upomienie 1/3
            }

            return 3; // żólta kartka 1/6
        }
        case INSTR_TREATMENT_HARD: {
            if (chance == 0) {
                return 1; //nie ma kartki 1/6
            }
            else if (chance == 1 || chance == 2) {
                return 2; //słowne upomienie 1/3
            }

            return 3; //żólta kartka 1/2
        }
        case INSTR_TREATMENT_NORMAL:
        default: {
            if (chance == 0 || chance == 1) {
                return 1; // nie ma kartki 1/3
            }
            else if (chance == 2 || chance == 3) {
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
        case T4_4_2:
        case T4_4_2_DEF:
        case T4_4_2_ATT:
        case T4_4_2_DIA: {
            return (rand() % 4) + 6; // middlefield 6-9
        }
        case T3_4_3: {
            return (rand() % 4) + 5; // middlefield 5-8
        }
        case T3_5_2:
        case T3_5_2_DEF:
        case T3_5_2_ATT: {
            return (rand() % 5) + 5; // middlefield 5-9
        }
        case T4_2_4: {
            return (rand() % 2) + 6;
        }
        case T4_3_3: {
            return (rand() % 3) + 6;;
        }
        case T4_5_1: {
            return (rand() % 5) + 6;
        }
        case T5_3_2:
        case T5_3_2_DEF:
        case T5_3_2_ATT: {
            return (rand() % 3) + 7;;
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
            return (rand() % 4) + 2;
        }
        case T3_4_3:
        case T3_5_2:
        case T3_5_2_DEF:
        case T3_5_2_ATT: {
            return (rand() % 3) + 2;
        }
        default: {
            return (rand() % 5) + 2;
        }
    }
}

int Match::getFootballerIdWhoShootDistance(int teamSetting)
{
    switch (teamSetting) {
        case T3_5_2:
        case T3_5_2_DEF:
        case T3_5_2_ATT: {
            return (rand() % 5) + 5;
        }
        case T4_5_1: {
            return (rand() % 5) + 6;
        }
        case T5_3_2:
        case T5_3_2_DEF:
        case T5_3_2_ATT: {
            return (rand() % 3) + 7;
        }
        case T3_4_3: {
            return (rand() % 4) + 5;
        }
        case T4_3_3: {
            return (rand() % 4) + 6;
        }
        default: {
            return (rand() % 4) + 6;
        }
    }
}

int Match::getGoooalMinute(int minuta, int koniec)
{
    int result = minuta + 2;
    if (result > 45 && (koniec == 0 || koniec == 1)) {
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
