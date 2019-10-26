
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctype.h>
#include <cwchar>
#include <algorithm>
#include <thread>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <vector>

#include "ConioColors.h"
#include "Match.h"
#include "Structs.h"
#include "Colors.h"
#include "BoxDrawingChars.h"
#include "TeamComposition.h"

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

    SNews news;
    while (fread(&news, sizeof (news), 1, f) == 1) {
        matchMsgs.push_back(news);
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
    int pilka = 0, ktoZacz = 0, minuta = 0, start = 0, k, czas = 2;
    int sumaB = 0, sumaO = 0, sumaP = 0, sumaN = 0; // sumy B, O, P, N duzyny gracza
    int sumaB2 = 0, sumaO2 = 0, sumaP2 = 0, sumaN2 = 0; // sumy B, O, P, N duzyny rywala
    int sumaBx = 0, sumaOx = 0, sumaPx = 0, sumaNx = 0;
    int gdzie = 1;
    int i = 0, los = 0, co = 0, x1 = 0, x2 = 0, x3 = 0, pos1 = 0, pos2 = 0, pos = 0;
    int pam; // pamieta numer zawodnika, ktorego trzeba przeniesc do nastepnych wiadomosci, np. "%s strzela!", pam = x, "%s strzelił"
    int q = 0, str[8] = {0, 0, 0, 0, 0, 0, 0, 0}, str2[8] = {0, 0, 0, 0, 0, 0, 0, 0}, dystans = 0;
    int pkt[16], pkt2[16], golr[16], gol[16], blokada[16], walkower = 0;
    float h = 0, strefa[6] = {0, 0, 0, 0, 0, 0}, premia = 0;

    howManyPlayerChanges = 0;
    isPlayerChanges = true;
    whoPlayerChanges = 0;

    time_t tmptime = time(NULL);
    struct tm t = *localtime(&tmptime);

    srand(time(NULL));

    walkower = pFootballers->getSizePlayerTeam();

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

        pInput->clrscr();
        pColors->textcolor(WHITE);
        if (clubRef.rivalData[1] == 0) { //w domu
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

        bool isHome = clubRef.rivalData[1] == 0;
        drawWhoScored(
            isHome ? mingol1 : mingol2,
            isHome ? mingol2 : mingol1,
            isHome ? dlagol1 : dlagol2,
            isHome ? dlagol2 : dlagol1
        );

        k = 0;
        sumaB = 0;
        sumaO = 0;
        sumaP = 0;
        sumaN = 0;
        sumaB2 = 0;
        sumaO2 = 0;
        sumaP2 = 0;
        sumaN2 = 0;

        while (k != 2) {
            k++;
            sumaBx = 0;
            sumaOx = 0;
            sumaPx = 0;
            sumaNx = 0;
            premia = 0;

            // sumowanie formacji
            vector<SFootballer> &tmpFootballers = k == 1 ? pFootballers->getPlayersTeam() : pFootballers->getRivals();
            int clubId      = k == 1 ? clubRef.clubId : clubRef.rivalData[0];
            int clubTactic  = k == 1 ? clubRef.teamSetting : clubRef.rivalData[2];

            for (size_t index = 0; index < tmpFootballers.size(); index++) {
                SFootballer &footballer = tmpFootballers[index];

                if (clubId == footballer.data[22] && footballer.data[12] < 2) {
                    if (footballer.data[0] < 12) {
                        premia += footballer.finances[2];
                    }

                    if (footballer.data[0] == 1) {
                        sumaBx = footballer.data[3] + footballer.data[7] + footballer.data[9] + footballer.data[20] + footballer.data[11] / 10;
                    }

                    if (footballer.data[0] == 2 || footballer.data[0] == 3 || footballer.data[0] == 4) {
                        sumaOx += footballer.data[4] + footballer.data[7] + footballer.data[9] + footballer.data[20] + footballer.data[11] / 10;
                    }

                    if ((footballer.data[0] == 5) &&
                        (
                            clubTactic == 1 || clubTactic == 2 || clubTactic == 3 || clubTactic == 4 ||
                            clubTactic == 9 || clubTactic == 10 || clubTactic == 11 || clubTactic == 12 || clubTactic == 13 || clubTactic == 14
                        )
                    ) {
                        sumaOx += footballer.data[4] + footballer.data[7] + footballer.data[9] + footballer.data[20] + footballer.data[11] / 10;
                    }
                    else if (footballer.data[0] == 5) {
                        sumaPx += footballer.data[5] + footballer.data[7] + footballer.data[9] + footballer.data[20] + footballer.data[11] / 10;
                    }

                    if ((footballer.data[0] == 6) && (clubTactic == 12 || clubTactic == 13 || clubTactic == 14)) {
                        sumaOx += footballer.data[4] + footballer.data[7] + footballer.data[9] + footballer.data[20] + footballer.data[11] / 10;
                    }
                    else if (footballer.data[0] == 6) {
                        sumaPx += footballer.data[5] + footballer.data[7] + footballer.data[9] + footballer.data[20] + footballer.data[11] / 10;
                    }

                    if (footballer.data[0] == 7) {
                        sumaPx += footballer.data[5] + footballer.data[7] + footballer.data[9] + footballer.data[20] + footballer.data[11] / 10;
                    }

                    if ((footballer.data[0] == 8) && (clubTactic == 9)) {
                        sumaNx += footballer.data[6] + footballer.data[7] + footballer.data[9] + footballer.data[20] + footballer.data[11] / 10;
                    }
                    else if (footballer.data[0] == 8) {
                        sumaPx += footballer.data[5] + footballer.data[7] + footballer.data[9] + footballer.data[20] + footballer.data[11] / 10;
                    }

                    if ((footballer.data[0] == 9) && (clubTactic == 5 || clubTactic == 10 || clubTactic == 9)) {
                        sumaNx += footballer.data[6] + footballer.data[7] + footballer.data[9] + footballer.data[20] + footballer.data[11] / 10;
                    }
                    else if (footballer.data[0] == 9) {
                        sumaPx += footballer.data[5] + footballer.data[7] + footballer.data[9] + footballer.data[20] + footballer.data[11] / 10;
                    }

                    if ((footballer.data[0] == 10) && (clubTactic == 11)) {
                        sumaPx += footballer.data[5] + footballer.data[7] + footballer.data[9] + footballer.data[20] + footballer.data[11] / 10;
                    }
                    else if (footballer.data[0] == 10) {
                        sumaNx += footballer.data[6] + footballer.data[7] + footballer.data[9] + footballer.data[20] + footballer.data[11] / 10;
                    }

                    if (footballer.data[0] == 11) {
                        sumaNx += footballer.data[6] + footballer.data[7] + footballer.data[9] + footballer.data[20] + footballer.data[11] / 10;
                    }
                }
            }
            // end - sumowanie formacji

            if (k == 1) {
                sumaB = sumaBx;
                sumaO = sumaOx;
                sumaP = sumaPx;
                sumaN = sumaNx;
                sumaN += premia / 1000;
                sumaP += premia / 1000;

                if (clubRef.inst[1] == INSTR_TREATMENT_HARD) {
                    // obchodzenie sie twarde
                    sumaO += 5;
                    sumaP += 5;
                    sumaN += 5;
                }
                if (clubRef.inst[1] == INSTR_TREATMENT_SOFT) {
                    // obchodzenie sie delikatne
                    sumaO -= 5;
                    sumaP -= 5;
                    sumaN -= 5;
                }

                if (clubRef.inst[5] == INSTR_ATTIT_DEFENSIVE) {
                    //nastawienie obronne
                    sumaO += 10;
                    sumaN -= 10;
                }
                if (clubRef.inst[5] == INSTR_ATTIT_ATTACK) {
                    //nastawienie atak
                    sumaO -= 10;
                    sumaN += 10;
                }

                sumaO += clubRef.trained[3];
                sumaP += clubRef.trained[3];
                sumaN += clubRef.trained[3];

                //taktyka
                if (clubRef.teamSetting == T4_4_2_DEF || clubRef.teamSetting == T3_5_2_DEF || clubRef.teamSetting == T5_3_2_DEF) {
                    //ustawienie obronne
                    sumaO += 10;
                    sumaN -= 10;
                }
                if (clubRef.teamSetting == T4_4_2_ATT || clubRef.teamSetting == T3_5_2_ATT || clubRef.teamSetting == T5_3_2_ATT) {
                    //ustawienie atak
                    sumaO -= 10;
                    sumaN += 10;
                }
                if (clubRef.teamSetting == T4_4_2_DIA) {
                    //ustawienie diamond
                    sumaO += 10;
                    sumaN += 10;
                    sumaP -= 15;
                }
                sumaN += clubRef.trained[2] * 2; // cos wieksza atak, ale tylko dla gracza
            }
            else {
                sumaB2 = sumaBx;
                sumaO2 = sumaOx;
                sumaP2 = sumaPx;
                sumaN2 = sumaNx;
                sumaN2 += premia / 1000;
                sumaP2 += premia / 1000;

                if (clubRef.rivalData[1] == 1) { // rywal gra u siebie
                    sumaB2 += 5;
                    sumaO2 += 10;
                    sumaP2 += 10;
                    sumaN2 += 10;
                }
                if (clubRef.rivalInst[1] == INSTR_TREATMENT_HARD) {
                    sumaO2 += 5;
                    sumaP2 += 5;
                    sumaN2 += 5;
                }
                if (clubRef.rivalInst[1] == INSTR_TREATMENT_SOFT) {
                    sumaO2 -= 5;
                    sumaP2 -= 5;
                    sumaN2 -= 5;
                }
                if (clubRef.rivalInst[5] == INSTR_ATTIT_DEFENSIVE) {
                    //nastawienie obronne
                    sumaO2 += 10;
                    sumaN2 -= 10;
                }
                if (clubRef.rivalInst[5] == INSTR_ATTIT_ATTACK) {
                    //nastawienie atak
                    sumaO2 -= 10;
                    sumaN2 += 10;
                }
                if (clubRef.rivalData[2] == T4_4_2_DEF || clubRef.rivalData[2] == T3_5_2_DEF || clubRef.rivalData[2] == T5_3_2_DEF) {
                    //ustawienie obronne
                    sumaO2 += 10;
                    sumaN2 -= 10;
                }
                if (clubRef.rivalData[2] == T4_4_2_ATT || clubRef.rivalData[2] == T3_5_2_ATT || clubRef.rivalData[2] == T5_3_2_ATT) {
                    //ustawienie atak
                    sumaO2 -= 10;
                    sumaN2 += 10;
                }
            }
        }//dla while k!=2

        OnaA = sumaO - sumaN2;
        PnaP = sumaP - sumaP2;
        AnaO = sumaN - sumaO2;
        if (dystans == 1) {
            sumaB += 50;
            sumaB2 += 50;
            dystans = 0;
        }

        if (clubRef.rivalData[1] == 0) { // gracz gra w domu
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
                gdzie,
                pilka
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
                gdzie,
                pilka
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
            co = whatHappened(pilka, PnaP, OnaA, AnaO, sumaB, sumaB2, gdzie, clubRef);
            //************ gdzie = 1 *******************
            if (co == 9) {
                dystans = 1;
            }

            if (gdzie == 1) {
                if (pilka == 1) {
                    los = (rand() % 3) + 2;
                    msgFootballers[0] = msgPlayerSurnames[los - 1];
                    msgFootballers[1] = msgPlayerSurnames[6]; //czyli 7
                }
                else { //if (pilka==2)
                    los = (rand() % 3) + 2;
                    msgFootballers[0] = msgRivalSurnames[los - 1];
                    msgFootballers[1] = msgRivalSurnames[6]; //czyli 7
                }

                if (co == 1) {
                    wiado[0] = 8;
                    los = (rand() % 3); //blokada=1;
                    msgWhoBall[0] = pilka;
                    msgWhoBall[1] = pilka;
                    if (pilka == 1) {
                        x1 = clubRef.teamSetting;
                        msgFootballers[2] = msgPlayerSurnames[6];
                    }
                    else {
                        x1 = clubRef.rivalData[2];
                        msgFootballers[2] = msgRivalSurnames[6];
                    }

                    if (los == 0) { //na lewe skrzydło
                        wiado[1] = 9;
                        gdzie = 2;
                        if (x1 == 1 || x1 == 2 || x1 == 3 || x1 == 4 || x1 == 10 || x1 == 11) {
                            if (pilka == 1) msgFootballers[3] = msgPlayerSurnames[5]; //czyli 6
                            else msgFootballers[3] = msgRivalSurnames[5];
                            pam = 5;
                        }
                        if (x1 == 5 || x1 == 6 || x1 == 7 || x1 == 8 || x1 == 12 || x1 == 13 || x1 == 14) {
                            if (pilka == 1) msgFootballers[3] = msgPlayerSurnames[4];
                            else msgFootballers[3] = msgRivalSurnames[4];
                            pam = 4;
                        }
                        if (x1 == 9) {
                            if (pilka == 1) msgFootballers[3] = msgPlayerSurnames[7];
                            else msgFootballers[3] = msgRivalSurnames[7];
                            pam = 7;
                        }
                    }
                    else { //if (los==1)//podanie na prawe skrzydło
                        wiado[1] = 10;
                        gdzie = 2;
                        if (x1 == 1 || x1 == 2 || x1 == 3 || x1 == 6 || x1 == 7 || x1 == 9) {
                            if (pilka == 1) msgFootballers[3] = msgPlayerSurnames[8]; // 9
                            else msgFootballers[3] = msgRivalSurnames[8];
                            pam = 8;
                        }

                        if (x1 == 4 || x1 == 5 || x1 == 8 || x1 == 10) {
                            if (pilka == 1) msgFootballers[3] = msgPlayerSurnames[7]; //8
                            else msgFootballers[3] = msgRivalSurnames[7];
                            pam = 7;
                        }

                        if (x1 == 11) {
                            if (pilka == 1) msgFootballers[3] = msgPlayerSurnames[9]; //10
                            else msgFootballers[3] = msgRivalSurnames[9];
                            pam = 9;
                        }

                        if (x1 == 12 || x1 == 13 || x1 == 14) {
                            if (pilka == 1) msgFootballers[3] = msgPlayerSurnames[5]; // 6
                            else msgFootballers[3] = msgRivalSurnames[5];
                            pam = 5;
                        }
                    }

                    if (los == 2) {
                        wiado[1] = 11;
                        gdzie = 3;
                        if (pilka == 1) msgFootballers[2] = msgPlayerSurnames[6];
                        else msgFootballers[2] = msgRivalSurnames[6];
                    }
                }//co=1
                else if (co == 2) {
                    wiado[0] = 12;
                    gdzie = 3;
                    msgWhoBall[0] = pilka;
                }//co=2
                else if (co == 3 || co == 4) {
                    wiado[0] = 12;
                    msgWhoBall[0] = pilka;
                    if (co == 4) {
                        wiado[0] = 8;
                    }
                    wiado[1] = 13;
                    if (co == 4) {
                        wiado[1] = 14;
                    }

                    if (pilka == 1) {
                        msgWhoBall[1] = 2;
                        msgFootballers[2] = msgRivalSurnames[6];
                        msgFootballers[3] = pClub->getClubName(clubRef.rivalData[0] - 1);
                        pilka = 2;
                    }
                    else {
                        msgWhoBall[1] = 1;
                        msgFootballers[2] = msgPlayerSurnames[6];
                        msgFootballers[3] = pClub->getClubName(clubRef.clubId - 1);
                        pilka = 1;
                    }
                    gdzie = 1;
                }//co=3||co=4
                else if (co == 5) {
                    wiado[0] = 8;
                    msgWhoBall[0] = pilka;
                    wiado[1] = 15;
                    msgWhoBall[1] = pilka;
                    wiado[2] = 16;
                    if (pilka == 1) {
                        msgWhoBall[2] = 2;
                        msgFootballers[4] = msgRivalSurnames[6];
                        pilka = 2;
                    }
                    else {
                        msgWhoBall[2] = 1;
                        msgFootballers[4] = msgPlayerSurnames[6];
                        pilka = 1;
                    }
                    gdzie = 1;
                }//co=5
                else if (co == 6) {
                    wiado[0] = 8;
                    msgWhoBall[0] = pilka;
                    wiado[1] = 17;
                    wiado[2] = 18;
                    if (pilka == 1) {
                        msgWhoBall[1] = 2;
                        msgWhoBall[2] = 2;
                        msgFootballers[2] = msgRivalSurnames[6];
                        msgFootballers[3] = msgFootballers[1];
                        msgFootballers[4] = msgRivalSurnames[6];
                        pilka = 2;
                    }
                    else {
                        msgWhoBall[1] = 1;
                        msgWhoBall[2] = 1;
                        msgFootballers[2] = msgPlayerSurnames[6];
                        msgFootballers[3] = msgFootballers[1];
                        msgFootballers[4] = msgPlayerSurnames[6];
                        pilka = 1;
                    }
                    gdzie = 1;
                }//co=6
                else if (co == 7) { //faul przeciwnika
                    wiado[0] = 8;
                    msgWhoBall[0] = pilka;
                    isPossibleGoToTactics = true;
                    wiado[1] = 14;
                    wiado[2] = 19;
                    if (pilka == 1) {
                        x1 = clubRef.rivalInst[1];
                        x2 = clubRef.rivalData[2];
                        str2[7]++;
                    }
                    else {
                        x1 = clubRef.inst[1];
                        x2 = clubRef.teamSetting;
                        str[7]++;
                    }
                    los = (rand() % 6);
                    if (x1 == 2) { //delikatne
                        if (los == 0 || los == 1 || los == 2) co = 1; //nie ma kartki
                        else if (los == 3 || los == 4) co = 2; //słowne upomienie
                        else co = 3; //żólta kartka
                    }
                    else if (x1 == 1) { //normalne
                        if (los == 0 || los == 1) co = 1; //nie ma kartki
                        else if (los == 3 || los == 4) co = 2; //słowne upomienie
                        else co = 3; //żólta kartka if (los==5||los==2)
                    }
                    else if (x1 == 3) { //twarde
                        if (los == 0) co = 1; //nie ma kartki
                        else if (los == 3 || los == 4) co = 2; //słowne upomienie
                        else co = 3; //żólta kartka if (los==5||los==2||los==1)
                    }

                    // tu chyba losuje zawodnika, ktory otrzyma zolta kartke
                    if (x2 == 1 || x2 == 2 || x2 == 3 || x2 == 4) los = (rand() % 4) + 6;
                    else if (x2 == 5) los = (rand() % 4) + 5;
                    else if (x2 == 6 || x2 == 7 || x2 == 8) los = (rand() % 5) + 5;
                    else if (x2 == 9) los = (rand() % 2) + 6;
                    else if (x2 == 10) los = (rand() % 3) + 6;
                    else if (x2 == 11) los = (rand() % 5) + 6;
                    else if (x2 == 12 || x2 == 13 || x2 == 14) los = (rand() % 3) + 7;

                    if (pilka == 1) {
                        msgFootballers[2] = msgRivalSurnames[los - 1];
                        msgWhoBall[1] = 2;
                        msgWhoBall[2] = 2;
                    }
                    else {
                        msgFootballers[2] = msgPlayerSurnames[los - 1];
                        msgWhoBall[1] = 1;
                        msgWhoBall[2] = 1;
                    }

                    if (co == 3) { //jest żółta
                        vector<SFootballer> &tmpFootballers = (pilka == 2)
                            ? pFootballers->getPlayersTeam()
                            : pFootballers->getRivals();
                        int clubId = (pilka == 2) ? clubRef.clubId : clubRef.rivalData[0];

                        for (size_t index = 0; index < tmpFootballers.size(); index++) {
                            SFootballer &footballer = tmpFootballers[index];
                            if (footballer.data[0] == los && clubId == footballer.data[22]) {
                                footballer.data[12]++; // zolta kartka +1
                                footballer.data[7]--; // moral -1
                                if (footballer.data[7] < -3) {
                                    footballer.data[7] = -3;
                                }
                            }
                        }

                        if (pilka == 2) {
                            pFootballers->savePlayerTeam();
                        }
                        else {
                            pFootballers->saveRivals();
                        }
                        k = 0;

                        for (size_t index = 0; index < tmpFootballers.size(); index++) {
                            SFootballer &footballer = tmpFootballers[index];
                            if (footballer.data[0] == los && clubId == footballer.data[22] && footballer.data[12] >= 2) {
                                k = 1;
                            }
                        }

                        if (k == 1) {
                            wiado[3] = 60;
                            if (pilka == 1) {
                                str2[5]++;
                                str2[4]++;
                            }
                            else {
                                str[5]++;
                                str[4]++;
                            }
                        }
                        else {
                            wiado[3] = 20;
                            if (pilka == 1) str2[4]++;
                            else str[4]++;
                        }

                        if (pilka == 1) {
                            msgFootballers[6] = msgRivalSurnames[los - 1];
                            msgWhoBall[3] = 2;
                        }
                        else {
                            msgFootballers[6] = msgPlayerSurnames[los - 1];
                            msgWhoBall[3] = 1;
                        }
                    }// jest żółta end
                    else if (co == 2) { //słownie
                        wiado[3] = 21;
                        if (pilka == 1) {
                            msgFootballers[6] = msgRivalSurnames[los - 1];
                            msgWhoBall[3] = 2;
                        }
                        else {
                            msgFootballers[6] = msgPlayerSurnames[los - 1];
                            msgWhoBall[3] = 1;
                        }
                    }//słownie end
                    wiado[4] = 22;
                    msgWhoBall[4] = pilka;
                    msgWhoBall[5] = pilka;
                    los = (rand() % 3);
                    if (pilka == 1) {
                        x1 = clubRef.teamSetting;
                        msgFootballers[8] = msgPlayerSurnames[6];
                        msgFootballers[10] = msgPlayerSurnames[6];
                    }
                    else {
                        x1 = clubRef.rivalData[2];
                        msgFootballers[8] = msgRivalSurnames[6];
                        msgFootballers[10] = msgRivalSurnames[6];
                    }

                    if (los == 0) { //na lewe skrzydło
                        wiado[5] = 9;
                        gdzie = 2;
                        if (x1 == 1 || x1 == 2 || x1 == 3 || x1 == 4 || x1 == 10 || x1 == 11) {
                            if (pilka == 1) msgFootballers[11] = msgPlayerSurnames[5]; //czyli 6
                            else msgFootballers[11] = msgRivalSurnames[5];
                            pam = 5;
                        }

                        if (x1 == 5 || x1 == 6 || x1 == 7 || x1 == 8 || x1 == 12 || x1 == 13 || x1 == 14) {
                            if (pilka == 1) msgFootballers[11] = msgPlayerSurnames[4];
                            else msgFootballers[11] = msgRivalSurnames[4];
                            pam = 4;
                        }

                        if (x1 == 9) {
                            if (pilka == 1) msgFootballers[11] = msgPlayerSurnames[7];
                            else msgFootballers[11] = msgRivalSurnames[7];
                            pam = 7;
                        }
                    }
                    else if (los == 1) { //podanie na prawe skrzydło
                        wiado[5] = 10;
                        gdzie = 2;
                        if (x1 == 1 || x1 == 2 || x1 == 3 || x1 == 6 || x1 == 7 || x1 == 9) {
                            if (pilka == 1) msgFootballers[11] = msgPlayerSurnames[8]; // 9
                            else msgFootballers[11] = msgRivalSurnames[8];
                            pam = 8;
                        }

                        if (x1 == 4 || x1 == 5 || x1 == 8 || x1 == 10) {
                            if (pilka == 1) msgFootballers[11] = msgPlayerSurnames[7]; //8
                            else msgFootballers[11] = msgRivalSurnames[7];
                            pam = 7;
                        }

                        if (x1 == 11) {
                            if (pilka == 1) msgFootballers[11] = msgPlayerSurnames[9]; //10
                            else msgFootballers[11] = msgRivalSurnames[9];
                            pam = 9;
                        }

                        if (x1 == 12 || x1 == 13 || x1 == 14) {
                            if (pilka == 1) msgFootballers[11] = msgPlayerSurnames[5]; // 6
                            else msgFootballers[11] = msgRivalSurnames[5];
                            pam = 5;
                        }
                    }
                    else if (los == 2) {
                        wiado[5] = 11;
                        gdzie = 3;
                    }
                }//co=7
                else if (co == 8) { //aut dla przeciwnika
                    wiado[0] = 8;
                    msgWhoBall[0] = pilka;
                    isPossibleGoToTactics = true;
                    wiado[1] = 23;
                    msgWhoBall[1] = pilka;
                    wiado[2] = (rand() % 2) == 0 ? 24 : 25;

                    los = (rand() % 3) + 2;
                    if (pilka == 1) {
                        msgFootballers[4] = msgRivalSurnames[los - 1];
                        pilka = 2;
                    }
                    else {
                        msgFootballers[4] = msgPlayerSurnames[los - 1];
                        pilka = 1;
                    }
                    gdzie = 1;
                    msgWhoBall[2] = pilka;
                }//co=8
                else if (co == 9) { //strzał z dystansu
                    wiado[0] = 8;
                    msgWhoBall[0] = pilka;
                    msgWhoBall[2] = pilka;
                    wiado[1] = 26;
                    msgWhoBall[1] = pilka;
                    wiado[2] = (rand() % 2) == 0 ? 27 : 28;

                    if (pilka == 1) {
                        str[0]++;
                        x1 = clubRef.teamSetting;
                    }
                    else {
                        str2[0]++;
                        x1 = clubRef.rivalData[2];
                    }

                    if (x1 == 6 || x1 == 7 || x1 == 8)          los = (rand() % 5) + 5;
                    else if (x1 == 11)                          los = (rand() % 5) + 6;
                    else if (x1 == 12 || x1 == 13 || x1 == 14)  los = (rand() % 3) + 7;
                    else if (x1 == 5)                           los = (rand() % 4) + 5;
                    else if (x1 == 10)                          los = (rand() % 3) + 6;
                    else                                        los = (rand() % 4) + 6;

                    msgFootballers[1] = pilka == 1 ? msgPlayerSurnames[los - 1] : msgRivalSurnames[los - 1];

                    msgFootballers[2] = msgFootballers[1];
                    msgFootballers[4] = msgFootballers[2];
                    gdzie = 4;
                    pam = los - 1;
                }//co=9
            }////dla blokada==1 gdzie==1
                //************** Skrzydłowy przy piłce ******************
            else if (gdzie == 2) {
                wiado[0] = 29;
                msgWhoBall[0] = pilka;
                msgFootballers[0] = pilka == 1 ? msgPlayerSurnames[pam] : msgRivalSurnames[pam];

                if (co == 10) { //udane dośrodkowanie
                    wiado[1] = 30;
                    msgWhoBall[1] = pilka;
                    msgFootballers[2] = msgFootballers[0];

                    vector<SFootballer> &tmpFootballers = (pilka == 1)
                        ? pFootballers->getPlayersTeam()
                        : pFootballers->getRivals();
                    int clubId = (pilka == 1) ? clubRef.clubId : clubRef.rivalData[0];
                    msgFootballers[3] = (pilka == 1) ? msgRivalSurnames[3] : msgPlayerSurnames[3];

                    for (size_t index = 0; index < tmpFootballers.size(); index++) {
                        SFootballer &footballer = tmpFootballers[index];
                        if (footballer.data[0] == pam + 1 && footballer.data[22] == clubId) {
                            footballer.data[20]++;
                        }
                    }

                    if (pilka == 1) {
                        pFootballers->savePlayerTeam();
                    }
                    else {
                        pFootballers->saveRivals();
                    }

                    wiado[2] = (rand() % 2) == 0 ? 31 : 32;

                    msgWhoBall[2] = pilka;
                    gdzie = 3;
                }
                else if (co == 11) { //faul
                    vector<SFootballer> &tmpFootballers = (pilka == 1)
                        ? pFootballers->getPlayersTeam()
                        : pFootballers->getRivals();
                    int clubId = (pilka == 1) ? clubRef.clubId : clubRef.rivalData[0];
                    msgFootballers[3] = (pilka == 1) ? msgRivalSurnames[3] : msgPlayerSurnames[3];

                    for (size_t index = 0; index < tmpFootballers.size(); index++) {
                        SFootballer &footballer = tmpFootballers[index];
                        if (footballer.data[0] == pam + 1 && footballer.data[22] == clubId) {
                            footballer.data[20]++;
                        }
                    }

                    if (pilka == 1) {
                        pFootballers->savePlayerTeam();
                    }
                    else {
                        pFootballers->saveRivals();
                    }

                    wiado[1] = 30;
                    msgWhoBall[1] = pilka;
                    msgFootballers[2] = msgFootballers[0];
                    isPossibleGoToTactics = true;

                    if (pilka == 1) {
                        x1 = clubRef.rivalData[2];
                        str2[7]++;
                    }
                    else {
                        x1 = clubRef.teamSetting;
                        str[7]++;
                    }

                    if (x1 == 5 || x1 == 6 || x1 == 7 || x1 == 8) {
                        los = (rand() % 3) + 2;
                    }
                    else if (x1 == 12 || x1 == 13 || x1 == 14) {
                        los = (rand() % 5) + 2;
                    }
                    else {
                        los = (rand() % 4) + 2;
                    }

                    msgFootballers[3] = pilka == 1 ? msgRivalSurnames[los - 1] : msgPlayerSurnames[los - 1];

                    wiado[2] = 33;
                    msgFootballers[4] = msgFootballers[3];
                    msgFootballers[5] = msgFootballers[2];
                    x2 = (rand() % 6);
                    if (pilka == 1) {
                        msgWhoBall[2] = 2;
                        msgWhoBall[3] = 2;
                        x1 = clubRef.rivalInst[1];
                    }
                    else {
                        msgWhoBall[2] = 1;
                        msgWhoBall[3] = 1;
                        x1 = clubRef.inst[1];
                    }

                    if (x1 == 2) { //delikatne
                        if (x2 == 0 || x2 == 1 || x2 == 2) {
                            co = 1; //nie ma kartki
                        }
                        else if (x2 == 3 || x2 == 4) {
                            co = 2; //słowne upomienie
                        }
                        else {
                            co = 3; //żólta kartka
                        }
                    }
                    else if (x1 == 1) { //normalne
                        if (x2 == 0 || x2 == 1) {
                            co = 1; //nie ma kartki
                        }
                        else if (x2 == 3 || x2 == 4) {
                            co = 2; //słowne upomienie
                        }
                        else {
                            co = 3; //żólta kartka
                        }
                    }
                    else if (x1 == 3) { //twarde
                        if (x2 == 0) {
                            co = 1; //nie ma kartki
                        }
                        else if (x2 == 3 || x2 == 4) {
                            co = 2; //słowne upomienie
                        }
                        else {
                            co = 3; //żólta kartka
                        }
                    }

                    if (co == 3) { //jest żółta
                        vector<SFootballer> &tmpFootballers = (pilka == 2)
                            ? pFootballers->getPlayersTeam()
                            : pFootballers->getRivals();
                        int clubId = (pilka == 2) ? clubRef.clubId : clubRef.rivalData[0];

                        for (size_t index = 0; index < tmpFootballers.size(); index++) {
                            SFootballer &footballer = tmpFootballers[index];
                            if (footballer.data[0] == los && clubId == footballer.data[22]) {
                                footballer.data[12]++;
                                footballer.data[7]--;
                                if (footballer.data[7] < -3) {
                                    footballer.data[7] = -3;
                                }
                            }
                        }

                        if (pilka == 2) {
                            pFootballers->savePlayerTeam();
                        }
                        else {
                            pFootballers->saveRivals();
                        }

                        k = 0;

                        for (size_t index = 0; index < tmpFootballers.size(); index++) {
                            SFootballer &footballer = tmpFootballers[index];
                            if (footballer.data[0] == los && clubId == footballer.data[22] && footballer.data[12] >= 2) {
                                k = 1;
                            }
                        }

                        msgFootballers[6] = msgFootballers[3];
                        if (k == 0) {
                            wiado[3] = 20;
                            if (pilka == 1) {
                                str2[4]++;
                            }
                            else {
                                str[4]++;
                            }
                        }
                        else if (k == 1) {
                            wiado[3] = 60;
                            if (pilka == 1) {
                                str2[5]++;
                                str2[4]++;
                            }
                            else {
                                str[5]++;
                                str[4]++;
                            }
                        }
                    }// jest żółta end
                    else if (co == 2) { //słownie
                        wiado[3] = 21;
                        msgFootballers[6] = msgFootballers[3];
                        if (pilka == 1) {
                            msgWhoBall[3] = 2;
                        }
                        else {
                            msgWhoBall[3] = 1;
                        }
                    }//słownie end
                    los = (rand() % 10) + 2;
                    wiado[4] = 85;
                    msgWhoBall[4] = pilka;
                    if (pilka == 1) {
                        msgFootballers[8] = msgPlayerSurnames[los - 1];
                        str[0]++;
                    }
                    else {
                        msgFootballers[8] = msgRivalSurnames[los - 1];
                        str2[0]++;
                    }
                    pam = los - 1;
                    wiado[5] = 63;
                    msgWhoBall[5] = pilka;
                    wiado[6] = 64;
                    msgWhoBall[6] = pilka;
                    msgFootballers[12] = msgFootballers[8];
                    gdzie = 6;
                }//end faul
                else if (co == 12) { //aut
                    wiado[1] = 34;
                    msgWhoBall[1] = pilka;
                    isPossibleGoToTactics = true;
                    los = (rand() % 2);
                    if (los == 0) {
                        wiado[2] = 24;
                    }
                    else {
                        wiado[2] = 25;
                    }

                    los = (rand() % 3) + 2;
                    if (pilka == 1) {
                        msgFootballers[4] = msgRivalSurnames[los - 1];
                        pilka = 2;
                        msgWhoBall[2] = 2;
                    }
                    else {
                        msgFootballers[4] = msgPlayerSurnames[los - 1];
                        pilka = 1;
                        msgWhoBall[2] = 1;
                    }
                    gdzie = 1;
                }
                else if (co == 13) { //nie dośrodkował
                    wiado[1] = 35;
                    los = (rand() % 3) + 2;
                    if (pilka == 1) {
                        pilka = 2;
                        msgFootballers[2] = msgRivalSurnames[los - 1];
                        msgWhoBall[1] = 2;
                        msgWhoBall[2] = 2;
                        x1 = clubRef.rivalInst[0];
                    }
                    else {
                        pilka = 1;
                        msgFootballers[2] = msgPlayerSurnames[los - 1];
                        msgWhoBall[1] = 1;
                        msgWhoBall[2] = 1;
                        x1 = clubRef.inst[0];
                    }

                    vector<SFootballer> &tmpFootballers = (pilka == 2)
                        ? pFootballers->getPlayersTeam()
                        : pFootballers->getRivals();
                    int clubId = (pilka == 2) ? clubRef.clubId : clubRef.rivalData[0];

                    for (size_t index = 0; index < tmpFootballers.size(); index++) {
                        SFootballer &footballer = tmpFootballers[index];
                        if (footballer.data[0] == pam + 1 && footballer.data[22] == clubId) {
                            footballer.data[20]--;
                        }
                    }

                    if (pilka == 2) {
                        pFootballers->savePlayerTeam();
                    }
                    else {
                        pFootballers->saveRivals();
                    }

                    msgFootballers[4] = msgFootballers[2];
                    los = (rand() % 6);
                    if (x1 == 4) { //długie podania
                        if (los == 1 || los == 2 || los == 3 || los == 4 || los == 5) {
                            wiado[2] = 36; //gdzie=3;
                        }
                        else {
                            wiado[2] = 11; //gdzie=1;
                        }
                    }
                    else if (x1 == 1 || x1 == 3) { //mieszane, średnie
                        if (los == 1 || los == 2 || los == 5) {
                            wiado[2] = 36; //gdzie=3;
                        }
                        else {
                            wiado[2] = 11; //gdzie=1;
                        }
                    }
                    else { //if (x1==2) //któtkie podania
                        if (los == 1 || los == 2 || los == 3 || los == 4 || los == 5) {
                            wiado[2] = 11; //gdzie=1;
                        }
                        else {
                            wiado[2] = 36; //gdzie=3;
                        }
                    }

                    gdzie = wiado[2] == 11 ? 1 : 3;
                }//nie dośrodkował
            }//dla skrzydłowy przy piłce
            else if (gdzie == 3) { //****************** Pole karne ********
                if (co == 14) { //spalony
                    wiado[0] = 37;
                    msgWhoBall[0] = pilka;
                    isPossibleGoToTactics = true;
                    msgFootballers[0] = pilka == 1 ? msgPlayerSurnames[10] : msgRivalSurnames[10];

                    los = (rand() % 3);
                    if (los == 0) {
                        wiado[1] = 38;
                    }
                    else if (los == 1) {
                        wiado[1] = 39;
                    }
                    else {
                        wiado[1] = 40;
                    }

                    msgFootballers[2] = msgFootballers[0];
                    msgWhoBall[1] = pilka;
                    wiado[2] = 41;
                    if (pilka == 1) {
                        pilka = 2;
                        msgFootballers[4] = msgRivalSurnames[0];
                        msgWhoBall[2] = 2;
                        str[3]++;
                    }
                    else {
                        pilka = 1;
                        msgFootballers[4] = msgPlayerSurnames[0];
                        msgWhoBall[2] = 1;
                        str2[3]++;
                    }
                    gdzie = 1;
                }//spalony
                else if (co == 15) { //spalony nieudany
                    wiado[0] = 37;
                    msgWhoBall[0] = pilka;
                    wiado[1] = 42;
                    msgWhoBall[1] = pilka;
                    wiado[2] = 43;
                    msgWhoBall[2] = pilka;

                    x1 = pilka == 1 ? clubRef.teamSetting : clubRef.rivalData[2];

                    if (x1 == T4_5_1) {
                        los = 11;
                    }
                    else if (x1 == T3_4_3 || x1 == T4_3_3) {
                        los = (rand() % 3) + 9;
                    }
                    else {
                        los = (rand() % 2) + 10;
                    }

                    if (pilka == 1) {
                        msgFootballers[0] = msgPlayerSurnames[los - 1];
                        str[0]++;
                    }
                    else {
                        msgFootballers[0] = msgRivalSurnames[los - 1];
                        str2[0]++;
                    }
                    pam = los - 1;
                    msgFootballers[2] = msgFootballers[0];
                    msgFootballers[4] = msgFootballers[0];
                    gdzie = 5;
                }//sam na sam
                else if (co == 16) { //obrona wykopuje
                    if (pilka == 1) {
                        x1 = clubRef.rivalInst[0];
                    }
                    else {
                        x1 = clubRef.inst[0];
                    }

                    los = (rand() % 6);
                    x2 = (rand() % 2);
                    if (x1 == 4) { //długie
                        if (los == 0) {
                            wiado[0] = x2 == 0 ? 44 : 45;
                            gdzie = 1;
                        }
                        else {
                            gdzie = 3;
                            wiado[0] = x2 == 0 ? 46 : 47;
                        }
                    }
                    else if (x1 == 2) { //krótkie
                        if (los == 0) {
                            gdzie = 3;
                            wiado[0] = x2 == 0 ? 46 : 47;
                        }
                        else {
                            gdzie = 1;
                            wiado[0] = x2 == 0 ? 44 : 45;
                        }
                    }
                    else { //pozostałe
                        if (los == 0 || los == 1 || los == 2) {
                            gdzie = 3;
                            wiado[0] = x2 == 0 ? 46 : 47;
                        }
                        else {
                            gdzie = 1;
                            wiado[0] = x2 == 0 ? 44 : 45;
                        }
                    }
                    los = (rand() % 3) + 2;
                    if (pilka == 1) {
                        pilka = 2;
                        msgFootballers[0] = msgRivalSurnames[los - 1];
                        msgWhoBall[0] = 2;
                    }
                    else {
                        pilka = 1;
                        msgFootballers[0] = msgPlayerSurnames[los - 1];
                        msgWhoBall[0] = 1;
                    }
                    //gdzie=1;
                }//obrona wykopuje
                else if (co == 17) { //B łapie
                    wiado[0] = 48;
                    wiado[1] = 49;
                    if (pilka == 1) {
                        pilka = 2;
                        msgFootballers[0] = msgRivalSurnames[0];
                        msgWhoBall[0] = 2;
                        msgWhoBall[1] = 2;
                    }
                    else {
                        pilka = 1;
                        msgFootballers[0] = msgPlayerSurnames[0];
                        msgWhoBall[0] = 1;
                        msgWhoBall[1] = 1;
                    }
                    gdzie = 1;
                }//B łapie
                else if (co == 18) { //karny
                    wiado[0] = 37;
                    msgWhoBall[0] = pilka;
                    wiado[1] = 50;
                    if (pilka == 1) {
                        msgFootballers[0] = msgPlayerSurnames[10];
                        msgWhoBall[1] = 2;
                        x1 = clubRef.rivalData[2];
                        str2[7]++;
                        str[6]++;
                    }
                    else {
                        msgFootballers[0] = msgRivalSurnames[10];
                        msgWhoBall[1] = 1;
                        x1 = clubRef.teamSetting;
                        str[7]++;
                        str2[6]++;
                    }

                    if (x1 == T4_4_2 || x1 == T4_4_2_DEF || x1 == T4_4_2_ATT || x1 == T4_4_2_DIA ||
                        x1 == T4_2_4 || x1 == T4_3_3 || x1 == T4_5_1
                    ) {
                        los = (rand() % 4) + 2;
                    }
                    else if (x1 == T3_4_3 || x1 == T3_5_2 || x1 == T3_5_2_DEF || x1 == T3_5_2_ATT) {
                        los = (rand() % 3) + 2;
                    }
                    else {
                        los = (rand() % 5) + 2;
                    }

                    msgFootballers[3] = msgFootballers[0];
                    msgFootballers[2] = pilka == 1 ? msgRivalSurnames[los - 1] : msgPlayerSurnames[los - 1];

                    x1 = (rand() % 2);
                    wiado[2] = x1 == 0 ? 51 : 52;

                    x2 = (rand() % 6);
                    if (pilka == 1) {
                        msgWhoBall[2] = 2;
                        msgWhoBall[3] = 2;
                        x1 = clubRef.rivalInst[1];
                        str[0]++;
                    }
                    else {
                        msgWhoBall[2] = 1;
                        msgWhoBall[3] = 1;
                        x1 = clubRef.inst[1];
                        str2[0]++;
                    }

                    if (x1 == INSTR_TREATMENT_SOFT) { //delikatne
                        if (x2 == 0 || x2 == 1 || x2 == 2) {
                            co = 1; //nie ma kartki
                        }
                        else if (x2 == 3 || x2 == 4) {
                            co = 2; //słowne upomienie
                        }
                        else {
                            co = 3; //żólta kartka
                        }
                    }
                    else if (x1 == INSTR_TREATMENT_NORMAL) { //normalne
                        if (x2 == 0 || x2 == 1) {
                            co = 1; //nie ma kartki
                        }
                        else if (x2 == 3 || x2 == 4) {
                            co = 2; //słowne upomienie
                        }
                        else {
                            co = 3; //żólta kartka
                        }
                    }
                    else if (x1 == INSTR_TREATMENT_HARD) { //twarde
                        if (x2 == 0) {
                            co = 1; //nie ma kartki
                        }
                        else if (x2 == 3 || x2 == 4) {
                            co = 2; //słowne upomienie
                        }
                        else {
                            co = 3; //żólta kartka
                        }
                    }

                    if (co == 3) { //jest żółta
                        vector<SFootballer> &tmpFootballers = (pilka == 2)
                            ? pFootballers->getPlayersTeam()
                            : pFootballers->getRivals();
                        int clubId = (pilka == 2) ? clubRef.clubId : clubRef.rivalData[0];

                        for (size_t index = 0; index < tmpFootballers.size(); index++) {
                            SFootballer &footballer = tmpFootballers[index];
                            if (footballer.data[0] == los && clubId == footballer.data[22]) {
                                footballer.data[12]++;
                                footballer.data[7]--;
                                if (footballer.data[7] < -3) {
                                    footballer.data[7] = -3;
                                }
                            }
                        }

                        if (pilka == 2) {
                            pFootballers->savePlayerTeam();
                        }
                        else {
                            pFootballers->saveRivals();
                        }

                        k = 0;
                        for (size_t index = 0; index < tmpFootballers.size(); index++) {
                            SFootballer &footballer = tmpFootballers[index];
                            if (footballer.data[0] == los &&
                                clubId == footballer.data[22] &&
                                footballer.data[12] >= 2
                            ) {
                                k = 1;
                            }
                        }

                        msgFootballers[6] = msgFootballers[2];
                        if (k == 0) {
                            wiado[3] = 20;
                            if (pilka == 1) {
                                str2[4]++;
                            }
                            else {
                                str[4]++;
                            }
                        }
                        else if (k == 1) {
                            wiado[3] = 60;
                            if (pilka == 1) {
                                str2[5]++;
                                str2[4]++;
                            }
                            else {
                                str[5]++;
                                str[4]++;
                            }
                        }
                    }// jest żółta end
                    else if (co == 2) { //słownie
                        wiado[3] = 21;
                        msgFootballers[6] = msgFootballers[2];
                        msgWhoBall[3] = pilka == 1 ? 2 : 1;
                    }//słownie end
                    x1 = (rand() % 2);
                    wiado[4] = x1 == 0 ? 53 : 54;
                    los = (rand() % 4) + 8;
                    msgFootballers[8] = (pilka == 1) ? msgPlayerSurnames[los - 1] : msgRivalSurnames[los - 1];
                    pam = los - 1;
                    gdzie = 5;
                    msgWhoBall[4] = pilka;
                }//karny
                else if (co == 19) { //symulowany
                    wiado[0] = 37;
                    msgWhoBall[0] = pilka;
                    msgWhoBall[2] = pilka;
                    if (pilka == 1) {
                        msgFootballers[0] = msgPlayerSurnames[10];
                        msgWhoBall[1] = 2;
                        msgWhoBall[3] = 2;
                        x1 = clubRef.rivalData[2];
                    }
                    else {
                        msgFootballers[0] = msgRivalSurnames[10];
                        msgWhoBall[1] = 1;
                        msgWhoBall[3] = 1;
                        x1 = clubRef.teamSetting;
                    }

                    if (x1 == T4_4_2 || x1 == T4_4_2_DEF || x1 == T4_4_2_ATT || x1 == T4_4_2_DIA ||
                        x1 == T4_2_4 || x1 == T4_3_3 || x1 == T4_5_1
                    ) {
                        los = (rand() % 4) + 2;
                    }
                    else if (x1 == T3_4_3 || x1 == T3_5_2 || x1 == T3_5_2_DEF || x1 == T3_5_2_ATT) {
                        los = (rand() % 3) + 2;
                    }
                    else {
                        los = (rand() % 5) + 2;
                    }

                    msgFootballers[3] = msgFootballers[0];
                    msgFootballers[2] = pilka == 1 ? msgRivalSurnames[los - 1] : msgPlayerSurnames[los - 1];
                    wiado[1] = 50;
                    wiado[2] = 55;
                    msgWhoBall[2] = pilka;
                    msgFootballers[4] = msgFootballers[0];
                    los = (rand() % 2);
                    wiado[3] = (los == 0) ? 44 : 45;
                    pilka = (pilka == 1) ? 2 : 1;
                    msgFootballers[6] = msgFootballers[2];
                    msgWhoBall[3] = pilka;
                    gdzie = 1;
                }//faul symulowany
                else if (co == 20) { //róg
                    wiado[0] = 37;
                    msgWhoBall[0] = pilka;
                    msgWhoBall[2] = pilka;
                    msgWhoBall[3] = pilka;
                    isPossibleGoToTactics = true;
                    if (pilka == 1) {
                        msgFootballers[0] = msgPlayerSurnames[10];
                        msgWhoBall[1] = 2;
                        x1 = clubRef.rivalData[2];
                        str[2]++;
                    }
                    else {
                        msgFootballers[0] = msgRivalSurnames[10];
                        msgWhoBall[1] = 1;
                        x1 = clubRef.teamSetting;
                        str2[2]++;
                    }

                    if (x1 == T4_4_2 || x1 == T4_4_2_DEF || x1 == T4_4_2_ATT || x1 == T4_4_2_DIA ||
                        x1 == T4_2_4 || x1 == T4_3_3 || x1 == T4_5_1
                    ) {
                        los = (rand() % 4) + 2;
                    }
                    else if (x1 == T3_4_3 || x1 == T3_5_2 || x1 == T3_5_2_DEF || x1 == T3_5_2_ATT) {
                        los = (rand() % 3) + 2;
                    }
                    else {
                        los = (rand() % 5) + 2;
                    }

                    msgFootballers[3] = msgFootballers[0];
                    msgFootballers[2] = (pilka == 1) ? msgRivalSurnames[los - 1] : msgPlayerSurnames[los - 1];
                    wiado[1] = 50;
                    wiado[2] = 56;
                    wiado[3] = 78;
                    msgFootballers[6] = (pilka == 1) ? msgPlayerSurnames[6] : msgRivalSurnames[6];
                    gdzie = 3;
                }//róg
                else if (co == 21) { //strzał
                    wiado[0] = 37;
                    msgWhoBall[0] = pilka;
                    msgWhoBall[1] = pilka;
                    los = (rand() % 2);
                    wiado[1] = (los == 0) ? 43 : 57;
                    x1 = (pilka == 1) ? clubRef.teamSetting : clubRef.rivalData[2];
                    if (x1 == T3_4_3 || x1 == T4_3_3) {
                        los = (rand() % 3) + 9;
                    }
                    else if (x1 == T4_5_1) {
                        los = 11;
                    }
                    else {
                        los = (rand() % 2) + 10;
                    }

                    if (pilka == 1) {
                        msgFootballers[0] = msgPlayerSurnames[los - 1];
                        str[0]++;
                    }
                    else {
                        msgFootballers[0] = msgRivalSurnames[los - 1];
                        str2[0]++;
                    }
                    msgFootballers[2] = msgFootballers[0];
                    pam = los - 1;
                    gdzie = 4;
                }//strzał
                else if (co == 22) { //podanie i strzał
                    wiado[0] = 37;
                    msgWhoBall[0] = pilka;
                    wiado[1] = 58;
                    msgWhoBall[1] = pilka;
                    msgWhoBall[2] = pilka;
                    los = (rand() % 2);
                    wiado[2] = (los == 0) ? 59 : 57;
                    msgFootballers[0] = (pilka == 1) ? msgPlayerSurnames[10] : msgRivalSurnames[10];
                    los = (rand() % 4) + 7;
                    if (pilka == 1) {
                        msgFootballers[2] = msgPlayerSurnames[los - 1];
                        str[0]++;
                    }
                    else {
                        msgFootballers[2] = msgRivalSurnames[los - 1];
                        str2[0]++;
                    }
                    msgFootballers[4] = msgFootballers[2];
                    pam = los - 1;
                    gdzie = 4;
                }//podanie i strzał
                else if (co == 23) { // faul napastnika
                    wiado[0] = 37;
                    msgWhoBall[0] = pilka;
                    isPossibleGoToTactics = true;
                    if (pilka == 1) {
                        str[7]++;
                        msgFootballers[2] = msgRivalSurnames[1];
                        msgWhoBall[1] = 2;
                        msgWhoBall[2] = 2;
                        x1 = clubRef.inst[1];
                        x2 = clubRef.teamSetting;
                    } else {
                        str2[7]++;
                        msgFootballers[2] = msgPlayerSurnames[1];
                        msgWhoBall[1] = 1;
                        msgWhoBall[2] = 1;
                        x1 = clubRef.rivalInst[1];
                        x2 = clubRef.rivalData[2];
                    }

                    if (x2 == T3_4_3 || x2 == T4_3_3) {
                        los = (rand() % 3) + 9;
                    }
                    else if (x2 == T4_5_1) {
                        los = 11;
                    }
                    else {
                        los = (rand() % 2) + 10;
                    }

                    msgFootballers[0] = (pilka == 1) ? msgPlayerSurnames[los - 1] : msgRivalSurnames[los - 1];
                    msgFootballers[3] = msgFootballers[0];
                    wiado[1] = 50;
                    wiado[2] = 61;
                    wiado[3] = 62;
                    msgWhoBall[3] = pilka;
                    msgFootballers[6] = msgFootballers[0];
                    wiado[4] = 19;
                    msgWhoBall[4] = pilka;
                    x2 = (rand() % 6);
                    if (x1 == INSTR_TREATMENT_SOFT) { //delikatne
                        if (x2 == 0 || x2 == 1 || x2 == 2) {
                            co = 1; //nie ma kartki
                        }
                        else if (x2 == 3 || x2 == 4) {
                            co = 2; //słowne upomienie
                        }
                        else {
                            co = 3; //żólta kartka
                        }
                    }
                    else if (x1 == INSTR_TREATMENT_NORMAL) { //normalne
                        if (x2 == 0 || x2 == 1) {
                            co = 1; //nie ma kartki
                        }
                        else if (x2 == 3 || x2 == 4) {
                            co = 2; //słowne upomienie
                        }
                        else {
                            co = 3; //żólta kartka
                        }
                    }
                    else if (x1 == INSTR_TREATMENT_HARD) { //twarde
                        if (x2 == 0) {
                            co = 1; //nie ma kartki
                        }
                        else if (x2 == 3 || x2 == 4) {
                            co = 2; //słowne upomienie
                        }
                        else {
                            co = 3; //żólta kartka
                        }
                    }

                    if (co == 3) { //jest żółta
                        vector<SFootballer> &tmpFootballers = (pilka == 1)
                            ? pFootballers->getPlayersTeam()
                            : pFootballers->getRivals();
                        int clubId = (pilka == 1) ? clubRef.clubId : clubRef.rivalData[0];

                        for (size_t index = 0; index < tmpFootballers.size(); index++) {
                            SFootballer &footballer = tmpFootballers[index];
                            if (footballer.data[0] == los && clubId == footballer.data[22]) {
                                footballer.data[12]++;
                                footballer.data[7]--;
                                if (footballer.data[7] < -3) {
                                    footballer.data[7] = -3;
                                }
                            }
                        }

                        if (pilka == 1) {
                            pFootballers->savePlayerTeam();
                        }
                        else {
                            pFootballers->saveRivals();
                        }

                        k = 0;
                        for (size_t index = 0; index < tmpFootballers.size(); index++) {
                            SFootballer &footballer = tmpFootballers[index];
                            if (footballer.data[0] == los &&
                                clubId == footballer.data[22] &&
                                footballer.data[12] >= 2
                            ) {
                                k = 1;
                            }
                        }

                        msgFootballers[10] = msgFootballers[0];
                        msgWhoBall[5] = pilka;
                        if (k == 0) {
                            wiado[5] = 20;
                            if (pilka == 1) {
                                str[4]++;
                            }
                            else {
                                str2[4]++;
                            }
                        }
                        else if (k == 1) {
                            wiado[5] = 60;
                            if (pilka == 1) {
                                str[4]++;
                                str[5]++;
                            }
                            else {
                                str2[4]++;
                                str2[5]++;
                            }
                        }
                    }// jest żółta end
                    else if (co == 2) { //słownie
                        wiado[5] = 21;
                        msgFootballers[10] = msgFootballers[3];
                        msgWhoBall[5] = pilka;
                    }//słownie end
                    wiado[6] = 41;
                    if (pilka == 1) {
                        pilka = 2;
                        msgFootballers[12] = msgRivalSurnames[0];
                    }
                    else {
                        pilka = 1;
                        msgFootballers[12] = msgPlayerSurnames[0];
                    }
                    msgWhoBall[6] = pilka;
                    gdzie = 1;
                }//faul napastnika
            }//gdzie=3,blokada=1,pole karne
            //*********************** obron B ***********************************
            else if (gdzie == 4 || gdzie == 6 || gdzie == 5) { //obrona B
                if (co == 24) { // udana
                    wiado[0] = (rand() % 2) == 0 ? 65 : 66;
                    wiado[1] = 67;
                    wiado[2] = 49;
                    if (pilka == 1) {
                        msgFootballers[0] = msgRivalSurnames[0];
                        msgWhoBall[0] = 2;
                        msgWhoBall[1] = 2;
                        pilka = 2;
                        str[1]++;
                    }
                    else {
                        msgFootballers[0] = msgPlayerSurnames[0];
                        msgWhoBall[0] = 1;
                        msgWhoBall[1] = 1;
                        pilka = 1;
                        str2[1]++;
                    }
                    msgFootballers[2] = msgFootballers[0];
                    msgWhoBall[2] = pilka;
                    gdzie = 1;

                    vector<SFootballer> &tmpFootballers = (pilka == 1)
                        ? pFootballers->getPlayersTeam()
                        : pFootballers->getRivals();
                    int clubId = (pilka == 1) ? clubRef.clubId : clubRef.rivalData[0];

                    for (size_t index = 0; index < tmpFootballers.size(); index++) {
                        SFootballer &footballer = tmpFootballers[index];
                        if (footballer.data[0] == 1 && clubId == footballer.data[22]) {
                            footballer.data[20]++;
                            footballer.data[7]++;
                            if (footballer.data[7] > 3) {
                                footballer.data[7] = 3;
                            }
                        }
                    }

                    if (pilka == 1) {
                        pFootballers->savePlayerTeam();
                    }
                    else {
                        pFootballers->saveRivals();
                    }
                }//udana
                else if (co == 25) { //no problem
                    wiado[0] = (rand() % 2 == 0) ? 68 : 69;
                    wiado[1] = 49;
                    if (pilka == 1) {
                        msgFootballers[0] = msgRivalSurnames[0];
                        msgWhoBall[0] = 2;
                        pilka = 2;
                        str[1]++;
                    }
                    else {
                        msgFootballers[0] = msgPlayerSurnames[0];
                        msgWhoBall[0] = 1;
                        pilka = 1;
                        str2[1]++;
                    }
                    msgWhoBall[1] = pilka;
                    gdzie = 1;

                    vector<SFootballer> &tmpFootballers = (pilka == 1)
                        ? pFootballers->getPlayersTeam()
                        : pFootballers->getRivals();
                    int clubId = (pilka == 1) ? clubRef.clubId : clubRef.rivalData[0];

                    for (size_t index = 0; index < tmpFootballers.size(); index++) {
                        SFootballer &footballer = tmpFootballers[index];
                        if (footballer.data[0] == 1 && clubId == footballer.data[22]) {
                            footballer.data[20]++;
                            footballer.data[7]++;
                            if (footballer.data[7] > 3) {
                                footballer.data[7] = 3;
                            }
                        }
                    }

                    if (pilka == 1) {
                        pFootballers->savePlayerTeam();
                    }
                    else {
                        pFootballers->saveRivals();
                    }
                }//no problem
                else if (co == 26) { //niepewnie
                    los = (rand() % 4);
                    if (los == 0) {
                        wiado[0] = 70;
                        msgWhoBall[0] = 1;
                        if (pilka == 1) {
                            msgWhoBall[0] = 2;
                            str[1]++;
                        }
                        else {
                            str2[1]++;
                        }
                    }
                    else if (los == 1) {
                        wiado[0] = 71;
                        msgWhoBall[0] = pilka;
                    }
                    else if (los == 2) {
                        wiado[0] = 72;
                        msgWhoBall[0] = pilka;
                    }
                    else {
                        wiado[0] = 73;
                        msgWhoBall[0] = 1;
                        if (pilka == 1) {
                            msgWhoBall[0] = 2;
                        }
                    }
                    msgFootballers[0] = (pilka == 1) ? msgRivalSurnames[0] : msgPlayerSurnames[0];
                    gdzie = 3;
                }//niepewnie
                else if (co == 27) { //róg
                    isPossibleGoToTactics = true;
                    if (rand() % 2 == 0) {
                        wiado[0] = 70;
                        if (pilka == 1) {
                            str[1]++;
                        }
                        else {
                            str2[1]++;
                        }
                    }
                    else {
                        wiado[0] = 73;
                    }

                    if (pilka == 1) {
                        str[2]++;
                        msgWhoBall[0] = 2;
                        msgFootballers[0] = msgRivalSurnames[0];
                        msgFootballers[4] = msgPlayerSurnames[6];
                    }
                    else {
                        str2[2]++;
                        msgWhoBall[0] = 1;
                        msgFootballers[0] = msgPlayerSurnames[0];
                        msgFootballers[4] = msgRivalSurnames[6];
                    }
                    wiado[1] = 56;
                    msgWhoBall[1] = pilka;
                    wiado[2] = 78;
                    msgWhoBall[2] = pilka;
                    gdzie = 3;
                }//róg
                else if (co == 28) { // GOOL
                    isPossibleGoToTactics = true;
                    wiado[0] = (rand() % 2 == 0) ? 65 : 66;
                    wiado[1] = 74;
                    msgWhoBall[1] = pilka;
                    msgWhoBall[2] = pilka;
                    wiado[2] = (rand() % 2 == 0) ? 75 : 76;
                    wiado[3] = 22;
                    los = (rand() % 11) + 1;

                    vector<SFootballer> &tmpFootballers = (pilka == 1)
                        ? pFootballers->getPlayersTeam()
                        : pFootballers->getRivals();
                    int clubId = (pilka == 1) ? clubRef.clubId : clubRef.rivalData[0];

                    for (size_t index = 0; index < tmpFootballers.size(); index++) {
                        SFootballer &footballer = tmpFootballers[index];
                        if (footballer.data[0] == pam + 1 && clubId == footballer.data[22]) {
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

                            if (pilka == 1) {
                                clubRef.finances[7] += footballer.finances[2]; // premia za gola
                            }
                        }
                        if (footballer.data[0] == los && clubId == footballer.data[22]) {
                            footballer.data[20]++; // losowy gracz dosatje formę
                        }
                    }

                    if (pilka == 1) {
                        pFootballers->savePlayerTeam();
                    }
                    else {
                        pFootballers->saveRivals();
                    }

                    if (pilka == 1) {
                        msgFootballers[0] = msgRivalSurnames[0];
                        msgFootballers[2] = pClub->getClubName(clubRef.clubId - 1);
                        msgFootballers[4] = msgPlayerSurnames[pam];
                        msgFootballers[6] = pClub->getClubName(clubRef.rivalData[0] - 1);
                        pilka = 2;
                        playerGoals++;
                        msgWhoBall[3] = 2;
                        msgWhoBall[0] = 2;
                        str[1]++;
                        dlagol1[playerGoals - 1] = msgPlayerSurnames[pam];
                        x1 = minuta + 2;
                        if (x1 > 45 && (koniec == 0 || koniec == 1)) {
                            x1 = 45;
                        }

                        if (x1 > 90) {
                            x1 = 90;
                        }
                        mingol1[playerGoals - 1] = x1;
                    }
                    else {
                        msgFootballers[0] = msgPlayerSurnames[0];
                        msgFootballers[2] = pClub->getClubName(clubRef.rivalData[0] - 1);
                        msgFootballers[4] = msgRivalSurnames[pam];
                        msgFootballers[6] = pClub->getClubName(clubRef.clubId - 1);
                        pilka = 1;
                        rivalGoals++;
                        msgWhoBall[3] = 1;
                        msgWhoBall[0] = 1;
                        str2[1]++;
                        dlagol2[rivalGoals - 1] = msgRivalSurnames[pam];
                        x1 = minuta + 2;
                        if (x1 > 45 && (koniec == 0 || koniec == 1)) {
                            x1 = 45;
                        }

                        if (x1 > 90) {
                            x1 = 90;
                        }
                        mingol2[rivalGoals - 1] = x1;
                    }
                    gdzie = 1;
                    los = (rand() % 11) + 1;

                    // musi byc raz jeszcze pobranie zespolu i klubu bo zmienila sie pilka
                    vector<SFootballer> &losersFootballers = (pilka == 1)
                        ? pFootballers->getPlayersTeam()
                        : pFootballers->getRivals();
                    clubId = (pilka == 1) ? clubRef.clubId : clubRef.rivalData[0];

                    for (size_t index = 0; index < losersFootballers.size(); index++) {
                        SFootballer &footballer = losersFootballers[index];

                        if (footballer.data[0] == 1 && clubId == footballer.data[22]) {
                            footballer.data[20]--; // bramkarz traci formę i morale
                            footballer.data[8] -= 3; // morale
                            if (footballer.data[8] < -5) {
                                footballer.data[8] = 0;
                                footballer.data[7]--;
                            }
                            if (footballer.data[7] < -3) {
                                footballer.data[7] = -3;
                            }
                        }
                        if (footballer.data[0] == los && clubId == footballer.data[22]) {
                            footballer.data[20]--; // losowy zawodnik dostaje tez minusy
                        }
                    }

                    if (pilka == 1) {
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
                    msgWhoBall[1] = pilka;
                    msgWhoBall[2] = pilka;
                    wiado[2] = (rand() % 2 == 0) ? 75 : 76;
                    wiado[3] = 77;
                    msgWhoBall[3] = pilka;
                    wiado[4] = 41;
                    if (pilka == 1) {
                        msgFootballers[0] = msgRivalSurnames[0];
                        msgFootballers[2] = pClub->getClubName(clubRef.clubId - 1);
                        msgFootballers[4] = msgPlayerSurnames[pam];
                        msgFootballers[8] = msgRivalSurnames[0];
                        pilka = 2;
                        msgWhoBall[4] = 2;
                        msgWhoBall[0] = 2;
                        str[0]--;
                    }
                    else {
                        msgFootballers[0] = msgPlayerSurnames[0];
                        msgFootballers[2] = pClub->getClubName(clubRef.rivalData[0] - 1);
                        msgFootballers[4] = msgRivalSurnames[pam];
                        msgFootballers[8] = msgPlayerSurnames[0];
                        pilka = 1;
                        msgWhoBall[4] = 1;
                        msgWhoBall[0] = 1;
                        str2[0]--;
                    }
                    gdzie = 1;
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

                    msgWhoBall[0] = pilka;
                    if (pilka == 1) {
                        msgFootballers[0] = msgPlayerSurnames[pam];
                        pilka = 2;
                        msgFootballers[2] = msgRivalSurnames[0];
                    }
                    else {
                        msgFootballers[0] = msgRivalSurnames[pam];
                        pilka = 1;
                        msgFootballers[2] = msgPlayerSurnames[0];
                    }
                    wiado[1] = 41;
                    msgWhoBall[1] = pilka;
                    gdzie = 1;
                }//niecelnie
            }//obrona B gdzie=4;

            pColors->textcolor(LIGHTGRAY);
            if (minuta == 0 && koniec == 0) { //początek meczu
                memset(wiado, 0, 10 * sizeof(int));
                wiado[0] = 1; //mecz się rozpoczął
                ktoZacz = (rand() % 2) + 1;
                if (ktoZacz == 1) { //zaczyna gracz
                    msgFootballers[0] = pClub->getClubName(clubRef.clubId - 1);
                    pilka = 1;
                }
                else { //zaczyna przeciwnik
                    msgFootballers[0] = pClub->getClubName(clubRef.rivalData[0] - 1);
                    pilka = 2;
                }
                gdzie = 1;
                msgWhoBall[0] = pilka;
            }//dla minuta=0 początek meczu
            else if (minuta == 45 && koniec == 2) { //początek 2 połowy
                if (ktoZacz == 1) { //zaczyna rywal
                    msgFootballers[0] = pClub->getClubName(clubRef.rivalData[0] - 1);
                    pilka = 2;
                }
                else { // zaczyna gracz
                    msgFootballers[0] = pClub->getClubName(clubRef.clubId - 1);
                    pilka = 1;
                }
                gdzie = 1;
                msgWhoBall[0] = pilka;
                memset(wiado, 0, 10 * sizeof(int));
                wiado[0] = 2;
            }//dla początek 2 połowy
            else if (koniec == 1) { //koniec 1 połowy
                isPossibleGoToTactics = true;
                minuta = 43;
                koniec = 2; // poczatek 2 polowy
                start = 0;
                gdzie = 1;
                memset(wiado, 0, 10 * sizeof(int));
                wiado[0] = 3; // "Koniec pierwszej połowy."
            }

            //************************ kontuzja *************************
            los = (rand() % 100); // losuj czy kontuzja 1/100
            if (los == 0) {
                x1 = (rand() % 2); // losuj ktory zespol 50/50
                los = (rand() % 11) + 1; // losuj ktory zawodnik

                vector<SFootballer> &tmpFootballers = (x1 == 0) ? pFootballers->getPlayersTeam() : pFootballers->getRivals();
                int clubId = (x1 == 0) ? clubRef.clubId : clubRef.rivalData[0];

                for (size_t index = 0; index < tmpFootballers.size(); index++) {
                    SFootballer &footballer = tmpFootballers[index];
                    if (footballer.data[0] == los && footballer.data[22] == clubId && footballer.data[12] < 2) {
                        footballer.data[11] = 0;
                        footballer.data[19] = 1;
                        for (i = 10; i > 0; i--) {
                            if (wiado[i] == 0) {
                                k = i;
                            }
                        }
                        wiado[k] = (x1 == 0) ? 83 : 84; // %ls doznaje kontuzji! / %ls kontuzjowany!

                        if (x1 == 0) {
                            msgWhoBall[k] = 1;
                            msgFootballers[k * 2] = msgPlayerSurnames[los - 1];
                        }
                        else {
                            msgWhoBall[k] = 2;
                            msgFootballers[k * 2] = msgRivalSurnames[los - 1];
                        }
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
                    if (footballer.data[11] == 0 && clubRef.rivalData[0] == footballer.data[22] && footballer.data[0] < 12) {
                        x1 = footballer.data[0];
                        x2 = footballer.data[2]; // formacja na jakiej gra
                    }
                }

                for (i = 0; i < 3; i++) {
                    if (juzzmienil[i] == 0) {
                        juzzmienil[i] = x1;
                        break;
                    }
                }
                for (i = 0; i < 3; i++) {
                    if (juzzmienil[i] == x1) {
                        x3++;
                    }
                }
                if (x3 > 1) {
                    for (i = 2; i >= 0; i--) {
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
                        if (footballer.data[0] == x1 && clubRef.rivalData[0] == footballer.data[22]) {
                            footballer.data[0] = x3;
                        }
                        else if (footballer.data[0] == x3 && clubRef.rivalData[0] == footballer.data[22]) {
                            footballer.data[0] = x1;
                        }
                    }
                    pFootballers->saveRivals();
                    whoPlayerChanges = 2;
                    isPlayerChanges = true;
                }
            }//dla if isPossibleGoToTactics
            //************************************************
            if (whoPlayerChanges > 0) { //zmiana zawodnika**************
                for (i = 10; i > 0; i--) {
                    if (wiado[i] == 0) {
                        k = i;
                    }
                }
                wiado[k] = 5;
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
            if (x1 == 1 && gdzie == 1) {
                for (i = 10; i > 0; i--) {
                    if (wiado[i] == 0) {
                        k = i;
                    }
                }
                wiado[k] = 86;
                wiado[k + 1] = 87;
                wiado[k + 2] = 88;
                wiado[k + 3] = 89;
                if (clubRef.rivalData[1] == 0) { //u mnie
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
            for (i = 0; i < 10; i++) {
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

                        swprintf(tmpMessage, MAX_NEWS_LENGTH, meczs.message, msgFootballers[k].c_str(), msgFootballers[k + 1].c_str());

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
            memset(wiado, 0, 10 * sizeof(int));
            minuta += 2;
            minuta2 += 2;
            //****************** posiadanie piłki i strefy **********************
            if (pilka == 1) {
                pos++;
            }
            pos1 = (pos * 100) / (minuta2 / 2);
            pos2 = 100 - pos1;

            if (gdzie == 1) {
                strefa[1]++;
            }
            else {
                if (pilka == 1){
                    strefa[2]++;
                }
                else {
                    strefa[0]++;
                }
            }
            strefa[3] = (strefa[0]*100) / (minuta2 / 2);
            strefa[4] = (strefa[1]*100) / (minuta2 / 2);
            strefa[5] = (strefa[2]*100) / (minuta2 / 2);
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
        if (minuta > 45 && gdzie == 1 && koniec == 0 && q == 0) {
            koniec = 1; // koniec 1 polowy
            minuta = 45;
            q = 1;
            gdzie = 1;
        }//koniec1 połowy

        if (minuta > 90 && gdzie == 1 && koniec == 2) {
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
                //scanf("%d", &czas);
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
    if (clubRef.rivalData[1] == 0) {
        x1 = (rand() % 30) + 30;
        h = x1 * 5000.0;
        clubRef.finances[0] += h; //bilety
        x2 = (rand() % 30) + 30;
        h = x2 * 5000.0;
        clubRef.finances[1] += h; //Tv
        k = (rand() % 10) + 1;
        clubRef.finances[2] += k * 1000; //handel
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
        while (k != 14) { //wszystkie mecze kolejki
            k += 2;
            sumaO = clubRef.roundNumber * 16;
            sumaP = (rand() % 5);
            sumaN = (rand() % 4);
            clubRef.goalsLeague[sumaO - 16 + k] = sumaP;
            clubRef.goalsLeague[sumaO - 16 + k + 1] = sumaN;
        }

        sumaO = pRounds->getClubIndexInRoundByClubId(clubRef.roundNumber, clubRef.clubId);

        sumaP = clubRef.roundNumber * 16;
        if (sumaO % 2 == 0) {
            clubRef.goalsLeague[sumaP - 16 + sumaO] = playerGoals;
            clubRef.goalsLeague[sumaP - 16 + sumaO + 1] = rivalGoals;
        }
        else {
            clubRef.goalsLeague[sumaP - 16 + sumaO] = playerGoals;
            clubRef.goalsLeague[sumaP - 16 + sumaO - 1] = rivalGoals;
        }

        for (size_t index = 0; pRounds->getSize(); index++) {
            SRound &round = pRounds->get(index);

            if (clubRef.roundNumber == round.number) {
                pTable->updateAfterMatch(round, sumaP, clubRef);
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
            footballer.data[13] = 0;
        }

        if (footballer.data[14] == 1) {
            footballer.data[14] = 0;
        }

        if (footballer.data[21] > 0) {
            footballer.data[16] += footballer.data[21];
            footballer.data[21] = 0;
        }

        if (footballer.data[12] >= 2) {
            footballer.data[14] = 1;
            footballer.data[12] = 0;
            footballer.data[13] = 0;
        }

        if (footballer.data[12] > 0) {
            footballer.data[13] += footballer.data[12];
        }

        footballer.data[9] += footballer.data[20];
        footballer.data[12] = 0;

        if (footballer.data[9] > 10) {
            footballer.data[9] = 10;
        }

        if (footballer.data[9] < 1) {
            footballer.data[9] = 1;
        }
    }
    pFootballers->savePlayerTeam();

    //******************** tabela ***********************************
    sumaN = 0;
    while (sumaN != 3) {
        sumaN++;
        k = -1;
        for (size_t index = 0; index < pTable->getSize(); index++) {
            STable &tableRef = pTable->get(index);
            k++;
            pkt[k] = tableRef.data[7]; //zapisuje punkty do pkt
            pkt2[k] = tableRef.num;
            golr[k] = tableRef.data[6];
            gol[k] = tableRef.data[4]; //gole zdobyte
            blokada[k] = 0;
        }

        for (i = 0; i < 16; i++) { //sortowanie wg. pkt
            blokada[i] = 0;
            for (k = i + 1; k < 16; k++) {
                if (pkt[i] < pkt[k]) {
                    sumaO = pkt2[i]; //pkt2 numery klubów, zamieniam wg. kolejności
                    pkt2[i] = pkt2[k];
                    pkt2[k] = sumaO;
                    //**********
                    sumaO = pkt[i]; //punkty klubów
                    pkt[i] = pkt[k];
                    pkt[k] = sumaO;
                }
            }
        }
        for (i = 0; i < 16; i++) { //sortowanie wg. +/-
            for (k = i + 1; k < 16; k++) {
                if (golr[i] < golr[k] && pkt[i] == pkt[k]) {
                    sumaO = pkt2[i]; //pkt2 numery klubów, zamieniam wg. kolejności
                    pkt2[i] = pkt2[k];
                    pkt2[k] = sumaO;
                    //**********
                    sumaO = pkt[i]; //punkty klubów
                    pkt[i] = pkt[k];
                    pkt[k] = sumaO;
                    //**********
                    sumaO = golr[i]; //różnica goli
                    golr[i] = golr[k];
                    golr[k] = sumaO;
                }
            }
        }
        for (i = 0; i < 16; i++) {//sortowanie wg. goli zdobytych
            for (k = i + 1; k < 16; k++) {
                if (gol[i] < gol[k] && golr[i] == golr[k] && pkt[i] == pkt[k]) {
                    sumaO = pkt2[i]; //pkt2 numery klubów, zamieniam wg. kolejności
                    pkt2[i] = pkt2[k];
                    pkt2[k] = sumaO;
                    //**********
                    sumaO = pkt[i]; //punkty klubów
                    pkt[i] = pkt[k];
                    pkt[k] = sumaO;
                    //**********
                    sumaO = golr[i]; //różnica goli
                    golr[i] = golr[k];
                    golr[k] = sumaO;
                    //***********
                    sumaO = gol[i]; //gole zdobyte
                    gol[i] = gol[k];
                    gol[k] = sumaO;
                }
            }
        }

        vector<STable> newTable;
        for (int i = 0; i < 16; i++) {
            for (int k = 0; k < 16; k++) {
                for (int v = 0; v < 16; v++) {
                    for (size_t index = 0; index < pTable->getSize(); index++) {
                        STable &tableRef = pTable->get(index);
                        if (pkt2[i] == tableRef.num &&
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
    }//dla while!=2
    pColors->textcolor(LIGHTGREEN);
    return true;
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

int Match::whatHappened(int pilka, int PnaP, int OnaA, int AnaO, int sumaB, int sumaB2, int gdzie, const SClub &clubRef) {
    int x1, x2, x3, los, co;
    if (gdzie == 1) { //gdzie = 1 P/P
        if (pilka == 1) {
            x1 = clubRef.inst[4]; //gra z kontry twoja
            x2 = clubRef.trained[1]; //podania trening
            x3 = clubRef.rivalInst[2]; //pressing rywala
        }
        else {  //if (pilka==2)
            x1 = clubRef.rivalInst[4]; //gra z kontry rywala
            x2 = 2; //podania trening rywala
            x3 = clubRef.inst[2]; //pressing twój
        }
        los = (rand() % 27) + 1;
        if (los < 10) {
            co = los;
        }
        else {
            if ((PnaP > 20 && pilka == 1) || (PnaP < -20 && pilka == 2)) { // przewaga duża
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
            else if ((PnaP > 10 && PnaP < 21 && pilka == 1) || (PnaP < -10 && PnaP > -21 && pilka == 2))//przewaga mała
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
            else if ((PnaP > 20 && pilka == 2) || (PnaP < -20 && pilka == 1)) { //osłabienie duże
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
            else if ((PnaP < -10 && PnaP > -21 && pilka == 1) || (PnaP > 10 && PnaP < 21 && pilka == 2)) { //osłabienie
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
    }//gdzie = 1 P/P
    else if (gdzie == 2) { //skrzydłowy przy piłce //gdzie =2 A/O
        los = (rand() % 12) + 1;
        if (los < 5) {
            co = los + 9;
        }
        else {
            if ((AnaO > 10 && pilka == 1) || (OnaA < -10 && pilka == 2)) { //duża przewaga ataku
                los = (rand() % 10) + 1;
                if (los == 1 || los == 2) co = 11;
                else co = 10;
            }
            else if ((AnaO > 0 && AnaO < 11 && pilka == 1) || (OnaA > -11 && OnaA < 0 && pilka == 2)) { //przewaga ataku
                if (los == 6 || los == 7 || los == 8 || los == 9) co = 10;
                else if (los == 10 || los == 11 || los == 12) co = 11;
                else co = 13;
            }//przewaga w ataku
            else if ((AnaO < -40 && pilka == 1) || (OnaA > 40 && pilka == 2)) { //duża przewaga obrony
                los = (rand() % 10) + 1;
                if (los == 1 || los == 2) co = 12;
                else co = 13;
            }
            else if ((AnaO > -41 && AnaO < -20 && pilka == 1) || (OnaA > 20 && OnaA < 41 && pilka == 2)) { //przewaga obrony
                if (los == 5 || los == 6 || los == 7 || los == 8 || los == 9) co = 13;
                else if (los == 10 || los == 11) co = 10;
                else co = 11;
            }//przewaga w obronie
            else { //if ((AnaO > -21&&AnaO<1&&pilka==1)||(OnaA<21&&OnaA > -1&&pilka==2))
                if (los > 4 && los < 9) co = los + 5;
                else co = los + 1;
            }
        }//else
    }
    else if (gdzie == 3) { //pole karne A/O
        los = (rand() % 30) + 1;
        if (pilka == 1) {
            x1 = clubRef.rivalInst[3]; //pułapki ofsajdowe rywala
        }
        else {
            x1 = clubRef.inst[3];
        }

        if (los < 11) {
            co = los + 13;
        }
        else {
            if ((AnaO > 10 && pilka == 1) || (OnaA < -10 && pilka == 2)) { //duża przewaga ataku
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
            else if ((AnaO > 0 && AnaO < 11 && pilka == 1) || (OnaA > -11 && OnaA < 0 && pilka == 2)) { //przewaga ataku
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
            else if ((AnaO < -40 && pilka == 1) || (OnaA > 40 && pilka == 2)) { //duża przewaga obrony
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
            else if ((AnaO > -41 && AnaO < -20 && pilka == 1) || (OnaA > 20 && OnaA < 41 && pilka == 2)) { //przewaga obrony
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
            else if ((AnaO > -21 && AnaO < 1 && pilka == 1) || (OnaA < 21 && OnaA > -1 && pilka == 2)) {
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
    }//dla gdzie==3 A/O
    else if (gdzie == 4 || gdzie == 6) { //obrona B sumaB max 43, mini 8, śred 26
        los = (rand() % 12) + 1;
        if (gdzie == 4) {
            if (pilka == 1) x1 = sumaB2; //umiejętności bramkarza
            else x1 = sumaB;
        }
        else if (gdzie == 6) {
            if (pilka == 1) x1 = sumaB2 - (clubRef.trained[2] * 2); //umiejętności bramkarza
            else x1 = sumaB - 2;
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
        //dystans=0;
    }//gdzie = 4//obrona B
    else if (gdzie == 5) { //obrona B - karny, sma na sam
        los = (rand() % 9) + 1;
        if (pilka == 1) {
            x1 = sumaB2 - clubRef.trained[2]; //um. bramkarza-stałe fragmenty
        }
        else {
            x1 = sumaB - 2;
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
    }//gdzie = 5//obrona B

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
    int gdzie,
    int pilka
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
    if (gdzie == 1) {
        wcout << L"                 ^";
    }
    else {
        if (pilka == (isHome ? 1 : 2)) {
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
