
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
#include "Logger.h"

class Match {
public:
    Match(
        PlayerClub *pClub,
        const Colors *pColors,
        const Input *pInput,
        Footballers *pFootballers,
        Table *pTable,
        Rounds *pRounds,
        const TeamInstructions *pTeamInstruction
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
    vector<SNews> matchMsgs;
    Logger *pLogger;

    // roznice {Obrona gracz} minus {Atak przeciwnika}, itd
    int OnaA;
    int PnaP;
    int AnaO;

    void drawTeam(int usta, int tryb, int kto);
    int whatHappened(int pilka, int PnaP, int OnaA, int AnaO, int sumaB, int sumaB2, int gdzie, const SClub &clubRef);
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
        int gdzie,
        int pilka
    );
    void drawOnaA(bool isHome, int OnaA);
    void drawAnaO(bool isHome, int AnaO);
    void drawWhoScored(int mingol1[], int mingol2[], wstring dlagol1[], wstring dlagol2[]);
    void saveMatchMessageToFile(int bkgcolor, int minuta, const wstring message, const wstring param1, wstring param2);
};

#endif /* MATCH_H */

