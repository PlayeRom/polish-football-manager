#include <stdio.h>
#include <cstdlib>
#include <ctype.h>
#include <cmath>
#include <cwchar>
#include <iomanip>

#include "Manager.h"
#include "Structs.h"
#include "BoxDrawingChars.h"
#include "ConioColors.h"

Manager::Manager(
    PlayerClub *pClub,
    const Colors *pColors,
    const Input *pInput,
    Footballers *pFootballers,
    Table *pTable,
    Rounds *pRounds,
    News *pNews,
    Language *pLang,
    const Random *pRand
) {
    this->pClub = pClub;
    this->pColors = pColors;
    this->pInput = pInput;
    this->pFootballers = pFootballers;
    this->pTable = pTable;
    this->pRounds = pRounds;
    this->pNews = pNews;
    this->pLang = pLang;
    this->pRand = pRand;

    pTactic = new Tactic(pColors, pLang);
    pSquad = new Squad(pColors, pLang);
    pTeamInstr = new TeamInstructions(pColors, pLang);

    pMatch = new Match(
        pClub,
        pColors,
        pInput,
        pFootballers,
        pTable,
        pRounds,
        pTeamInstr,
        pLang,
        pRand
    );

    pLogger = new Logger();

    // transfer list filters:
    filterPosition = 0; // 0 = everybody
    filterGoalkeeper = 1;
    filterDefense = 1;
    filterMidfield = 1;
    filterAttact = 1;
    filterForm = 1;
    filterMinPrice = 0;
    filterMaxPrice = 6000000;
}

Manager::~Manager()
{
    delete pTactic;
    delete pSquad;
    delete pLogger;
    delete pMatch;
    delete pTeamInstr;
}

void Manager::runManager()
{
    wchar_t mainMenu = 0;

    do {
        setAssistantMessageAfterMatch();
        setRivalForPlayer();

        info();
        mainMenu = displayMainMenu(pTactic->getTeamSettings());
        pColors->textColor(GREEN);
        switch (mainMenu) {
            case 'C': { // continue / match
                menuItemContinue();
                break;
            }
            case 'A': { // tactics
                menuItemTactics();
                break;
            }
            case 'S': { // team composition
                menuItemTeamSquad();
                break;
            }
            case 'R': { // training
                menuItemTraining();
                break;
            }
            case 'O': { // last match
                menuItemLastMatch();
                break;
            }
            case 'P': { // upcoming opponent
                menuItemRival();
                break;
            }
            case 'K': { // calendar
                menuItemCalendar();
                break;
            }
            case 'T': { // league table
                menuItemTable();
                break;
            }
            case 'L': { // transfer list
                menuItemTransfersList();
                break;
            }
            case 'F': { // finances
                menuItemFinance();
                break;
            }
            case 'D': {// club management
                menuItemManagement();
                break;
            }
            case 'M': { // manager - statistics
                menuItemManagerStats();
                break;
            }
            case 'E': { // old news
                menuItemNewsOld();
                break;
            }
            case 'W': { // you have a message
                menuItemNews();
                break;
            }
            case 'H': { // option for the match
                menuItemOptions();
                break;
            }
            case 'Q': { // exit
                pColors->textColor(LIGHTRED);
                wcout << endl << pLang->get(L"Do you want to exit the game? (Y/n): ");

                wchar_t yn = pInput->getKeyboardPressed();
                mainMenu = (yn == pLang->getYesKeyboard() || yn == L'\n') ? 'Q' : 0;
                break;
            }
        }
    }
    while (mainMenu != 'Q');

    pInput->clrscr();
    wcout << pColors->setColors(1, Colors::RESET);
}

void Manager::info()
{
    wstring dniT[7] = {
        pLang->get(L"Mon"),
        pLang->get(L"Tue"),
        pLang->get(L"Wed"),
        pLang->get(L"Thu"),
        pLang->get(L"Fri"),
        pLang->get(L"Sat"),
        pLang->get(L"Sun")
    };
    pInput->clrscr();

    pColors->textColor(WHITE);

    const SClub &clubRef = pClub->get();

    if (clubRef.clubId > 0) {
        wcout << L" " << pClub->getClubName(clubRef.clubId - 1);
    }
    else {
        wcout << pLang->get(L" Unemployed");
    }
    pColors->textColor(GREEN);

    wcout << L" " << clubRef.managerName << L" " << clubRef.managerSurname;
    if (clubRef.isNick) {
        wcout << L" \"" << clubRef.nick << L"\"";
    }
    pColors->textColor(WHITE);

    wprintf(
        pLang->get(L" Date: %ls %02d/%02d/%d").c_str(),
        dniT[clubRef.weekNumber - 1].c_str(),
        pLang->getLngCode() == "pl_PL" ? clubRef.day : clubRef.month,
        pLang->getLngCode() == "pl_PL" ? clubRef.month : clubRef.month,
        clubRef.year
    );
}

wchar_t Manager::displayMainMenu(const wstring *pTactics)
{
    const SClub &clubRef = pClub->get();
    int tablePos = pTable->getPositionInTable(clubRef.clubId);
    pColors->textColor(LIGHTGRAY);

    wcout << endl << endl << pLang->get(L" MENU") << endl;

    pColors->textColor(LIGHTGREEN);
    wcout << endl << pLang->get(clubRef.isMatch ? L" C Match" : L" C Continue");

    pColors->textColor(GREEN);
    wcout << endl << pLang->get(L" A Tactics: ") << pTactics[clubRef.teamSetting - 1];
    wcout << endl << pLang->get(L" S Squad");
    wcout << endl << pLang->get(L" R Training");

    pColors->textColor(LIGHTBLUE);
    wcout << endl << pLang->get(L" O Last match - report");
    wcout << endl << pLang->get(L" P Opponent: ") << pClub->getClubName(clubRef.rivalData[0] - 1);
    wcout << endl << pLang->get(L" K Calendar");
    wcout << endl;
    wprintf(pLang->get(L" T Table: %d place").c_str(), tablePos);
    wcout << endl << pLang->get(L" L Transfer list");

    pColors->textColor(YELLOW);
    wcout << endl;
    wprintf(pLang->get(L" F Finances: $%.2f").c_str(), clubRef.finances[11]);
    wcout << endl << pLang->get(L" D Club management");
    wcout << endl << pLang->get(L" M Manager");
    if (pNews->isMessage()) {
        pColors->startBlinking();
        pColors->textColor(LIGHTRED);
        wcout << endl << pLang->get(L" W You have a message");
        pColors->stopBlinking();
    }
    else {
        pColors->textColor(LIGHTGRAY);
        wcout << endl << pLang->get(L" E Old news");
    };

    pColors->textColor(BROWN);
    wcout << endl << endl << pLang->get(L" H Match options");

    pColors->textColor(RED);
    wcout << endl << pLang->get(L" Q Quit") << endl;

    return pInput->getKeyboardPressed();
}

void Manager::swapFootballers()
{
    int no1, no2;

    const size_t playersCount = pFootballers->getSizePlayerTeam();

    pColors->textColor(LIGHTGRAY);
    wcout << endl << pLang->get(L"Enter the number, space, second number and press Enter: ");
    pInput->getNumbersExchange(no1, no2);

    if (no1 < 1 || no1 > playersCount ||
        no2 < 1 || no2 > playersCount
    ) {
        return; // Numbers out of scope
    }

    if (no1 == no2) {
        return; // The same numbers, nothing to swap
    }

    for (size_t i = 0; i < playersCount; i++) {
        SFootballer &footballer = pFootballers->getPlayerTeam(i);
        if (footballer.data[0] == no1) {
            footballer.data[0] = no2;
        }
        else if (footballer.data[0] == no2) {
            footballer.data[0] = no1;
        }
    }

    pFootballers->sortPlayersTeam();
    pFootballers->savePlayerTeam();
}

void Manager::menuItemContinue()
{
    SClub &clubRef = pClub->get();

    if (clubRef.clubId > 0) {
        if (clubRef.isMatch) {
            menuItemContinueMatch(clubRef);
        }
        else {
            // no match, so processing
            menuItemContinueProcessing(clubRef);
        }
    }
    else { // manager unemployed
        menuItemContinueUnemployed(clubRef);
    }
}

/**
 *  Count the number of days off training
 */
int Manager::getTrainingHolidaysCounter(SClub &clubRef) const
{
    int holidaysCounter = 0;
    for (int i = 0; i < TRAINING_SLOTS_NUMBER; i++) {
        if (clubRef.training[i] == TRAINING_HOLIDAY) {
            holidaysCounter++; // count the number of days off training
        }
    }

    return holidaysCounter;
}

/**
 * Processing of passing days
 *
 * @param clubRef
 */
void Manager::menuItemContinueProcessing(SClub &clubRef)
{
    pColors->textColor(LIGHTGRAY);
    clubRef.isBlockTransferFunds = 0;

    pFootballers->changeTransferList();

    if (pNews->isMessage()) { // is there any message at all
        pNews->addDisplayManagerMessages(clubRef, false);
    }

    bool isProcessing = true;
    while (isProcessing) {
        clubRef.day++;
        clubRef.weekNumber++;
        if (clubRef.weekNumber == 8) {
            clubRef.weekNumber = 1;

            int holidaysCounter = getTrainingHolidaysCounter(clubRef);
            if (holidaysCounter == 0) {
                // 0 days off, morale down
                for (size_t i = 0; i < pFootballers->getSizePlayerTeam(); i++) {
                    SFootballer &footballer = pFootballers->getPlayerTeam(i);
                    pFootballers->moraleDown(footballer, 3);
                }

                pFootballers->savePlayerTeam();
            }
            else if (holidaysCounter > 1) {
                // there are days off, morale up
                for (size_t i = 0; i < pFootballers->getSizePlayerTeam(); i++) {
                    SFootballer &footballer = pFootballers->getPlayerTeam(i);
                    pFootballers->moraleUp(footballer, holidaysCounter - 1);
                }

                pFootballers->savePlayerTeam();
            }
        }

        if ((
                clubRef.month == 1
                || clubRef.month == 3
                || clubRef.month == 5
                || clubRef.month == 7
                || clubRef.month == 8
                || clubRef.month == 10
            ) && clubRef.day == 32
        ) {
            clubRef.day = 1;
            clubRef.month++;
        }

        if (clubRef.month == 2 && clubRef.day == 29) {
            clubRef.day = 1;
            clubRef.month++;
        }

        if ((clubRef.month == 4 || clubRef.month == 6 || clubRef.month == 9 || clubRef.month == 11)
            && clubRef.day == 31
        ) {
            clubRef.day = 1;
            clubRef.month++;
        }

        if (clubRef.month == 12 && clubRef.day == 32) {
            clubRef.day = 1;
            clubRef.month = 1;
            clubRef.year++;
        }

        //****************************** co miesiąc ****************
        if (clubRef.day == 1) {
            isProcessing = false;
            int tablePos = pTable->getPositionInTable(clubRef.clubId);

            if (tablePos == 0) {
                // Zarząd: Wiążemy z panem wielkie nadzieje i oczekujemy sukcesów. Oby współpraca między nami była jak najlepsza.
                pNews->setTmpMsgData(43);
            }
            else if (tablePos < 4) {
                // Zarząd: Jesteśmy bardzo zadowoleni z pańskiej pracy. Mamy nadzieję, iż utrzyma pan dobrą passę.
                pNews->setTmpMsgData(31);
            }
            else if (tablePos > 3 && tablePos < 10) {
                pNews->setTmpMsgData(32);
            }
            else if (tablePos > 9 && tablePos < 14) {
                pNews->setTmpMsgData(33);
            }
            else if (tablePos > 13) {
                pNews->setTmpMsgData(34);
            }

            if (clubRef.finances[12] <= 500000 && clubRef.finances[11] <= 0) {
                pNews->setTmpMsgData(35);
            }
            else if (clubRef.finances[12] <= 500000 && clubRef.finances[11] > 0) {
                pNews->setTmpMsgData(36);
            }
            else if (clubRef.finances[12] <= 1000000 && clubRef.finances[11] <= 0) {
                pNews->setTmpMsgData(37);
            }
            else if (clubRef.finances[12] <= 1000000 && clubRef.finances[11] > 0) {
                pNews->setTmpMsgData(38);
            }
            else if (clubRef.finances[12] <= 2000000 && clubRef.finances[11] <= 0) {
                pNews->setTmpMsgData(39);
            }
            else if (clubRef.finances[12] <= 2000000 && clubRef.finances[11] > 0) {
                pNews->setTmpMsgData(40);
            }
            else if (clubRef.finances[12] > 2000000 && clubRef.finances[11] <= 0) {
                pNews->setTmpMsgData(41);
            }
            else if (clubRef.finances[12] > 2000000 && clubRef.finances[11] > 0) {
                pNews->setTmpMsgData(42);
            }

            //***** manager of month *****
            if (clubRef.month == 4
                || clubRef.month == 5
                || clubRef.month == 6
                || clubRef.month == 7
                || clubRef.month == 9
                || clubRef.month == 10
                || clubRef.month == 11
                || clubRef.month == 12
            ) {
                int tablePosRandom = pRand->get(1, 2);
                int clubId = pTable->getClubNumberInPosition(tablePosRandom);
                if (clubId == clubRef.clubId) {
                    // player got award "manager of month"
                    // %ls (<club name>) zostaje ogłoszony managerem miesiąca.
                    pNews->setTmpMsgData(clubId + 60, clubRef.managerSurname);
                    clubRef.managerStats[2]++;
                    clubRef.managerStats[3] += (clubRef.managerStats[2] * 10);
                }
                else {
                    // another manager got award "manager of month"
                    pNews->setTmpMsgData(clubId + 44);
                }
            }
        }
        //**********************************************************
        for (size_t index = 0; index < pRounds->getSize(); index++) {
            const SRound &round = pRounds->get(index);
            if (clubRef.day - 1 == round.day && clubRef.month == round.month) { // wczoraj był mecz
                clubRef.isRivalSet = 0;
                clubRef.roundNumber = round.number + 1;
            }

            if (clubRef.day == round.day && clubRef.month == round.month) {
                clubRef.isMatch = 1; // nastał dzien rozegrania meczu
                isProcessing = false;
            }
        }

        //ustalam kolejkę end
        int years = 0;
        float condition = 0;
        float passes = 0;
        float setPieces = 0; // stałe fragmenty
        float tactics = 0;
        for (int i = clubRef.weekNumber - 1; i < TRAINING_SLOTS_NUMBER; i += 7) { //sprawdzamy dany dzien
            if (clubRef.training[i] == 1) condition++; //kondycja
            if (clubRef.training[i] == 2) passes++; //podania
            if (clubRef.training[i] == 3) setPieces++; //stałe
            if (clubRef.training[i] == 4) tactics++; //taktyka
            if (clubRef.training[i] == 5) years++;
        }
        clubRef.trained[0] += condition * 0.2;
        if (condition == 0) {
            clubRef.trained[0] -= 0.1;
        }
        clubRef.trained[1] += passes * 0.2;
        if (passes == 0) {
            clubRef.trained[1] -= 0.1;
        }
        clubRef.trained[2] += setPieces * 0.2;
        if (setPieces == 0) {
            clubRef.trained[2] -= 0.1;
        }
        clubRef.trained[3] += tactics * 0.2;
        if (tactics == 0) {
            clubRef.trained[3] -= 0.1;
        }

        int playerTeamSize = pFootballers->getSizePlayerTeam();

        int randMinus = pRand->get(12); // losuje czy odjąć 1/12
        if (years == 0 && randMinus == 1) {
            clubRef.treBOPN--;
            int footballerId = pRand->get(playerTeamSize); // draw which footballer

            for (size_t index = 0; index < playerTeamSize; index++) {
                SFootballer &footballer = pFootballers->getPlayerTeam(index);
                if (footballer.data[0] == footballerId) { //--
                    randMinus = pRand->get(3, 6); // losujemy ktora umiejetnosc spada
                    footballer.data[randMinus]--;
                    if (footballer.data[randMinus] < 1) {
                        footballer.data[randMinus] = 1;
                        clubRef.treBOPN++;
                    }

                    break;
                }
            }

            pFootballers->savePlayerTeam();
        }//dla years == 0

        int randPlus = pRand->get(6); //losuje czy dodać trening BOPN (1/6 szans)
        if (years > 0 && randPlus == 1) {
            int footballerId = pRand->get(playerTeamSize); // draw which footballer

            for (size_t index = 0; index < playerTeamSize; index++) {
                SFootballer &footballer = pFootballers->getPlayerTeam(index);
                if (footballer.data[0] == footballerId) {
                    int skillIndex = footballer.data[1] + 2;
                    if (footballer.data[skillIndex] < 20) {
                        footballer.data[skillIndex]++;
                        clubRef.treBOPN++;
                    }
                    break;
                }
            }
            pFootballers->savePlayerTeam();
        } // dla if (years > 0)

        //dni kontraktów

        int newFootballerId = 0;
        int lostContractCounter = 0;
        clubRef.finances[6] = 0;
        for (size_t index = 0; index < pFootballers->getSizePlayerTeam(); index++) {
            SFootballer &footballer = pFootballers->getPlayerTeam(index);
            //************ sprzedaż zawodnika *****************
            //lista transferowa
            if (footballer.data[17] == 1 && pRand->get(20) == 1) { // 1/20 szans
                pInput->clrscr();

                // random club which want to buy footballer
                int clubIdBuyer = 0;
                do {
                    clubIdBuyer = pRand->get(16);
                }
                while (clubIdBuyer == clubRef.clubId);

                pColors->textColor(LIGHTGRAY);
                int random = pRand->get(-5, 4);
                float transfer = random * 10000.0;
                float price = transfer + footballer.finances[0];
                if (price > footballer.finances[3]) {
                    price = footballer.finances[3];
                }
                wcout << endl;
                wprintf(
                    pLang->get(L"%ls is interested in buying %ls%ls").c_str(),
                    pClub->getClubName(clubIdBuyer - 1).c_str(),
                    footballer.name,
                    footballer.surname
                );
                wcout << endl;
                wprintf(
                    pLang->get(L"The value of %ls is $%.2f.").c_str(),
                    footballer.surname,
                    footballer.finances[0] // wartosc $ zawodnika
                );
                wcout << endl;
                wprintf(
                    pLang->get(L"%ls demands $%.2f.").c_str(),
                    pClub->getClubName(clubRef.clubId - 1).c_str(),
                    footballer.finances[3] // ile jakiś klub żąda $ za kupno zawodnika
                );
                wcout << endl;
                wprintf(
                    pLang->get(L"%ls is ready to pay $%.2f.").c_str(),
                    pClub->getClubName(clubIdBuyer - 1).c_str(),
                    price
                );

                wcout << endl << endl << pLang->get(L"Do you accept the offer? (Y/n): ");
                wchar_t yn = pInput->getKeyboardPressed();
                if (yn == pLang->getYesKeyboard() || yn == L'\n') {
                    pColors->textColor(GREEN);
                    wcout << endl << endl;
                    wprintf(
                        pLang->get(L"%ls's contract expires. %ls is sold to %ls.").c_str(),
                        footballer.surname,
                        footballer.surname,
                        pClub->getClubName(clubIdBuyer - 1).c_str()
                    );
                    footballer.data[18] = 1;
                    footballer.data[17] = 0;
                    footballer.data[22] = clubIdBuyer;
                    clubRef.finances[3] += price;
                    clubRef.finances[5] = clubRef.finances[0] +
                            clubRef.finances[1] +
                            clubRef.finances[2] +
                            clubRef.finances[3] +
                            clubRef.finances[4];
                    clubRef.finances[11] = clubRef.finances[5] - clubRef.finances[10];
                    clubRef.managerStats[11]++;
                    clubRef.totalRevenuesTransfers += price;

                    pClub->save();

                    pInput->getKeyboardPressed();
                }
                else {
                    pColors->textColor(RED);
                    wcout << endl << endl <<
                        pClub->getClubName(clubRef.clubId - 1) << pLang->get(L" rejects the offer.");
                    pInput->getKeyboardPressed();
                }
            }//dla if
            //*********************************************
            footballer.data[18]--; // dni kontraktu
            //finanse - płaca
            clubRef.finances[6] += footballer.finances[1];
            //cena zawodnika
            footballer.finances[0] = 0;
            switch (footballer.data[2]) {
                case PLAYERS_POS_B: footballer.finances[0] = footballer.data[3] * 5; break;
                case PLAYERS_POS_O: footballer.finances[0] = footballer.data[4] * 5; break;
                case PLAYERS_POS_P: footballer.finances[0] = footballer.data[5] * 5; break;
                case PLAYERS_POS_N: footballer.finances[0] = footballer.data[6] * 5; break;
            }

            float price = footballer.data[footballer.data[2] + 2];
            float salary = 6000.0;

            if (price == 20) salary = 36000.0;
            else if (price >= 17) salary = 27000.0;
            else if (price >= 15) salary = 18000.0;
            else if (price >= 12) salary = 14000.0;
            else if (price >= 10) salary = 9000.0;
            else if (price >= 5) salary = 7000.0;

            footballer.finances[0] += footballer.data[3] + footballer.data[4] + footballer.data[5] + footballer.data[6];
            footballer.finances[0] *= salary;
            footballer.finances[0] += footballer.data[9] * 10000.0;
            //cena zawodnika end

            //kondycja
            if (footballer.data[11] < 100) {
                int x = footballer.data[11];
                footballer.data[11] += 2;

                if (clubRef.trained[0] >= 0) {
                    footballer.data[11] += clubRef.trained[0];
                }

                if (footballer.data[11] > 100) {
                    footballer.data[11] = 100;
                }
            }
            //kondycja end

            //kontuzja
            if (footballer.data[19] == 1) {
                footballer.data[19] = 0;
                isProcessing = false;
                // LEKARZ: %ls doznał kontuzji. Za około %d dni będzie do dyspozycji.
                int injuryDays = pRand->get(8, 90); // in how many days heals injuries
                pNews->setTmpMsgData(6, footballer.surname, injuryDays);

                footballer.data[15] = injuryDays;
                footballer.data[9]--; //spadek formy
                if (footballer.data[9] < 1) {
                    footballer.data[9] = 1;
                }
            }

            if (footballer.data[15] > 0) {
                // zmniejszam licznik ilosci dni do wyleczenia kontuzji
                footballer.data[15]--;
                footballer.data[1] = PLAYERS_TRAINING_NONE; //nie trenuje

                if (footballer.data[15] > 7) {
                    footballer.data[11] = 0;
                }

                if (footballer.data[15] <= 7) {
                    footballer.data[11] = 90;
                }

                if (footballer.data[15] == 0 || footballer.data[15] == 7) {
                    if (footballer.data[15] == 0) { // nie ma kontuzji
                        // trenuje to na jakiej gra pozycji
                        footballer.data[1] = footballer.data[2];
                    }

                    // 4 = LEKARZ: %ls całkowicie wyleczył kontuzję, zaczął trenować i może grać.
                    // 5 = LEKARZ: %ls częściowo wyleczył kontuzję i może grać, ale jeszcze nie trenuje.
                    pNews->setTmpMsgData(footballer.data[15] == 0 ? 4 : 5, footballer.surname);
                }
            }
            //kontuzja end

            // gdy zawodnik stracił kontrakt, przepisz numery porzadkowe zawodnikow
            if (footballer.data[18] == 0) { // kontrakt wygasł
                newFootballerId = footballer.data[0];
                lostContractCounter++;
                if (lostContractCounter >= 2) {
                    newFootballerId -= lostContractCounter - 1;
                }
            }

            if (footballer.data[18] > 0) {
                if (lostContractCounter > 0) {
                    footballer.data[0] = newFootballerId;
                    newFootballerId++;
                }
            }

            // 0 dni kontraktu, tracimy zawodnika i laduje on na liscie transferowej
            if (footballer.data[18] == 0) {
                isProcessing = false;

                // 3 = %ls odchodzi z klubu na mocy wygaśnięcia kontraktu.
                pNews->setTmpMsgData(3, footballer.surname);

                footballer.data[0] = pFootballers->getSizeTransfers() + 1;
                if (footballer.data[22] == clubRef.clubId) {
                    footballer.data[22] = 0;
                }

                //transferPlayers.push_back(footballer);
                pFootballers->pushBackTransfersTeam(footballer);
            }
            else if (footballer.data[18] == 7) { // przypomnienie ze za 7 dni konczy sie kontrakt
                isProcessing = false;

                // 2 = %ls: chcę przypomnieć iż za tydzień kończy mi się kontrakt.
                pNews->setTmpMsgData(2, footballer.surname);
            }
        }
        pFootballers->savePlayerTeam();
        pFootballers->saveTransfers();

        //******************** komunikaty kibiców ******************
        int random = pRand->get(6);
        if (clubRef.playerGoals != clubRef.rivalGoals && random == 1) {
            if (clubRef.playerGoals > clubRef.rivalGoals) { // win
                random = pRand->get(8, 13);
            }
            else { // loss
                random = pRand->get(14, 19);
            }

            isProcessing = false;
            pNews->setTmpMsgData(random, clubRef.isNick ? clubRef.nick : clubRef.managerSurname);
        }

        //****************** zadyma **************
        if (clubRef.isRiot) {
            isProcessing = false;
            clubRef.isRiot = 0;
            clubRef.finances[9] += 300000.0;

            // 44 = PZPN: W związku z zamieszkami w ostatnim meczu, klub %ls zostaje ukarany kwotą: 300000.00 zł.
            pNews->setTmpMsgData(44, pClub->getClubName(clubRef.clubId - 1));
        }

        //******** wywalenie z klubu i koniec ligi**************
        if (clubRef.month == 7 && clubRef.day == 1) { // nowy sezon
            clubRef.treBOPN = 0;
            clubRef.isMatch = 0;
            clubRef.roundNumber = 1;
            clubRef.isRivalSet = 0;
            clubRef.controlMatchesAmount = 1;
            clubRef.isNotAllowedTeamPlayer = 0;
            clubRef.isAssistantMsg = 0;
            clubRef.playerGoals = 0;
            clubRef.rivalGoals = 0;
            clubRef.isRiot = 0;
            clubRef.isBlockTransferFunds = 0;
            clubRef.season++;

            memset(clubRef.trained, 0, 5 * sizeof(float));
            memset(clubRef.goalsLeague, 0, 480 * sizeof(int));
            memset(clubRef.goalsControls, 0, 10 * sizeof(int));

            pRounds->initial();
            pRounds->save();

            pTable->createTable();
            pTable->save();

            isProcessing = false;
        }

        if (clubRef.month == 6 && clubRef.day == 23) { // message who won the league
            int clubId = pTable->getClubNumberInPosition(1); // get club on 1st place

            isProcessing = false;

            // 78 = %ls MISTRZEM POLSKI!!
            pNews->setTmpMsgData(78, pClub->getClubName(clubId - 1));
        }

        if ((clubRef.month == 2 && clubRef.day == 1) || (clubRef.month == 6 && clubRef.day == 23)) {
            memset(clubRef.trained, 0, 5 * sizeof(float));

            for (size_t i = 0; i < pFootballers->getSizePlayerTeam(); i++) {
                SFootballer &footballer = pFootballers->getPlayerTeam(i);
                footballer.data[9] = 6; // forma = 6
                footballer.data[10] = 0; // wyzeruj licznik zmiany formy
            }

            pFootballers->savePlayerTeam();
        }

        int clubPos = pTable->getPositionInTable(clubRef.clubId);

        if (clubRef.month == 6 && clubRef.day == 23 && clubPos == 1) { // wygranie ligi
            clubRef.managerStats[0]++;
            clubRef.managerStats[3] += (clubRef.managerStats[0] * 100);
            isProcessing = false;

            // 79 = PREZES: Wraz z zarządem składamy panu gratulacje i podziękowanie za wielkie osiągnięcia jakie pan dokonał.
            pNews->setTmpMsgData(79);
        }
        else if (((clubRef.month == 12 && clubRef.day == 1) || (clubRef.month == 6 && clubRef.day == 25)) && clubPos > 13) { // spadek do 2 ligi
            isProcessing = false;
            clubRef.clubId = 0;

            // 77 = PREZES: Wraz z zarządem oświadczamy, iż został pan zwolniony ze stanowiska managera klubu %ls.
            pNews->setTmpMsgData(77, pClub->getClubName(clubRef.clubId - 1));
        }

        //finanse
        clubRef.finances[5] = clubRef.finances[0] + clubRef.finances[1] + clubRef.finances[2] + clubRef.finances[3] + clubRef.finances[4];
        clubRef.finances[10] = clubRef.finances[6] + clubRef.finances[7] + clubRef.finances[8] + clubRef.finances[9];
        clubRef.finances[11] = clubRef.finances[5] - clubRef.finances[10];
        if (clubRef.day == 1) {
            clubRef.finances[12] += clubRef.finances[8];
            for (int i = 0; i < 12; i++) {
                clubRef.financesLastMonth[i] = clubRef.finances[i];
            }
            for (int i = 0; i < 11; i++) {
                clubRef.finances[i] = 0;
            }
            clubRef.finances[12] += clubRef.finances[11];

            for (size_t i = 0; i < pFootballers->getSizePlayerTeam(); i++) {
                clubRef.finances[6] += pFootballers->getPlayerTeam(i).finances[1];
            }

            clubRef.finances[10] = clubRef.finances[6];
            clubRef.finances[11] = clubRef.finances[5] - clubRef.finances[10];
        }
        clubRef.finances[13] = clubRef.finances[12] / 2;

        pClub->save();
    } // while
}

/**
 * To keep going, the player must play a match
 *
 * @param clubRef
 */
void Manager::menuItemContinueMatch(SClub &clubRef)
{
    clubRef.isNotAllowedTeamPlayer = pFootballers->isNotAllowedTeamPlayer() ? 1 : 0;

    pClub->save();

    if (!clubRef.isNotAllowedTeamPlayer) {
        pColors->textColor(GREEN);
        wcout << endl << endl << pLang->get(L"Do you want to go to the match? (Y/n): ");
        wchar_t matchYN = pInput->getKeyboardPressed();
        if (matchYN == pLang->getYesKeyboard() || matchYN == L'\n') {
            pMatch->runMatch();
        }
    }
    else {
        pColors->textColor(LIGHTRED);
        wcout << endl << endl <<
            pLang->get(L"The match cannot be played! The squad contains prohibited players!");
        pInput->getKeyboardPressed();
    }
}

/**
 * Get new job
 *
 * @param clubRef
 */
void Manager::menuItemContinueUnemployed(const SClub &clubRef)
{
    info();
    int club1 = pTable->getClubNumberInPosition(12);
    int club2 = pTable->getClubNumberInPosition(13);

    pColors->textColor(LIGHTGRAY);
    wcout << endl << endl <<
        pLang->get(L"You get two job offers from clubs:") << endl <<
        L"1. " << pClub->getClubName(club1 - 1) << endl <<
        L"2. " << pClub->getClubName(club2 - 1);
    wcout << endl << endl << pLang->get(L"In which club do you want to work (enter the appropriate number): ");
    int select = pInput->getNumber();
    if (select == 1 || select == 2) {
        int clubId = (select == 1) ? club1 : club2;

        pFootballers->initialPlayerTeam(clubId);
        pFootballers->savePlayerTeam();

        // 1 = PREZES: Panie %ls, witam pana w nowym miejscu pracy i mam nadzieję iż spełni pan nasze oczekiwania.
        pNews->setTmpMsgData(1, clubRef.managerSurname);

        pClub->initNewClub(clubId);

        pRounds->initial();
        pRounds->save();

        pClub->save();

        pColors->textColor(GREEN);
        wcout << endl << pLang->get(L"You took up a job at club ") << pClub->getClubName(clubRef.clubId - 1);
    }
    else {
        pColors->textColor(RED);
        wcout << endl << pLang->get(L"You entered the wrong number!");
    }

    pInput->getKeyboardPressed();
}

void Manager::menuItemTactics()
{
    SClub &clubRef = pClub->get();

    if (clubRef.clubId > 0) {
        wchar_t menu = 0;
        do {
            info();
            pTactic->drawTeamSetting(clubRef.teamSetting);
            pTactic->drawChart(clubRef.teamSetting, clubRef.clubId, pFootballers->getPlayersTeam());
            pSquad->draw(pFootballers->getPlayersTeam(), clubRef.teamSetting, 11, clubRef.clubId);

            menu = pInput->getKeyboardPressed();
            switch (menu) {
                case 'U': {
                    bool loop = false;
                    while (!loop) {
                        pInput->clrscr();
                        pColors->textBackground(BLACK);
                        pColors->textColor(LIGHTRED);
                        wcout << pLang->get(L"Current formation: ") << pTactic->getTeamSetting(clubRef.teamSetting - 1);

                        pColors->textColor(LIGHTGRAY);
                        wcout << endl << pLang->get(L"Choose a team formation:");
                        for (int i = 0; i < 14; i++) {
                            wcout << endl;
                            if (i < 9) {
                                wcout << L" ";
                            }
                            wcout << i + 1 << L". " << pTactic->getTeamSetting(i);
                        }
                        wcout << endl << endl << pLang->get(L"Enter the appropriate number: ");
                        clubRef.teamSetting = pInput->getNumber();
                        if (clubRef.teamSetting > 0 && clubRef.teamSetting < 15) {
                            loop = true;

                            pClub->save();
                        }
                        else {
                            clubRef.teamSetting = 15;
                        }
                    }
                    break;
                }
                case 'P': {
                    swapFootballers();
                    break;
                }
                case 'R': {
                    info();
                    pColors->textColor(WHITE);
                    wcout << endl << pLang->get(L"SUBSTITUTE'S BENCH");
                    pSquad->draw(pFootballers->getPlayersTeam(), clubRef.teamSetting, 16, clubRef.clubId);
                    pColors->textColor(LIGHTGRAY);
                    wcout << endl << endl << pLang->get(L"Press any key...");
                    pInput->getKeyboardPressed();
                    break;
                }
                case 'I': {
                    int belka = 1;
                    wchar_t menuInstr = 0;
                    do {
                        info();
                        pColors->textColor(WHITE);
                        wcout << endl << endl << pLang->get(L"TEAM INSTRUCTIONS") << endl;

                        pTeamInstr->draw(
                            clubRef.inst[0],
                            clubRef.inst[1],
                            clubRef.inst[2],
                            clubRef.inst[3],
                            clubRef.inst[4],
                            clubRef.inst[5],
                            belka
                        );

                        pColors->textColor(RED);
                        wcout << endl << L" " << pLang->get(L"Q Back") << endl;

                        pColors->textColor(GREEN);
                        pColors->textBackground(BLACK);

                        menuInstr = pInput->getKeyboardPressed();
                        switch (menuInstr) {
                            case _KEY_UP: {
                                if (--belka == 0) {
                                    belka = 6;
                                }
                                break;
                            }
                            case _KEY_DOWN: {
                                if (++belka == 7) {
                                    belka = 1;
                                }
                                break;
                            }
                            case _KEY_RIGHT: { // C 67
                                if (belka == 1) {
                                    if (++clubRef.inst[5] == 4) {
                                        clubRef.inst[5] = 1;
                                    }
                                }
                                else if (belka == 2) {
                                    if (++clubRef.inst[0] == 5) {
                                        clubRef.inst[0] = 1;
                                    }
                                }
                                else if (belka == 3) {
                                    if (++clubRef.inst[1] == 4) {
                                        clubRef.inst[1] = 1;
                                    }
                                }
                                else if (belka == 4) {
                                    if (++clubRef.inst[2] == 2) {
                                        clubRef.inst[2] = 0;
                                    }
                                }
                                else if (belka == 5) {
                                    if (++clubRef.inst[3] == 2) {
                                        clubRef.inst[3] = 0;
                                    }
                                }
                                else if (belka == 6) {
                                    if (++clubRef.inst[4] == 2) {
                                        clubRef.inst[4] = 0;
                                    }
                                }
                                break;
                            }
                            case _KEY_LEFT: { // D 68
                                if (belka == 1) {
                                    if (--clubRef.inst[5] == 0) {
                                        clubRef.inst[5] = 3;
                                    }
                                }
                                else if (belka == 2) {
                                    if (--clubRef.inst[0] == 0) {
                                        clubRef.inst[0] = 4;
                                    }
                                }
                                else if (belka == 3) {
                                    if (--clubRef.inst[1] == 0) {
                                        clubRef.inst[1] = 3;
                                    }
                                }
                                else if (belka == 4) {
                                    if (--clubRef.inst[2] == -1) {
                                        clubRef.inst[2] = 1;
                                    }
                                }
                                else if (belka == 5) {
                                    if (--clubRef.inst[3] == -1) {
                                        clubRef.inst[3] = 1;
                                    }
                                }
                                else if (belka == 6) {
                                    if (--clubRef.inst[4] == -1) {
                                        clubRef.inst[4] = 1;
                                    }
                                }
                                break;
                            }
                        }
                        pClub->save();
                    }
                    while (menuInstr != 'Q');
                    break;
                }
            }
        }
        while (menu != 'Q');
    }
}

void Manager::menuItemTeamSquad()
{
    SClub &clubRef = pClub->get();

    if (clubRef.clubId > 0) {
        bool isNext = false;
        int mode = 0;
        int sort = 0;
        wchar_t menu = 0;
        do {
            info();
            pColors->textColor(WHITE);
            wcout << endl << pLang->get(L"SQUAD") << getSortTitle(sort);

            pColors->textColor(LIGHTGRAY);
            if (sort == 0) {
                mode = isNext ? 40 : 20;
            }

            pSquad->draw(pFootballers->getPlayersTeam(), clubRef.teamSetting, mode, clubRef.clubId, false, sort);

            size_t playersCount = pFootballers->getSizePlayerTeam();

            pColors->textColor(GREEN);
            wcout << endl;
            if (sort == 0) {
                if (playersCount > 20) {
                    wprintf(L"%-10ls", pLang->get(L"D Next").c_str());
                }
                wcout << pLang->get(L"P Swap players         G Details     S Sort");
            }
            else {
                wcout << pLang->get(L"T Traditional view   G Details     S Sort");
            }

            pColors->textColor(RED);
            wcout << L"   " << pLang->get(L"Q Back to MENU");

            pColors->textColor(LIGHTGRAY);
            menu = pInput->getKeyboardPressed();
            switch (menu) {
                case 'T': { // reset sorting
                    sort = 0;
                    break;
                }
                case 'D': {
                    if (sort == 0) {
                        isNext = (!isNext && playersCount > 20) ? true : false;
                    }
                    break;
                }
                case 'S': { // sorting
                    pInput->clrscr();

                    pColors->textColor(GREEN);
                    wcout << endl << pLang->get(L"Sort by:")
                        << endl << L" " << pLang->get(L"B Goalkeeper skills")
                        << endl << L" " << pLang->get(L"O Defencs skills")
                        << endl << L" " << pLang->get(L"P Midfield skills")
                        << endl << L" " << pLang->get(L"N Attack skills")
                        << endl << L" " << pLang->get(L"M Morale")
                        << endl << L" " << pLang->get(L"F Form")
                        << endl << L" " << pLang->get(L"K Condition")
                        << endl << L" " << pLang->get(L"G Goals")
                        << endl << L" " << pLang->get(L"W Contract expiry order");
                    //\n\r C Ceny\n\r E Pensji");

                    pColors->textColor(RED);
                    wcout << endl << L" " << pLang->get(L"Q Back") << endl;

                    pColors->textColor(GREEN);
                    wchar_t menuSort = pInput->getKeyboardPressed();
                    switch (menuSort) {
                        case 'B':
                            sort = 3;
                            break;
                        case 'O':
                            sort = 4;
                            break;
                        case 'P':
                            sort = 5;
                            break;
                        case 'N':
                            sort = 6;
                            break;
                        case 'M'://morale
                            sort = 7;
                            break;
                        case 'F'://forma
                            sort = 9;
                            break;
                        case 'K'://kondycja
                            sort = 11;
                            break;
                        case 'G'://gole
                            sort = 16;
                            break;
                        case 'W':
                            sort = 18;
                            break;
                        case 'C'://cena
                            break;
                        case 'E'://pensja
                            break;
                        default:
                            sort = 0;
                            break;
                    }//dla switch
                    break;
                }
                case 'G': { // szczegóły zawodnika
                    menuItemTeamSquadFootballerDetails();
                    break;
                }
                case 'P': {
                    if (sort == 0) {
                        swapFootballers();
                    }
                    break;
                }
            }
        }
        while (menu != 'Q');
    }
}

void Manager::menuItemTeamSquadFootballerDetails()
{
    pColors->textColor(LIGHTGRAY);
    wcout << endl << pLang->get(L"Enter the No: ");
    int footballerId = pInput->getNumber();

    for (size_t index = 0; index < pFootballers->getSizePlayerTeam(); index++) {
        SFootballer &footballer = pFootballers->getPlayerTeam(index);
        if (footballer.data[0] == footballerId) {
            wchar_t menuDetails = 0;
            do {
                info();

                pColors->textColor(LIGHTGRAY);
                wcout << endl << endl << pLang->get(L"Surname and name: ") << footballer.surname << L" " << footballer.name;
                wstring positionPlayer;
                switch (footballer.data[2]) {
                    case PLAYERS_POS_B: positionPlayer = L"Goalkeeper"; break;
                    case PLAYERS_POS_O: positionPlayer = L"Defender"; break;
                    case PLAYERS_POS_P: positionPlayer = L"Midfielder"; break;
                    case PLAYERS_POS_N: positionPlayer = L"Striker"; break;
                }
                wcout << endl << endl << pLang->get(L"Position:") << L" " << pLang->get(positionPlayer) << endl;
                wprintf(
                    pLang->get(L"Skills: G-%d, D-%d, M-%d, A-%d     Goals: %d").c_str(),
                    footballer.data[3],
                    footballer.data[4],
                    footballer.data[5],
                    footballer.data[6],
                    footballer.data[16]
                );

                wcout << endl << endl;
                const wstring morale = pSquad->getMorale(footballer.data[7]);
                wprintf(
                    pLang->get(L"Morale: %ls    Form: %d    Condition: %d%%").c_str(),
                    morale.c_str(),
                    footballer.data[9],
                    footballer.data[11]
                );
                wcout << endl << endl;
                wcout << pLang->get(L"Yellow cards:") << L" ";
                pColors->textColor(YELLOW);
                wcout << footballer.data[13];
                pColors->textColor(LIGHTGRAY);
                wcout << L"    " << pLang->get(L"Red cards:") << L" ";
                pColors->textColor(RED);
                wcout << footballer.data[14];

                if (footballer.data[15] > 0) {
                    pColors->textColor(LIGHTBLUE);
                    wcout << endl << endl << pLang->get(L"Number of days to heal the injury: ") << footballer.data[15];
                }
                pColors->textColor(LIGHTGRAY);
                wcout << endl << endl;
                wprintf(pLang->get(L"The contract will expire in: %d day(s)").c_str(), footballer.data[18]);
                wcout << endl;
                wprintf(pLang->get(L"Footballer value: $%.2f").c_str(), footballer.finances[0]);
                wcout << endl;
                wprintf(pLang->get(L"Monthly wage: $%.2f").c_str(), footballer.finances[1]);

                if (footballer.data[2] == PLAYERS_POS_P ||
                    footballer.data[2] == PLAYERS_POS_N
                ) {
                    wcout << endl;
                    wprintf(pLang->get(L"Goal bonus:: $%.2f").c_str(), footballer.finances[2]);
                }
                if (footballer.data[17] == 1) {
                    pColors->textColor(LIGHTBLUE);
                    wcout << endl << endl;
                    wprintf(pLang->get(L"Footballer for sale for $%.2f").c_str(), footballer.finances[3]);
                    pColors->textColor(LIGHTGRAY);
                }
                pColors->textColor(GREEN);
                wcout << endl << endl <<
                    pLang->get(L"K New contract") << endl <<
                    pLang->get(L"T For sale/Cancel") << endl <<
                    pLang->get(L"W Expulsion from the club");

                pColors->textColor(RED);
                wcout << endl << pLang->get(L"Q Back") << endl;

                pColors->textColor(GREEN);
                menuDetails = pInput->getKeyboardPressed();
                switch (menuDetails) {
                    case 'W': { // remove footballer from the club
                        info();
                        pColors->textColor(LIGHTGRAY);
                        wcout << endl << endl;
                        wprintf(
                            pLang->get(L"Are you sure you want to kick %ls%ls from the club? (Y/n): ").c_str(),
                            footballer.name,
                            footballer.surname
                        );
                        wchar_t yn = pInput->getKeyboardPressed();
                        if (yn == pLang->getYesKeyboard() || yn == L'\n') {
                            wcout << endl;
                            wprintf(
                                pLang->get(L"Tomorrow, the contract of %ls%ls will expire.").c_str(),
                                footballer.name,
                                footballer.surname
                            );
                            footballer.data[18] = 1;
                            pInput->getKeyboardPressed();
                        }
                        break;
                    }
                    case 'T': { // footballer on sale or cancel it
                        if (footballer.data[17] == 0) {
                            info();

                            pColors->textColor(WHITE);
                            wcout << endl << endl;
                            wprintf(pLang->get(L"Sales: %ls%ls").c_str(), footballer.name, footballer.surname);

                            pColors->textColor(LIGHTGRAY);
                            wcout << endl << endl;
                            wprintf(pLang->get(L"Enter the price (%.2f): ").c_str(), footballer.finances[0]);

                            footballer.finances[3] = pInput->getFloat();
                            if (footballer.data[7] > 0) {
                                footballer.data[7]--;
                                footballer.data[8] = 0;
                                if (footballer.data[7] < PLAYERS_MORALE_FATAL) {
                                    footballer.data[7] = PLAYERS_MORALE_FATAL;
                                }
                            }
                            footballer.data[17] = 1;
                        }
                        else if (footballer.data[17] == 1) {
                            footballer.data[17] = 0;
                        }
                        break;
                    }
                    case 'K': { // new contract for footballer
                        int weight = 0;
                        int mode = 0;
                        info();
                        pColors->textColor(WHITE);
                        wcout << endl << endl;
                        wprintf(pLang->get(L"Contract for: %ls%ls").c_str(), footballer.name, footballer.surname);

                        pColors->textColor(LIGHTGRAY);
                        wcout << endl << endl;
                        wprintf(pLang->get(L"Enter the monthly wage (%.2f): ").c_str(), footballer.finances[1]);

                        float salary = pInput->getFloat();
                        float bonus = 0;
                        if (footballer.data[2] == PLAYERS_POS_P ||
                            footballer.data[2] == PLAYERS_POS_N
                        ) {
                            wcout << endl;
                            wprintf(pLang->get(L"Enter the goal bonus (%.2f): ").c_str(), footballer.finances[2]);
                            bonus = pInput->getFloat();
                        }

                        wcout << endl << pLang->get(L"Enter for how many years the contract will apply (1, 2 or 3): ");
                        int contractYears = pInput->getNumber();

                        if (footballer.data[2] == PLAYERS_POS_P ||
                            footballer.data[2] == PLAYERS_POS_N
                        ) {
                            float transfer = bonus - footballer.finances[2];

                            if (transfer == 0) weight++;
                            if (transfer >= 50 && transfer < 100) weight += 2;
                            if (transfer >= 100) weight += 3;
                            if (transfer < 0) weight--;
                        }
                        else  {
                            weight++;
                        }
                        float transfer = salary - footballer.finances[1];
                        if (transfer < 0) weight--;
                        if (transfer < -1000) weight -= 3;
                        if (transfer == 0) weight++;
                        if (transfer >= 100 && transfer < 500) weight += 2;
                        if (transfer >= 500 && transfer < 1000) weight += 3;
                        if (transfer >= 1000) weight += 4;

                        if (contractYears == 1) weight += 2;
                        if (contractYears == 2) weight++;
                        if (contractYears < 1 || contractYears > 3) weight = -3;

                        int sumSkills = footballer.data[3] + footballer.data[4] + footballer.data[5] + footballer.data[6];
                        if (footballer.data[2] == PLAYERS_POS_B && sumSkills > 30) weight -= 2;
                        if (sumSkills >= 30) weight--;
                        if (sumSkills >= 50) weight -= 2;
                        if (sumSkills < 20) weight++;
                        if (sumSkills < 10) weight += 2;
                        if (footballer.data[17] == 1) weight++;

                        if (weight >= 4) {
                            pColors->textColor(GREEN);
                            wcout << endl;
                            wprintf(
                                pLang->get(L"%ls: I agree to the proposed terms of the contract.").c_str(),
                                footballer.surname
                            );
                            pColors->textColor(LIGHTGRAY);
                            wcout << endl << endl << pLang->get(L"Do you accept? (Y/n): ");
                            wchar_t yn = pInput->getKeyboardPressed();
                            if (yn == pLang->getYesKeyboard() || yn == L'\n') {
                                footballer.finances[1] = salary;
                                if (footballer.data[2] == PLAYERS_POS_P ||
                                    footballer.data[2] == PLAYERS_POS_N
                                ) {
                                    footballer.finances[2] = bonus;
                                }
                                footballer.data[18] = contractYears * 365;
                                if (footballer.data[17] == 1) {
                                    footballer.data[17] = 0;
                                }
                                if (mode >= 500 && mode < 1000) { // TODO: tutaj jest jakiś błąd, tak wysoki mode nigdzie nie jest ustawiany?
                                    footballer.data[8] += 3; // mini morale up
                                    if (footballer.data[8] >= 5) {
                                        footballer.data[7]++; // moral up
                                        footballer.data[8] = 0;
                                        if (footballer.data[7] > PLAYERS_MORALE_SUPER) {
                                            footballer.data[7] = PLAYERS_MORALE_SUPER;
                                        }
                                    }
                                }
                                if (mode >= 1000) {
                                    footballer.data[7]++; // morale up
                                    footballer.data[8] = 0;
                                    if (footballer.data[7] > PLAYERS_MORALE_SUPER) {
                                        footballer.data[7] = PLAYERS_MORALE_SUPER;
                                    }
                                }
                            }
                        }
                        else {
                            pColors->textColor(RED);
                            wcout << endl;
                            wprintf(
                                pLang->get(L"%ls: I reject the proposed terms of the contract.").c_str(),
                                footballer.surname
                            );
                            pInput->getKeyboardPressed();
                        }
                        break;
                    }
                }// dla switch
            }//dla do
            while (menuDetails != 'Q');
        }
    }

    pFootballers->savePlayerTeam();
}

void Manager::menuItemTraining()
{
    SClub &clubRef = pClub->get();

    if (clubRef.clubId > 0) {
        wchar_t trainingMenu = 0;
        const wstring weekDays[7] = {
            pLang->get(L"Mon"),
            pLang->get(L"Tue"),
            pLang->get(L"Wed"),
            pLang->get(L"Thu"),
            pLang->get(L"Fri"),
            pLang->get(L"Sat"),
            pLang->get(L"Sun")
        };
        const int maxWhatTraining = 10;
        const wstring whatTrain[10] = {
            pLang->cut(pLang->get(L"Condition"), maxWhatTraining),
            pLang->cut(pLang->get(L"Passing"), maxWhatTraining),
            pLang->cut(pLang->get(L"Free kicks"), maxWhatTraining),
            pLang->cut(pLang->get(L"Tactics"), maxWhatTraining),
            pLang->cut(pLang->get(L"Goalkeeper"), maxWhatTraining),
            pLang->cut(pLang->get(L"Defence"), maxWhatTraining),
            pLang->cut(pLang->get(L"Midfield"), maxWhatTraining),
            pLang->cut(pLang->get(L"Attack"), maxWhatTraining),
            pLang->cut(pLang->get(L"Day off"), maxWhatTraining),
            L" "
        };

        do {
            info();
            pColors->textColor(LIGHTGRAY);
            wcout << endl;
            wcout << BOX_LIGHT_DOWN_RIGHT;
            for (int j = 0; j < 3; j++) {
                for (int i = 0; i < 10; i++) {
                    wcout << BOX_LIGHT_HORIZONTAL;
                }
                wcout << BOX_LIGHT_DOWN_HORIZONTAL;
            }
            wcout << BOX_LIGHT_HORIZONTAL;
            pColors->textColor(WHITE);
            const int maxLabel = 8;
            const wstring label = pLang->cut(pLang->get(L"TRAINING"), maxLabel);
            wcout << label;
            pColors->textColor(LIGHTGRAY);
            // draw extra horizontal line when "TRAINING" in another language is shorter
            for (int i = label.length(); i < maxLabel; ++i) {
                wcout << BOX_LIGHT_HORIZONTAL;
            }
            wcout << BOX_LIGHT_HORIZONTAL;
            wcout << BOX_LIGHT_DOWN_HORIZONTAL;
            for (int i = 0; i < 10; i++) {
                wcout << BOX_LIGHT_HORIZONTAL;
            }
            wcout << BOX_LIGHT_DOWN_HORIZONTAL;
            for (int i = 0; i < 10; i++) {
                wcout << BOX_LIGHT_HORIZONTAL;
            }
            wcout << BOX_LIGHT_DOWN_HORIZONTAL;
            for (int i = 0; i < 10; i++) {
                wcout << BOX_LIGHT_HORIZONTAL;
            }
            wcout << BOX_LIGHT_DOWN_LEFT;
            wcout << endl;

            //pColors->textColor(GREEN);
            wcout << BOX_LIGHT_VERTICAL;
            for (int i = 0; i < 7; i++) {
                pColors->textColor(GREEN);
                wprintf(L"  %d %-5ls ", i + 1, weekDays[i].c_str());
                pColors->textColor(LIGHTGRAY);
                wcout << BOX_LIGHT_VERTICAL;
            }
            wcout << endl;

            pColors->textColor(LIGHTGRAY);
            wcout << BOX_LIGHT_VERTICAL_RIGHT;
            for (int j = 0; j < 6; j++) {
                for (int i = 0; i < 10; i++) {
                    wcout << BOX_LIGHT_HORIZONTAL;
                }
                wcout << BOX_LIGHT_CROSS;
            }
            for (int i = 0; i < 10; i++) {
                wcout << BOX_LIGHT_HORIZONTAL;
            }
            wcout << BOX_LIGHT_VERTICAL_LEFT << endl << BOX_LIGHT_VERTICAL;

            int ilex[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

            for (int i = 0; i < TRAINING_SLOTS_NUMBER; i++) {
                if (i == 7 || i == 14 || i == 21 || i == 28) {
                    wcout << endl << BOX_LIGHT_VERTICAL;
                }

                for (int k = 0; k < 10; k++) {
                    if (clubRef.training[i] == k + 1) {
                        ilex[k]++;
                        wprintf(L"%-10ls", whatTrain[k].c_str());
                        wcout << BOX_LIGHT_VERTICAL;
                    }
                }
            }
            wcout << endl << BOX_LIGHT_UP_RIGHT;
            for (int j = 0; j < 6; j++) {
                for (int i = 0; i < 10; i++) {
                    wcout << BOX_LIGHT_HORIZONTAL;
                }
                wcout << BOX_LIGHT_UP_HORIZONTAL;
            }
            for (int i = 0; i < 10; i++) {
                wcout << BOX_LIGHT_HORIZONTAL;
            }
            wcout << BOX_LIGHT_UP_LEFT;

            wcout << endl << pLang->get(L"Intensywność:               Pkt:");
            wcout << endl;
            wprintf(pLang->get(L" G, D, M, A: .........%3d%% %5d").c_str(), ilex[4] * 3 * 100 / 21, clubRef.treBOPN);
            wcout << endl;
            wprintf(pLang->get(L" Condition: ..........%3d%% %5.1f").c_str(), ilex[0] * 100 / 21, clubRef.trained[0]);
            wcout << endl;
            wprintf(pLang->get(L" Passing: ............%3d%% %5.1f").c_str(), ilex[1] * 100 / 21, clubRef.trained[1]);
            wcout << endl;
            wprintf(pLang->get(L" Free kicks: .........%3d%% %5.1f").c_str(), ilex[2] * 100 / 21, clubRef.trained[2]);
            wcout << endl;
            wprintf(pLang->get(L" Tactics: ............%3d%% %5.1f").c_str(), ilex[3] * 100 / 21, clubRef.trained[3]);
            wcout << endl;
            wprintf(pLang->get(L" Day off: ............%3d%%").c_str(), ilex[8] * 3 * 100 / 21);

            pColors->textColor(GREEN);
            wcout << endl <<
                pLang->get(L" 1-7 Change of weekday schedule") << endl <<
                pLang->get(L" D   Selection of players for individual training");

            pColors->textColor(RED);
            wcout << endl << L" " << pLang->get(L"Q Back to MENU") << endl;
            trainingMenu = pInput->getKeyboardPressed();
            switch (trainingMenu) {
                case L'1':
                case L'2':
                case L'3':
                case L'4':
                case L'5':
                case L'6':
                case L'7': {
                    menuItemTrainingWeekDay(trainingMenu, ilex);
                    break;
                }
                case 'D': { // kto ma trenować B,O,P,N
                    menuItemTrainingIndividual();
                    break;
                }
            }
        }
        while (trainingMenu != 'Q');
    }
}

void Manager::menuItemTrainingWeekDay(wchar_t trainingMenu, int *ilex)
{
    SClub &clubRef = pClub->get();
    int day = wcstol(&trainingMenu, NULL, 10);

    pColors->textColor(LIGHTGRAY);
    wprintf(pLang->get(L"Change training plan for %ls").c_str(), getTrainingDayName(day).c_str());
    wcout << endl << pLang->get(L"1. Individual training (G, D, M, A)");
    wcout << endl << pLang->get(L"2. Team training");
    wcout << endl << pLang->get(L"3. Day off") << endl;

    wchar_t trainingPlanMenu = pInput->getKeyboardPressed();
    switch (trainingPlanMenu) {
        case '1': {//B,O,P,N
            for (int i = day - 1, j = 1; i < TRAINING_SLOTS_NUMBER; i += 7, ++j) {
                if (j == 1) clubRef.training[i] = TRAINING_B;
                if (j == 2) clubRef.training[i] = TRAINING_O;
                if (j == 3) clubRef.training[i] = TRAINING_P;
                if (j == 4) clubRef.training[i] = TRAINING_N;
            }
            pClub->save();
            break;
        }
        case  '3': { //wolne
            for (int i = day - 1, j = 1; i < TRAINING_SLOTS_NUMBER; i += 7, ++j) {
                if (j == 1) clubRef.training[i] = TRAINING_HOLIDAY;
                if (j == 2) clubRef.training[i] = TRAINING_EMPTY;
                if (j == 3) clubRef.training[i] = TRAINING_EMPTY;
                if (j == 4) clubRef.training[i] = TRAINING_EMPTY;
            }
            pClub->save();
            break;
        }
        case '2': {//drużynowy
            ilex[0] = 0;
            ilex[1] = 0;
            ilex[2] = 0;
            ilex[3] = 0;
            ilex[4] = 0;

            int slotNumber = 0; // <- ile obsadzono rodzi treningu, max 3 z 4 dostępnych
            while (slotNumber != 4) {
                slotNumber++;
                if (slotNumber < 4) {
                    pInput->clrscr();
                    pColors->textColor(WHITE);
                    wcout << endl << pLang->get(L"TEAM TRAINING");

                    pColors->textColor(LIGHTGRAY);
                    wcout << endl << pLang->get(L"You need to fill 3 types of training.");

                    pColors->textColor(GREEN);
                    for (int i = 0; i < 4; i++) {
                        if (ilex[i] == 1) {
                            wcout << endl << pLang->get(L"Selected:") << L" " << 1 << L". " << pLang->get(L"Condition");
                        }
                        if (ilex[i] == 2) {
                            wcout << endl << pLang->get(L"Selected:") << L" " << 2 << L". " << pLang->get(L"Passing");
                        }
                        if (ilex[i] == 3) {
                            wcout << endl << pLang->get(L"Selected:") << L" " << 3 << L". " << pLang->get(L"Free kicks");
                        }
                        if (ilex[i] == 4) {
                            wcout << endl << pLang->get(L"Selected:") << L" " << 4 << L". " << pLang->get(L"Tactics");
                        }
                    }
                    pColors->textColor(LIGHTGRAY);
                    wcout << endl;
                    wprintf(pLang->get(L"What do you want to train? (%d remaining):").c_str(), 4 - slotNumber);
                    wcout << endl << 1 << L". " << pLang->get(L"Condition");
                    wcout << endl << 2 << L". " << pLang->get(L"Passing");
                    wcout << endl << 3 << L". " << pLang->get(L"Free kicks");
                    wcout << endl << 4 << L". " << pLang->get(L"Tactics") << endl;

                    pColors->textColor(GREEN);
                    ilex[slotNumber - 1] = pInput->getNumber();
                    if (ilex[slotNumber - 1] < TRAINING_CONDITIONS ||
                        ilex[slotNumber - 1] > TRAINING_TACTICS
                    ) {
                        slotNumber--; // błędny wybór
                    }
                }

                int index = day - 1 + (7 * (slotNumber - 1));
                switch (ilex[slotNumber - 1]) {
                    case TRAINING_CONDITIONS: {
                        clubRef.training[index] = TRAINING_CONDITIONS;
                        break;
                    }
                    case TRAINING_PASSES: {
                        clubRef.training[index] = TRAINING_PASSES;
                        break;
                    }
                    case TRAINING_FREE_KICKS: {
                        clubRef.training[index] = TRAINING_FREE_KICKS;
                        break;
                    }
                    case TRAINING_TACTICS: {
                        clubRef.training[index] = TRAINING_TACTICS;
                        break;
                    }
                }

                if (slotNumber == 4) {
                    clubRef.training[day - 1 + (7 * 3)] = TRAINING_EMPTY;
                }
            }//dla while
            pClub->save();
            break;
        }
    }
}

void Manager::menuItemTrainingIndividual()
{
    SClub &clubRef = pClub->get();
    wchar_t menuPersonalTraining = 0;
    bool isNext = false;
    int mode = 0;
    do {
        info();
        pColors->textColor(WHITE);
        wcout << endl << pLang->get(L"SELECTION OF PLAYERS FOR INDIVIDUAL TRAINING");
        mode = isNext ? 40 : 20;

        pSquad->draw(pFootballers->getPlayersTeam(), clubRef.teamSetting, mode, clubRef.clubId, true);

        size_t playersCount = pFootballers->getSizePlayerTeam();

        pColors->textColor(GREEN);
        wcout << endl;
        if (playersCount > 20) {
            wprintf(L"%-10ls", pLang->get(L"D Next").c_str());
        }
        wcout << pLang->get(L"T Change training");

        pColors->textColor(RED);
        wcout << L"  " << pLang->get(L"Q Back");
        pColors->textColor(GREEN);

        menuPersonalTraining = pInput->getKeyboardPressed();
        switch (menuPersonalTraining) {
            case 'D': { // dalej
                isNext = (!isNext && playersCount > 20) ? true : false;
                break;
            }
            case 'T': { // zmiana treningu
                pColors->textColor(LIGHTGRAY);
                wcout << pLang->get(L" Enter No: ");
                int footballerId = pInput->getNumber();

                wcout << pLang->get(L" What training? (1-G, 2-D, 3-M, 4-A): ");
                int training = pInput->getNumber();

                if (training < PLAYERS_TRAINING_B ||
                    training > PLAYERS_TRAINING_N
                ) {
                    break;
                }

                for (size_t i = 0; i < pFootballers->getSizePlayerTeam(); i++) {
                    SFootballer &footballer = pFootballers->getPlayerTeam(i);
                    if (footballer.data[0] == footballerId) {
                        footballer.data[1] = training;
                    }
                    if (footballer.data[15] > 0) {
                        footballer.data[1] = PLAYERS_TRAINING_NONE;
                    }
                }
                pFootballers->savePlayerTeam();
                break;
            }
        }
    }
    while (menuPersonalTraining != 'Q');
}

void Manager::menuItemLastMatch()
{
    SClub &clubRef = pClub->get();
    if (clubRef.lastMatchRivalId <= 0) {
        pInput->clrscr();

        pColors->textColor(LIGHTGRAY);
        wcout << pLang->get(L"LAST MATCH REPORT");

        pColors->textColor(RED);
        wcout << endl << endl << pLang->get(L"There are no matches played yet") << endl << endl;
        pColors->textColor(LIGHTGRAY);
        wcout << endl << endl << pLang->get(L"Press any key...");
        pInput->getKeyboardPressed();
        return;
    }

    // wczytaj raport z meczu do pamieci
    vector<SLastMatch> lastMatchArray;
    FILE *f = fopen(FILE_SAVE_LAST_MATCH_REPORTS, "r");
    SLastMatch lastMatch;
    while (fread(&lastMatch, sizeof (SLastMatch), 1, f) == 1) {
        lastMatchArray.push_back(lastMatch);
    }
    fclose(f);

    // append end of match text
    lastMatch.textColor = LIGHTGRAY;
    wcscpy(lastMatch.text, pLang->get(L"End of the match").c_str());
    lastMatchArray.push_back(lastMatch);

    int startFrom = 0;
    const int maxCount = 19;
    const int pages = ceil(lastMatchArray.size() / (float)maxCount);

    wchar_t keyPressed = 0;
    do {
        pInput->clrscr();
        pColors->textColor(LIGHTGRAY);
        wcout << pLang->get(L"LAST MATCH REPORT") << L": ";
        pColors->textColor(LIGHTBLUE);
        wcout << L" " << pClub->getClubName(clubRef.clubId - 1) << L" " << clubRef.playerGoals << L" ";
        pColors->textColor(RED);
        wcout << L" " << clubRef.rivalGoals << L" " << pClub->getClubName(clubRef.lastMatchRivalId - 1) << L" ";
        wcout << endl;

        // wczytujemy max 19 raportow
        for (size_t index = startFrom, counter = 0;
            index < lastMatchArray.size() && counter < maxCount;
            ++index, ++counter
        ) {
            const SLastMatch &lastMatch = lastMatchArray[index];
            pColors->textColor(lastMatch.textColor);
            wcout << endl << lastMatch.text;
        }

        wcout << endl;

        pColors->textBackground(BLACK);
        pColors->textColor(GREEN);
        wcout << endl << L"<- " << pLang->get(L"Prvious page")
                << L" (" << (startFrom / maxCount) + 1 << "/" << pages << ") "
                << pLang->get(L"Next page") << L" ->";

        pColors->textColor(RED);
        wcout << endl << pLang->get(L"Q Quit") << L" ";

        pColors->textColor(GREEN);
        keyPressed = pInput->getKeyboardPressed();
        switch (keyPressed) {
            case _KEY_RIGHT:
            case L'>':
            case L'.':
            case L' ':
            case L'\n': {
                // prev page
                startFrom += maxCount;
                if (startFrom > lastMatchArray.size()) {
                    startFrom = 0;
                }
                break;
            }
            case _KEY_LEFT:
            case L'<':
            case L',': {
                // next page
                startFrom -= maxCount;
                if (startFrom < 0) {
                    startFrom = (pages - 1) * maxCount;
                }
                break;
            }
        }
    }
    while (keyPressed != 'Q');
}

void Manager::menuItemRival()
{
    SClub &clubRef = pClub->get();
    if (clubRef.clubId > 0) {
        wchar_t menu = 0;
        do {
            pInput->clrscr();
            pColors->textColor(WHITE);
            wcout << pLang->get(L"THE NEAREST OPPONENT - ");

            int opponentClubId = pRounds->getNearestRivalId(clubRef.roundNumber, clubRef.clubId);
            wcout << pClub->getClubName(opponentClubId);

            int tablePos = pTable->getPositionInTable(clubRef.rivalData[0]);

            wcout << L" " << tablePos << L" " << pLang->get(L"place");
            wcout << L" " << (clubRef.rivalData[1] == 0 ? pLang->get(L"(Home)") : pLang->get(L"(Away)"));

            pTactic->drawTeamSetting(clubRef.rivalData[2], false);

            pTactic->drawChart(clubRef.rivalData[2], clubRef.rivalData[0], pFootballers->getRivals(), true);
            pTactic->drawChart(clubRef.teamSetting, clubRef.clubId, pFootballers->getPlayersTeam());

            pSquad->draw(pFootballers->getRivals(), clubRef.rivalData[2], 11, clubRef.rivalData[0]);

            menu = pInput->getKeyboardPressed();
            switch (menu) {
                case 'R': {
                    pInput->clrscr();

                    pColors->textColor(WHITE);
                    wcout << endl << pLang->get(L"SUBSTITUTES' BENCH");

                    pSquad->draw(pFootballers->getRivals(), clubRef.rivalData[2], 16, clubRef.rivalData[0]);

                    pColors->textColor(LIGHTGRAY);
                    wcout << endl << endl << pLang->get(L"Press any key...");
                    pInput->getKeyboardPressed();
                    break;
                }
                case 'I': {
                    pInput->clrscr();
                    pColors->textColor(WHITE);
                    wcout << endl << pLang->get(L"Team instructions:");

                    pTeamInstr->draw(
                        clubRef.rivalInst[0],
                        clubRef.rivalInst[1],
                        clubRef.rivalInst[2],
                        clubRef.rivalInst[3],
                        clubRef.rivalInst[4],
                        clubRef.rivalInst[5],
                        0
                    );

                    pColors->textColor(LIGHTGRAY);
                    wcout << endl << endl << pLang->get(L"Press any key...");
                    pInput->getKeyboardPressed();
                    break;
                }
            }
        }
        while (menu != 'Q');
    }
}

void Manager::menuItemCalendar()
{
    SClub &clubRef = pClub->get();

    if (clubRef.clubId > 0) {
        int mode = 0;
        bool isSpring = false;
        int roundNum = 0;

        if (clubRef.month < 7) {
            //runda wiosenna, pokaż kolejki od 16 do 30
            roundNum = MAX_CLUBS - 1;
            mode = roundNum * 2;
            isSpring = true;
        }
        else {
            //runda jesienna, pokaż kolejki od 1 do 15
            roundNum = 0;
            mode = MAX_CLUBS - 1;
            isSpring = false;
        }

        wchar_t menu = 0;
        do {
            if (!isSpring) { // runda jesienna, 1-15
                roundNum = 0;
                mode = MAX_CLUBS - 1;
            }
            else { // runda wiosenna, 16-30
                roundNum = MAX_CLUBS - 1;
                mode = roundNum * 2;
            }

            info();

            pColors->textColor(WHITE);
            wcout << endl << pLang->get(L"CALENDAR") << L" - " << pLang->get(isSpring ? L"spring round" : L"autumn round");

            pColors->textColor(LIGHTGRAY);
            if (clubRef.controlMatchesAmount <= 0 && !isSpring) {
                // mamy mecze kontrolne, dodaj je do rundy
                roundNum = clubRef.controlMatchesAmount - 1;
            }

            while (roundNum != mode) {
                roundNum++;
                int myClubIndex = 0;
                int oponentIndex = 0;

                myClubIndex = pRounds->getClubIndexInRoundByClubId(roundNum, clubRef.clubId);

                pColors->textColor(roundNum <= 0 ? MAGENTA : LIGHTGRAY);

                bool isHome = myClubIndex % 2 == 0;
                if (isHome) { // u siebie
                    oponentIndex = myClubIndex + 1; // oponent jest następny (+1)
                }
                else { // wyjazd
                    oponentIndex = myClubIndex - 1; // oponent jest poprzedni (-1)
                }

                for (size_t index = 0; index < pRounds->getSize(); index++) {
                    const SRound &round = pRounds->get(index);
                    if (round.number == roundNum) {
                        wcout << endl;
                        wprintf(
                            L"%02d/%02d/%d %-19ls",
                            pLang->getLngCode() == "pl_PL" ? round.day : round.month,
                            pLang->getLngCode() == "pl_PL" ? round.month : round.day,
                            round.year + clubRef.season,
                            pClub->getClubName(round.clubNumbers[oponentIndex] - 1).c_str()
                        );

                    }
                }

                wprintf(L" %-9ls", pLang->get(isHome ? L"Home" : L"Away").c_str());
                wprintf(L" %-11ls", pLang->get(roundNum <= 0 ? L"Friendly" : L"I league").c_str());

                int kk = clubRef.isMatch ? 1 : 0;

                if (!clubRef.isMatch) {
                    for (size_t index = 0; index < pRounds->getSize(); index++) {
                        const SRound &round = pRounds->get(index);
                        if (clubRef.roundNumber == round.number &&
                            clubRef.day != round.day
                        ) {
                            kk++;
                            break;
                        }
                    }
                }

                for (size_t index = 0; index < pRounds->getSize(); index++) {
                    const SRound &round = pRounds->get(index);
                    if (roundNum <= 0 &&
                        roundNum == round.number &&
                        roundNum <= clubRef.roundNumber - kk
                    ) {
                        // mecze kontrolne, rozegrane
                        wprintf(
                            L"%5d - %d",
                            clubRef.goalsControls[(roundNum + 4) * 2],
                            clubRef.goalsControls[(roundNum + 4) * 2 + 1]
                        );
                    }
                    else if (
                        roundNum <= 0 &&
                        roundNum == round.number &&
                        roundNum >= clubRef.roundNumber - kk
                    ) {
                        // mecze kontrolne nierozegrane
                        wcout << L"      - ";
                    }
                    else if (
                        roundNum > 0 &&
                        roundNum == round.number &&
                        roundNum <= clubRef.roundNumber - kk
                    ) {
                        // mecze ligowe, rozegrane
                        wprintf(
                            L"%5d - %d",
                            clubRef.goalsLeague[(roundNum * 16) - 16 + myClubIndex],
                            clubRef.goalsLeague[(roundNum * 16) - 16 + oponentIndex]
                        );
                    }
                    else if (
                        roundNum > 0 &&
                        roundNum == round.number &&
                        roundNum >= clubRef.roundNumber - kk
                    ) {
                        // mecze ligowe nierozegrane
                        wcout << L"      - ";
                    }

                    if (roundNum == clubRef.roundNumber - kk + 1 && roundNum == round.number) {
                        wcout << L" <-";
                    }
                }
            }
            pColors->textColor(GREEN);
            wcout << endl << endl << pLang->get(isSpring ? L"D Show the autumn round" : L"D Show the spring round");
            wcout << L"   " << pLang->get(L"M Friendly matches");

            pColors->textColor(RED);
            wcout << L"   " << pLang->get(L"Q Back to MENU");

            pColors->textColor(GREEN);
            menu = pInput->getKeyboardPressed();
            switch (menu) {
                case 'D': {
                    isSpring = !isSpring;
                    break;
                }
                case 'M': { // mecze kontrolne
                    menuItemCalendarControlMatch();
                    break;
                }
            }
        }
        while (menu != 'Q');
    }
}

void Manager::menuItemCalendarControlMatch()
{
    SClub &clubRef = pClub->get();

    if (clubRef.month == 7) { // 7 miesiac
        pColors->textColor(LIGHTGRAY);
        wcout << endl << pLang->get(L"Enter the day, space, month - friendly match: ");
        int wishDay = 0;
        int wishMonth = 0;
        pInput->getNumbersExchange(wishDay, wishMonth);

        bool canPlayControlMatch = true;
        for (size_t index = 0; index < pRounds->getSize(); index++) {
            const SRound &round = pRounds->get(index);
            if (round.number == clubRef.controlMatchesAmount &&
                round.day <= wishDay &&
                round.month <= wishMonth
            ) {
                canPlayControlMatch = false;
                break;
            }
        }

        if (wishMonth < clubRef.month ||
            (wishDay == clubRef.day && wishMonth == clubRef.month) ||
            wishDay > 31
        ) {
            canPlayControlMatch = false;
        }

        info();

        pColors->textColor(LIGHTGRAY);
        wcout << endl << endl << pLang->get(L"Choose a sparring partner:");
        for (int i = 0; i < MAX_CLUBS; i++) {
            if (i + 1 != clubRef.clubId) {
                wprintf(L"\n\r%2d. %ls", i + 1, pClub->getClubName(i).c_str());
            }
        }
        wcout << endl << endl << pLang->get(L"Enter the appropriate number: ");
        int rivalClubId = pInput->getNumber();
        if (rivalClubId < 0 ||
            rivalClubId > 16 ||
            rivalClubId == clubRef.clubId ||
            clubRef.controlMatchesAmount < -3 // max 4 mecze kontrolne?
        ) {
            canPlayControlMatch = false;
        }

        if (!canPlayControlMatch) {
            pColors->textColor(RED);
            wcout << endl << pLang->get(L"The offer has been rejected.");
            pInput->getKeyboardPressed();
        }
        else if (canPlayControlMatch) {
            clubRef.controlMatchesAmount--;
            clubRef.roundNumber--;
            clubRef.isRivalSet = 0;

            SRound round = {0};
            round.day = wishDay;
            round.month = wishMonth;
            round.year = 2002;
            round.number = clubRef.controlMatchesAmount; //numer kolejki, liczba ujemna
            round.clubNumbers[0] = rivalClubId;
            round.clubNumbers[1] = clubRef.clubId;

            for (int i = 0; i < 2; i++) {
                const wstring clubName = pClub->getClubName(round.clubNumbers[i] - 1);
                wcscpy(round.kol1[i], clubName.c_str());
            }

            pRounds->insert(round);
            pRounds->save();
            pClub->save();
        }
    } // dla miesiąca 7
    else {
        pColors->textColor(LIGHTRED);
        wcout << endl << pLang->get(L"A friendly match cannot be played.") << endl;
        sleep(2);
    }
}

void Manager::menuItemTable()
{
    SClub &clubRef = pClub->get();
    wchar_t menu = 0;
    do {
        pInput->clrscr();
        pTable->drawTable(clubRef.clubId, pClub);

        pColors->textColor(GREEN);
        wcout << endl << pLang->get(L" W Results   P Remaining matches");

        pColors->textColor(RED);
        wcout << L"    " << pLang->get(L"Q Back to MENU") << L" ";

        bool isShowPlayed = false; // 0 - tryb mecze nie rozegrane, 1 - rozegrane
        int kk = clubRef.isMatch ? 1 : 0;

        if (!clubRef.isMatch) {
            for (size_t index = 0; index < pRounds->getSize(); index++) {
                const SRound &round = pRounds->get(index);
                if (clubRef.roundNumber == round.number &&
                    clubRef.day != round.day
                ) {
                    kk++;
                    break;
                }
            }
        }
        // o co chodzi z kk?, na tym etapie czy jest mecz, czy go nie ma, kk = 1

        int roundNumber = clubRef.roundNumber - kk;
        int maxRounds = 1;

        menu = pInput->getKeyboardPressed();
        switch (menu) {
            case 'W': { // wyniki
                maxRounds = 1;
                isShowPlayed = true; // tryb - mecze rozegrane
                roundNumber++; // zwiększ jeszcze o jeden, bo zaraz w pętli while zmniejszysz o 1
                if (clubRef.roundNumber <= 0) {
                    break;
                }
                // break; // <- NO break!
            }
            case 'P': { // pozostałe mecze
                if (!isShowPlayed) {
                    maxRounds = 30; // 30 kolejek, 2x po 15 meczy
                }

                while (roundNumber != maxRounds) {
                    if (isShowPlayed) {
                        roundNumber--; // dla meczy rozegranych, cofamy się
                    }
                    else {
                        roundNumber++; // dla meczy nie rozegranych, idziemy do przodu
                    }

                    info();

                    wcout << endl << endl;
                    if (isShowPlayed) {
                        wcout << pLang->get(L"  Matches results of the I League - ");
                    }
                    else {
                        wcout << pLang->get(L"  Remaining matches of the I League - ");
                    }
                    wcout << roundNumber << pLang->get(L" round");

                    pColors->textColor(LIGHTGRAY);

                    for (int i = 0; i < MAX_CLUBS; i++) {
                        for (size_t index = 0; index < pRounds->getSize(); index++) {
                            const SRound &round = pRounds->get(index);
                            if (roundNumber == round.number) {
                                pColors->textColor(round.clubNumbers[i] == clubRef.clubId ? GREEN : LIGHTGRAY);

                                int prevClubIndex = round.clubNumbers[i] - 1;
                                if (isShowPlayed) {
                                    if (i % 2 == 0) {
                                        wprintf(
                                            L"\n\r%19ls %d -",
                                            pClub->getClubName(prevClubIndex).c_str(),
                                            clubRef.goalsLeague[roundNumber * 16 - 16 + i]
                                        );
                                    }
                                    else {
                                        wprintf(
                                            L" %d %ls",
                                            clubRef.goalsLeague[roundNumber * 16 - 16 + i],
                                            pClub->getClubName(prevClubIndex).c_str()
                                        );
                                    }
                                }
                                else {
                                    if (i % 2 == 0) {
                                        wprintf(L"\n\r%19ls -", pClub->getClubName(prevClubIndex).c_str());
                                    }
                                    else {
                                        wprintf(L" %ls", pClub->getClubName(prevClubIndex).c_str());
                                    }
                                }

                                break;
                            }
                        }
                    }
                    pColors->textColor(GREEN);
                    wcout << endl << endl << pLang->get(L" Next - any key");

                    pColors->textColor(RED);
                    wcout << endl << L" " << pLang->get(L"Q Quit") << L" ";

                    wchar_t yn = pInput->getKeyboardPressed();
                    if (yn == 'Q') {
                        break;
                    }
                }
                break;
            }
        }//dla switch
    }
    while (menu != 'Q');
}

void Manager::menuItemTransfersList()
{
    SClub &clubRef = pClub->get();
    if (clubRef.clubId > 0) {
        filterMaxPrice = clubRef.finances[13];
        int loop = 0;
        int mode = 20;
        wchar_t menu = 0;

        do {
            size_t transferPlayersCount = pFootballers->getSizeTransfers();

            info();

            wcout << endl << pLang->get(L"TRANSFER LIST");

            pColors->textColor(clubRef.finances[13] > 0 ? LIGHTGREEN : LIGHTRED);
            wcout << L"   ";
            wprintf(pLang->get(L"Funds for transfers: $%.2f").c_str(), clubRef.finances[13]);

            pColors->textColor(GREEN);
            wcout << endl << pLang->get(L" No    Footballer       Po.  ");

            pColors->textColor(LIGHTBLUE);
            wcout << pLang->get(L"G") << L"  ";

            pColors->textColor(MAGENTA);
            wcout << pLang->get(L"D") << L"  ";

            pColors->textColor(LIGHTCYAN);
            wcout << pLang->get(L"M") << L"  ";

            pColors->textColor(LIGHTGREEN);
            wcout << pLang->get(L"A") << L"  ";

            pColors->textColor(GREEN);
            wcout << pLang->get(L"Morale  For. Con.    Price");

            if (loop >= transferPlayersCount) {
                loop = 0;
                mode = 20;
            }

            while (loop != mode) {
                loop++;

                for (size_t index = 0; index < pFootballers->getSizeTransfers(); index++) {
                    SFootballer &footballer = pFootballers->getTransfer(index);
                    if (footballer.data[0] == loop && footballer.data[22] != clubRef.clubId) {
                        bool isFilterPosition = (filterPosition == footballer.data[2] || filterPosition == 0);
                        if (isFilterPosition
                            && (
                                footballer.data[3] >= filterGoalkeeper
                                && footballer.data[4] >= filterDefense
                                && footballer.data[5] >= filterMidfield
                                && footballer.data[6] >= filterAttact
                                && footballer.data[9] >= filterForm
                                && footballer.finances[0] >= filterMinPrice
                                && footballer.finances[0] <= filterMaxPrice
                            )
                        ) {
                            pColors->textColor(WHITE);
                            pColors->textBackground(BLUE);
                        }
                        else {
                            pColors->textColor(LIGHTGRAY);
                            pColors->textBackground(BLACK);
                        }
                        wprintf(L"\n\r%3d.  ", footballer.data[0]);
                        wprintf(
                            pLang->get(L"%3ls%-15ls %ls  %2d %2d %2d %2d  %-7ls  %2d  %3d%% $%8.0f").c_str(),
                            footballer.name,
                            footballer.surname,
                            pSquad->getFootballerPosition(footballer.data[2]).c_str(),
                            footballer.data[3],
                            footballer.data[4],
                            footballer.data[5],
                            footballer.data[6],
                            pSquad->getMorale(footballer.data[7]).c_str(),
                            footballer.data[9],
                            footballer.data[11],
                            footballer.finances[0]
                        );
                        pColors->textColor(LIGHTGRAY);
                        pColors->textBackground(BLACK);

                        break;
                    }
                }
            }
            pColors->textColor(GREEN);

            wcout << endl << pLang->get(L"D Next    F Filters  K Player purchase   ");
            pColors->textColor(RED);
            wcout << pLang->get(L"Q Back to MENU");
            pColors->textColor(GREEN);

            menu = pInput->getKeyboardPressed();
            switch (menu) {
                case 'K': { // kupno zawodnika
                    menuItemTransfersListBuyFootballer(mode);

                    loop = 0;
                    mode = 20;
                    break;
                }
                case 'D': {
                    mode += 20;
                    loop = mode - 20;
                    break;
                }
                case 'F': {
                    loop = 0;
                    mode = 20;
                    menuItemTransfersListFilters();
                    break;
                }
                default: {
                    mode = 20;
                    loop = 0;
                    break;
                }
            }
        }
        while (menu != 'Q');
    }
}

void Manager::menuItemTransfersListBuyFootballer(int mode)
{
    SClub &clubRef = pClub->get();
    if (clubRef.finances[13] > 0) { // masz fundusze
        size_t playersCount = pFootballers->getSizePlayerTeam();

        pColors->textColor(LIGHTGRAY);

        wcout << endl << pLang->get(L"Enter the No: ");
        int footballerId = pInput->getNumber();

        for (size_t index = 0; index < pFootballers->getSizeTransfers(); index++) {
            SFootballer &footballer = pFootballers->getTransfer(index);
            if (footballer.data[0] == footballerId) {
                pInput->clrscr();
                int weight = 0;
                float price = 0;

                wcout << endl;
                wprintf(pLang->get(L"TRANSFERS FUNDS: $%.2f").c_str(), clubRef.finances[13]);
                wcout << endl << endl;
                wprintf(pLang->get(L"Player purchase: %ls%ls").c_str(), footballer.name, footballer.surname);

                wcout << endl << pLang->get(L"Club: ");
                if (footballer.data[22] > 0) {
                    wcout << pClub->getClubName(footballer.data[22] - 1);
                }
                else {
                    wcout << pLang->get(L"none");
                }

                if (footballer.data[22] > 0) {
                    // podaj cenę tylko wtedy gdy zawodnik należy do jakiegoś klubu
                    // bo to kasa dla klubu za wykupienie
                    wcout << endl << endl;
                    wprintf(pLang->get(L"Enter the price (%.2f): ").c_str(), footballer.finances[0]);
                    price = pInput->getFloat();
                    if (price > clubRef.finances[13] || price < 0) {
                        weight -= 10;
                    }
                }
                else {
                    wcout << endl;
                }

                wcout << endl;
                wprintf(pLang->get(L"Enter the monthly wage (%.2f): ").c_str(), footballer.finances[1]);
                float salary = pInput->getFloat();
                float bonus = 0;
                if (footballer.data[2] == PLAYERS_POS_P ||
                    footballer.data[2] == PLAYERS_POS_N
                ) {
                    wcout << endl;
                    wprintf(pLang->get(L"Enter the goal bonus (%.2f): ").c_str(), footballer.finances[2]);
                    bonus = pInput->getFloat();
                }
                wcout << endl << pLang->get(L"Enter for how many years the contract will apply (1, 2 or 3): ");
                int contractYears = pInput->getNumber();
                if (footballer.data[22] > 0) {
                    float transfer = price - footballer.finances[0];

                    if (transfer < 0) weight--;
                    if (transfer < -5000) weight -= 5;
                    if (transfer == 0) weight++;
                    if (transfer >= 1000 && transfer < 8000) weight += 2;
                    if (transfer >= 8000 && transfer < 10000) weight += 3;
                    if (transfer >= 10000) weight += 4;
                }
                if (footballer.data[2] == PLAYERS_POS_P ||
                    footballer.data[2] == PLAYERS_POS_N
                ) {
                    float transfer = bonus - footballer.finances[2];

                    if (transfer < 0) weight--;
                    if (transfer == 0) weight++;
                    if (transfer >= 50 && transfer < 100) weight += 2;
                    if (transfer >= 100) weight += 3;
                }
                else {
                    weight++;
                }

                float transfer = salary - footballer.finances[1];

                if (transfer < 0) weight--;
                if (transfer < -1000) weight -= 5;
                if (transfer == 0) weight++;
                if (transfer >= 100 && transfer < 500) weight += 2;
                if (transfer >= 500 && transfer < 1000) weight += 3;
                if (transfer >= 1000) weight += 4;

                if (contractYears == 1) weight += 2;
                if (contractYears == 2) weight++;
                if (contractYears < 1 || contractYears > 3) weight = -5;

                int sumSkills = footballer.data[3] + footballer.data[4] + footballer.data[5] + footballer.data[6];

                if (footballer.data[2] == PLAYERS_POS_B && sumSkills > 30) weight -= 2;
                if (sumSkills >= 30) weight--;
                if (sumSkills >= 50) weight -= 2;
                if (sumSkills < 20) weight++;
                if (sumSkills < 10) weight += 2;
                if (footballer.data[17] == 1) weight++;

                if (weight >= 6) {
                    pColors->textColor(GREEN);

                    wcout << endl;
                    wprintf(pLang->get(L"%ls: I agree to the proposed terms of the contract.").c_str(), footballer.surname);

                    pColors->textColor(LIGHTGRAY);
                    wcout << endl << endl << pLang->get(L"Do you accept? (Y/n): ");
                    wchar_t yn = pInput->getKeyboardPressed();
                    if (yn == pLang->getYesKeyboard() || yn == L'\n') {
                        footballer.data[22] = clubRef.clubId;
                        footballer.finances[1] = salary;
                        footballer.finances[0] = price;
                        if (footballer.data[2] == PLAYERS_POS_P ||
                            footballer.data[2] == PLAYERS_POS_N
                        ) {
                            footballer.finances[2] = bonus;
                        }
                        footballer.data[18] = contractYears * 365;
                        if (footballer.data[17] == 1) {
                            footballer.data[17] = 0;
                        }

                        if (mode >= 500 && mode < 1000) { // TODO: kto ustawia tak wysokie mode? nikt
                            footballer.data[8] += 3;
                            if (footballer.data[8] >= 5) {
                                footballer.data[7]++;
                                footballer.data[8] = 0;
                                if (footballer.data[7] > PLAYERS_MORALE_SUPER) {
                                    footballer.data[7] = PLAYERS_MORALE_SUPER;
                                }
                            }
                        }
                        if (mode >= 1000) {
                            footballer.data[7]++;
                            footballer.data[8] = 0;
                            if (footballer.data[7] > PLAYERS_MORALE_SUPER) {
                                footballer.data[7] = PLAYERS_MORALE_SUPER;
                            }
                        }
                        //zapisanie do drużyny
                        footballer.data[0] = playersCount + 1;
                        clubRef.finances[8] += price;
                        clubRef.finances[12] -= price;
                        clubRef.finances[13] = clubRef.finances[12] / 2;
                        clubRef.finances[10] = clubRef.finances[6] + clubRef.finances[7] + clubRef.finances[8] + clubRef.finances[9];
                        clubRef.finances[11] = clubRef.finances[5] - clubRef.finances[10];
                        clubRef.managerStats[10]++;
                        clubRef.totalExpensesTransfers += price;

                        // TODO: czy tu trzeba usunac zawodnika z listy transferowej bo bedzie zdublowany?
                        pFootballers->pushBackPlayerTeam(footballer);
                        pFootballers->savePlayerTeam();

                        pClub->save();
                    }
                }
                else {
                    pColors->textColor(RED);
                    wcout << endl;
                    wprintf(pLang->get(L"%ls: I reject the proposed terms of the contract.").c_str(), footballer.surname);
                    pInput->getKeyboardPressed();
                }

                break;
            }
        }
        pFootballers->saveTransfers();
    }
    else { // brak funduszy
        pColors->textColor(LIGHTRED);
        wcout << endl << pLang->get(L"No funds for transfers!") << endl;
        sleep(2);
    }
}

void Manager::menuItemTransfersListFilters()
{
    SClub &clubRef = pClub->get();
    int belka = 1;
    wchar_t menuFilters = 0;

    do {
        pInput->clrscr();
        pColors->textColor(WHITE);
        wcout << endl << pLang->get(L"FILTERS FOR TRANSFER LIST");

        pColors->textColor(LIGHTGRAY);
        wcout << endl;
        wprintf(pLang->get(L"Funds for transfers: $%.2f").c_str(), clubRef.finances[13]);

        pColors->textColor(GREEN);

        const wstring position = getFilterByPosition(filterPosition);

        setFilterColors(belka, 1);
        wcout << endl << endl;
        wprintf(pLang->get(L"   Relative to position: %ls ").c_str(), position.c_str());

        setFilterColors(belka, 2);
        wcout << endl;
        const wstring strTemplateText = pLang->get(L"   Relative to skill: %ls: %d ");
        wprintf(strTemplateText.c_str(), pLang->get(L"G").c_str(), filterGoalkeeper);

        setFilterColors(belka, 3);
        wcout << endl;
        wprintf(strTemplateText.c_str(), pLang->get(L"D").c_str(), filterDefense);

        setFilterColors(belka, 4);
        wcout << endl;
        wprintf(strTemplateText.c_str(), pLang->get(L"M").c_str(), filterMidfield);

        setFilterColors(belka, 5);
        wcout << endl;
        wprintf(strTemplateText.c_str(), pLang->get(L"A").c_str(), filterAttact);

        setFilterColors(belka, 6);
        wcout << endl;
        wprintf(pLang->get(L"   Relative to form: %d ").c_str(), filterForm);

        setFilterColors(belka, 7);
        wcout << endl;
        wprintf(pLang->get(L" M Relative to the minimum price: from $%.2f ").c_str(), filterMinPrice);

        setFilterColors(belka, 8);
        wcout << endl;
        wprintf(pLang->get(L" X Relative to the maximum price: from $%.2f ").c_str(), filterMaxPrice);

        pColors->textColor(RED);
        pColors->textBackground(BLACK);
        wcout << endl << L" " << pLang->get(L"Q Back") << endl;

        menuFilters = pInput->getKeyboardPressed();
        switch (menuFilters) {
            case _KEY_UP: {
                if (--belka == 0) {
                    belka = 8;
                }
                break;
            }
            case _KEY_DOWN: {
                if (++belka == 9) {
                    belka = 1;
                }
                break;
            }
            case _KEY_RIGHT: {
                if (belka == 1) {
                    if (++filterPosition == 5) {
                        filterPosition = 0;
                    }
                }
                else if (belka == 2) {
                    if (++filterGoalkeeper == 21) {
                        filterGoalkeeper = 1;
                    }
                }
                else if (belka == 3) {
                    if (++filterDefense == 21) {
                        filterDefense = 1;
                    }
                }
                else if (belka == 4) {
                    if (++filterMidfield == 21) {
                        filterMidfield = 1;
                    }
                }
                else if (belka == 5) {
                    if (++filterAttact == 21) {
                        filterAttact = 1;
                    }
                }
                else if (belka == 6) {
                    if (++filterForm == 11) {
                        filterForm = 1;
                    }
                }
                else if (belka == 7) {
                    filterMinPrice += 100000.0;
                    if (filterMinPrice > clubRef.finances[13]) {
                        filterMinPrice = clubRef.finances[13];
                    }
                }
                else if (belka == 8) {
                    filterMaxPrice += 100000.0;
                    if (filterMaxPrice > clubRef.finances[13]) {
                        filterMaxPrice = clubRef.finances[13];
                    }
                }
                break;
            }
            case _KEY_LEFT: {
                if (belka == 1) {
                    if (--filterPosition == -1) {
                        filterPosition = 4;
                    }
                }
                else if (belka == 2) {
                    if (--filterGoalkeeper == 0) {
                        filterGoalkeeper = 20;
                    }
                }
                else if (belka == 3) {
                    if (--filterDefense == 0) {
                        filterDefense = 20;
                    }
                }
                else if (belka == 4) {
                    if (--filterMidfield == 0) {
                        filterMidfield = 20;
                    }
                }
                else if (belka == 5) {
                    if (--filterAttact == 0) {
                        filterAttact = 20;
                    }
                }
                else if (belka == 6) {
                    if (--filterForm == 0) {
                        filterForm = 10;
                    }
                }
                else if (belka == 7) {
                    filterMinPrice -= 100000.0;
                    if (filterMinPrice < 0) {
                        filterMinPrice = 0;
                    }
                }
                else if (belka == 8) {
                    filterMaxPrice -= 100000.0;
                    if (filterMaxPrice < 0) {
                        filterMaxPrice = 0;
                    }
                }
                break;
            }
            case 'M': {
                pColors->textColor(LIGHTGRAY);
                wcout << endl << pLang->get(L"Enter the minimum player price: ");
                filterMinPrice = pInput->getFloat();
                break;
            }
            case 'X': {
                pColors->textColor(LIGHTGRAY);
                wcout << endl << pLang->get(L"Enter the maximum player price: ");
                filterMaxPrice = pInput->getFloat();
                break;
            }
        }
    }
    while (menuFilters != 'Q');

    if (filterMinPrice > filterMaxPrice) {
        filterMaxPrice = clubRef.finances[13];
    }
}

void Manager::printFinancesValues(int financesIndex, const wstring label)
{
    SClub &clubRef = pClub->get();

    const int maxBuffer = 128;
    wchar_t amountThis[maxBuffer];
    wchar_t bufferThisMonth[maxBuffer];

    wchar_t amountLast[maxBuffer];
    wchar_t bufferLastMonth[maxBuffer];

    swprintf(amountThis, maxBuffer, pLang->get(L"$%.2f").c_str(), clubRef.finances[financesIndex]);
    swprintf(bufferThisMonth, maxBuffer, L"%16ls - %ls", amountThis, pLang->get(label).c_str());

    swprintf(amountLast, maxBuffer, pLang->get(L"$%.2f").c_str(), clubRef.financesLastMonth[financesIndex]);
    swprintf(bufferLastMonth, maxBuffer, L"%13ls - %ls", amountThis, pLang->get(label).c_str());

    wcout << endl;
    wprintf(L"%-42ls %-42ls", bufferThisMonth, bufferLastMonth);
}

void Manager::menuItemFinance()
{
    SClub &clubRef = pClub->get();
    if (clubRef.clubId > 0) {
        info();
        pColors->textColor(YELLOW);
        wcout << endl << pLang->get(L"FINANCES");

        pColors->textColor(LIGHTGREEN);
        wcout << endl;
        wprintf(L"%-42ls %-42ls", pLang->get(L"Income this month:").c_str(), pLang->get(L"Income last month:").c_str());
        printFinancesValues(0, L"Tickets");
        printFinancesValues(1, L"TV income");
        printFinancesValues(2, L"Shops");
        printFinancesValues(3, L"Sale of players");
        printFinancesValues(4, L"Ads");
        printFinancesValues(5, L"TOTAL");

        pColors->textColor(LIGHTRED);
        wcout << endl;
        wprintf(L"%-42ls %-42ls", pLang->get(L"Losses this month:").c_str(), pLang->get(L"Losses last month:").c_str());
        printFinancesValues(6, L"Salaries");
        printFinancesValues(7, L"Bonuses");
        printFinancesValues(8, L"Purchase of players");
        printFinancesValues(9, L"League penalties");
        printFinancesValues(10, L"TOTAL");

        wchar_t profitThisMonth[128];
        wchar_t profitLastMonth[128];
        swprintf(profitThisMonth, 128, pLang->get(L"Profit this month: $%.2f").c_str(), clubRef.finances[11]);
        swprintf(profitLastMonth, 128, pLang->get(L"Profit last month: $%.2f").c_str(), clubRef.financesLastMonth[11]);

        pColors->textColor(clubRef.finances[11] > 0 ? GREEN : RED);
        wcout << endl << endl;
        wprintf(L"%-39ls", profitThisMonth);
        pColors->textColor(LIGHTGRAY);
        wprintf(L"%-39ls", profitLastMonth);

        wcout << endl << endl;
        wprintf(pLang->get(L"Club finances: $%.2f").c_str(), clubRef.finances[12]);
        wcout << endl;
        wprintf(pLang->get(L"Funds for transfers: $%.2f").c_str(), clubRef.finances[13]);

        wcout << endl << endl << pLang->get(L"Back to MENU - press any key...");
        pInput->getKeyboardPressed();
    }
}

void Manager::menuItemManagement()
{
    SClub &clubRef = pClub->get();
    if (clubRef.clubId > 0) {
        wchar_t menu = 0;
        do {
            info();
            wcout << endl << endl << pLang->get(L"CLUB MANAGEMENT");
            pColors->textColor(LIGHTGRAY);

            int tablePos = pTable->getPositionInTable(clubRef.clubId);

            const int MAX_MSG = 2;
            int messages[MAX_MSG] = {0};

            // club management about player
            if (tablePos == 0) {
                messages[0] = 43; // Zarząd: Wiążemy z panem wielkie nadzieje i oczekujemy sukcesów. Oby współpraca między nami była jak najlepsza.
            }
            else if (tablePos < 4) {
                messages[0] = 31; // Zarząd: Jesteśmy bardzo zadowoleni z pańskiej pracy. Mamy nadzieję, iż utrzyma pan dobrą passę.
            }
            else if (tablePos > 3 && tablePos < 10) {
                messages[0] = 32; // Zarząd: Nie jest źle, ale mogłoby być lepiej. Mamy nadzieję, iż tak będzie.
            }
            else if (tablePos > 9 && tablePos < 14) {
                messages[0] = 33; // Zarząd: Jesteśmy zaniepokojeni pańskimi poczynaniami. Mamy nadzieję, iż poprawi pan obecną sytuację.
            }
            else if (tablePos > 13) {
                messages[0] = 34; // Zarząd: Jesteśmy bardzo rozczarowani pańskimi poczynaniami. Jeśli nie poprawi pan sytuacji, będziemy musieli pana zwolnić.
            }

            // finances message
            if (clubRef.finances[12] <= 500000 && clubRef.finances[11] <= 0)        messages[1] = 35; // Zarząd: Finanse klubu są katastrofalne!
            else if (clubRef.finances[12] <= 500000 && clubRef.finances[11] > 0)    messages[1] = 36;
            else if (clubRef.finances[12] <= 1000000 && clubRef.finances[11] <= 0)  messages[1] = 37;
            else if (clubRef.finances[12] <= 1000000 && clubRef.finances[11] > 0)   messages[1] = 38;
            else if (clubRef.finances[12] <= 2000000 && clubRef.finances[11] <= 0)  messages[1] = 39;
            else if (clubRef.finances[12] <= 2000000 && clubRef.finances[11] > 0)   messages[1] = 40;
            else if (clubRef.finances[12] > 2000000 && clubRef.finances[11] <= 0)   messages[1] = 41;
            else if (clubRef.finances[12] > 2000000 && clubRef.finances[11] > 0)    messages[1] = 42; // Zarząd: Finanse klubu są bardzo dobre.

            for (int i = 0; i < MAX_MSG; i++) {
                wcout << endl << endl;
                const SNews& news = pNews->getManagerNewsByNumber(messages[i]);
                if (news.num > 0) {
                    wcout << news.message;
                }
            }

            pColors->textColor(GREEN);
            wcout << endl << endl << L" " <<  pLang->get(L"T Request for additional funds for transfers");

            pColors->textColor(RED);
            wcout << endl << L" " << pLang->get(L"Q Back to MENU") << endl;
            menu = pInput->getKeyboardPressed();
            switch (menu) {
                case 'T': { // Request for additional funds for transfers
                    if (isGrantedFunds(clubRef) && !clubRef.isBlockTransferFunds) {
                        pColors->textColor(GREEN);
                        wcout << endl << endl <<
                            pLang->get(L"Management: We agree at your request. We hope you will use the funds wisely and strengthen our team.");
                        clubRef.finances[13] = clubRef.finances[12];
                        clubRef.isBlockTransferFunds = 1;

                        pClub->save();
                    }
                    else {
                        pColors->textColor(RED);
                        clubRef.isBlockTransferFunds = 1;
                        wcout << endl << endl << pLang->get(L"Management: We reject your request.");

                        pClub->save();
                    }
                    pInput->getKeyboardPressed();
                    break;
                }
            }
        }
        while (menu != 'Q');
    }
}

void Manager::menuItemManagerStats()
{
    const SClub &clubRef = pClub->get();

    info();
    wcout << endl << endl << L"MANAGER STATISTICS";

    pColors->textColor(LIGHTGRAY);
    wcout << endl << endl;
    wprintf(pLang->get(L"Leagues won:            %4d").c_str(), clubRef.managerStats[0]);
    wcout << endl;
    wprintf(pLang->get(L"Prizes awarded:         %4d").c_str(), clubRef.managerStats[2]);
    wcout << endl;
    wprintf(pLang->get(L"Manager points:         %4d").c_str(), clubRef.managerStats[3]);
    wcout << endl;
    wprintf(pLang->get(L"Matches played:         %4d").c_str(), clubRef.managerStats[4]);
    wcout << endl;
    wprintf(pLang->get(L"Matches won:            %4d").c_str(), clubRef.managerStats[5]);
    wcout << endl;
    wprintf(pLang->get(L"Matches draws:          %4d").c_str(), clubRef.managerStats[6]);
    wcout << endl;
    wprintf(pLang->get(L"Matches lost:           %4d").c_str(), clubRef.managerStats[7]);
    wcout << endl;
    wprintf(pLang->get(L"Goals scored:           %4d").c_str(), clubRef.managerStats[8]);
    wcout << endl;
    wprintf(pLang->get(L"Goals lost:             %4d").c_str(), clubRef.managerStats[9]);
    wcout << endl;
    wprintf(pLang->get(L"Bought players:         %4d - total $%.2f").c_str(), clubRef.managerStats[10], clubRef.totalExpensesTransfers);
    wcout << endl;
    wprintf(pLang->get(L"Sold players:           %4d - total $%.2f").c_str(), clubRef.managerStats[11], clubRef.totalRevenuesTransfers);

    wcout << endl << endl << pLang->get(L"Back to MENU - press any key...");

    pInput->getKeyboardPressed();
}

void Manager::menuItemNewsOld()
{
    const int maxCount = 4;
    const int pages = ceil(pNews->getSizeManagerMessages() / (float)maxCount);
    int startFrom = pNews->getSizeManagerMessages() - 1;
    wchar_t ch;

    do {
        pInput->clrscr();
        info();
        pColors->textColor(GREEN);
        wcout << endl << endl << pLang->get(L"OLD NEWS") << endl;

        pColors->textColor(LIGHTGRAY);
        // wczytujemy maxCount wiadomosci
        for (int index = startFrom, counter = maxCount - 1;
            index >= 0 && counter >= 0;
            --index, --counter
        ) {
            wcout << endl << pNews->getManagerMessage(index) << endl;
        }

        wcout << endl;

        pColors->textBackground(BLACK);
        pColors->textColor(GREEN);
        wcout << endl << L"<- " << pLang->get(L"Newer messages")
                << L" (" << (startFrom / maxCount) + 1 << "/" << pages << ") "
                << pLang->get(L"Older messages") << L" ->";

        pColors->textColor(RED);
        wcout << endl << pLang->get(L"Q Quit") << L" ";

        pColors->textColor(GREEN);
        ch = pInput->getKeyboardPressed();
        switch (ch) {
            case _KEY_RIGHT:
            case L'>':
            case L'.':
            case L' ':
            case L'\n': {
                // prev page
                startFrom -= maxCount;
                if (startFrom < 0) {
                    startFrom = pNews->getSizeManagerMessages() - 1;
                }
                break;
            }
            case _KEY_LEFT:
            case L'<':
            case L',': {
                // next page
                startFrom += maxCount;
                if (startFrom > pNews->getSizeManagerMessages()) {
                    startFrom = (pNews->getSizeManagerMessages() - ((pages - 1) * maxCount)) - 1;
                }
                break;
            }
        }
    }
    while (ch != 'Q');
}

void Manager::menuItemNews()
{
    info();
    pColors->textColor(GREEN);
    wcout << endl << endl << pLang->get(L"NEWS");

    pColors->textColor(LIGHTGRAY);
    if (pNews->isMessage()) { //czy w ogóle jest jakaś wiadomość
        pNews->addDisplayManagerMessages(pClub->get(), true);
        pInput->getKeyboardPressed();
    }
}

void Manager::menuItemOptions()
{
    SClub &clubRef = pClub->get();

    bool isOptionCorrect = false;
    int count = 0;
    do {
        pInput->clrscr();
        pColors->textBackground(BLACK);

        if (count > 0) {
            pColors->textColor(RED);
            wcout << pLang->get(L"Error. Invalid number. Enter 1 or 2.");
        }

        pColors->textColor(LIGHTGRAY);
        wcout << endl
            << pLang->get(L"If the display time of messages during the match is not adequate to the Speed settings, change the option below to Manual.") << endl
            << endl
            << pLang->get(L"Displaying subsequent messages during the match is to take place:") << endl
            << pLang->get(L"1. Automatic (recommended)") << endl
            << pLang->get(L"2. Manually (in case of problems)") << endl << endl
            << pLang->get(L"Enter the appropriate number (1 or 2):") << L" ";

        int option = pInput->getNumber();
        isOptionCorrect = option == 1 || option == 2;
        clubRef.isMatchAutoMsg = option == 1 ? 1 : 0;

        count++;
    }
    while (!isOptionCorrect);

    pClub->save();
}

wstring Manager::getSortTitle(int sort)
{
    switch (sort) {
        case  3: return pLang->get(L" - sorted by Goalkeepers skill");
        case  4: return pLang->get(L" - sorder by Defenses skill");
        case  5: return pLang->get(L" - sorted by Midfields skill");
        case  6: return pLang->get(L" - sorted by Attacks skill");
        case  7: return pLang->get(L" - sorted by morale");
        case  9: return pLang->get(L" - sorted by forms");
        case 11: return pLang->get(L" - sorted by conditions");
        case 16: return pLang->get(L" - sorted by goals");
        case 18: return pLang->get(L" - sorted by contract expiry order");
        default: return L"";
    }
}

wstring Manager::getTrainingDayName(int dayNumber)
{
    switch (dayNumber) {
        case 1:  return pLang->get(L"Monday");
        case 2:  return pLang->get(L"Tuesday");
        case 3:  return pLang->get(L"Wednesday");
        case 4:  return pLang->get(L"Thursday");
        case 5:  return pLang->get(L"Friday");
        case 6:  return pLang->get(L"Saturday");
        case 7:  return pLang->get(L"Sunday");
        default: return L"";
    }
}

wstring Manager::getFilterByPosition(int pos)
{
    switch (pos) {
        case 0:  return pLang->get(L"All");
        case 1:  return pLang->get(L"Goalkeepers");
        case 2:  return pLang->get(L"Defenders");
        case 3:  return pLang->get(L"Midfielders");
        case 4:  return pLang->get(L"Strikers");
        default: return pLang->get(L"Error");
    }
}

void Manager::setFilterColors(int bar, int barValue)
{
    if (bar == barValue) {
        pColors->textBackground(BLUE);
        pColors->textColor(LIGHTGRAY);
    }
    else {
        pColors->textBackground(BLACK);
        pColors->textColor(GREEN);
    }
}

void Manager::setAssistantMessageAfterMatch()
{
    if (pClub->get().isAssistantMsg) { // wiadomość asystenta po meczu
        SClub &clubRef = pClub->get();
        clubRef.lastMatchRivalId = clubRef.rivalData[0];

        // odczytanie sil formacji w meczu
        int OnaA = pMatch->getDonA();
        int PnaP = pMatch->getMonM();
        int AnaO = pMatch->getAonD();

        if (!clubRef.isWalkover) {
            if (clubRef.playerGoals <= clubRef.rivalGoals) {
                // przegrana
                if (AnaO < -20 && clubRef.inst[4] == 0) {
                    // 20 = ASYSTENT: Byliśmy osłabieni w ataku, więc moim zdaniem gra z kontry przyniosłaby lepsze rezultaty.
                    pNews->setTmpMsgData(20);
                }
                if (PnaP < -10 && clubRef.inst[0] != 4) {
                    pNews->setTmpMsgData(21);
                }
                if (OnaA < 0 && clubRef.inst[3] == 0) {
                    pNews->setTmpMsgData(22);
                }
                if (OnaA > 20 && clubRef.inst[3] == 1) {
                    pNews->setTmpMsgData(23);
                }
                if (PnaP > 10 && clubRef.inst[0] != 2) {
                    pNews->setTmpMsgData(24);
                }
                if (AnaO > 0 && clubRef.inst[4] == 1) {
                    pNews->setTmpMsgData(25);
                }

                if (clubRef.inst[2] == 0) {
                    pNews->setTmpMsgData(26);
                }
                if (clubRef.inst[2] == 1) {
                    pNews->setTmpMsgData(27);
                }
                if (clubRef.inst[1] != 3) {
                    pNews->setTmpMsgData(28);
                }
            }
            else { // wygrana meczu
                // ASYSTENT: Gratuluję zwycięstwa. Dobrał pan właściwą taktykę.
                // ASYSTENT: Gratuluję wygranej. Nie mam zastrzeżeń co do zastosowanej taktyki.
                pNews->setTmpMsgData(pRand->get(29, 30));
            }
        }
        clubRef.isAssistantMsg = 0;

        pClub->save();
    }
}

void Manager::setRivalForPlayer()
{
    SClub &clubRef = pClub->get();

    if (!clubRef.isRivalSet) {
        clubRef.isRivalSet = 1;

        int clubIndexInRound = pRounds->getClubIndexInRoundByClubId(clubRef.roundNumber, clubRef.clubId);

        if (clubIndexInRound % 2 == 0) { //gram u siebie
            clubRef.rivalData[3] = clubIndexInRound + 1; // więc rywal jest na prawo ode mnie
            clubRef.rivalData[1] = 0;
        }
        else { // gram na wyjezdzie
            clubRef.rivalData[3] = clubIndexInRound - 1; // więc rywal jest na lewo ode mnie
            clubRef.rivalData[1] = 1;
        }

        // wez znajdz i przypisz do clubRef.rivalData[0], numer klubu rywala z kolejki
        clubRef.rivalData[0] = pRounds->getClubIdInRoundByClubIndex(clubRef.roundNumber, clubRef.rivalData[3]);
        clubRef.rivalData[2] = getRivalSetting(clubRef.rivalData[0]);

        clubRef.rivalInst[0] = pRand->get(4); // passing always random
        clubRef.rivalInst[1] = (pRand->get(3) == 1) // rival treatment (never delicate)
            ? 3 // hard 1/3 chance
            : 1; // normal 2/3 chance

        clubRef.rivalInst[2] = (pRand->get(4) == 1) // pressing 3/4 chance for yes
            ? 0 // no
            : 1; // yes

        clubRef.rivalInst[3] = getRivalOffsideTrap(clubRef.rivalData[2]);
        clubRef.rivalInst[4] = getRivalContra(clubRef.rivalData[2]);

        // nastawienie rywala: normalne, obronne, atak
        clubRef.rivalInst[5] = getRivalAttitude(clubRef.rivalData[2]);

        pClub->save();

        for (size_t i = 0; i < pFootballers->getSizeRivals(); i++) {
            SFootballer &footballer = pFootballers->getRival(i);
            footballer.data[7] = pRand->get(0, 3); // morale from middle to super
            footballer.data[11] = 100; // kondycja 100%
        }
        pFootballers->saveRivals();
    }
}

int Manager::getRivalSetting(int clubId)
{
    switch (clubId) {
        case 1:  // Amica Wronki
        case 10: // Polonia Warszawa
        case 12: // Ruch Chorzów
        case 13: // Widzew Łódź
        case 16: // Zagłębie Lubin
            return T4_4_2;

        case 2:  // Garbarnia Szczak.
        case 4:  // Górnik Zabrze
        case 11: // Pogoń Szczecin
            return T5_3_2;

        case 3: // GKS Katowice
        case 9: // Odra Wodzisław
            return T3_5_2;

        case 5: // Groclin Dyskobolia
            return T5_3_2_DEF;

        case 6: // KSZO Ostrowiec
            return T4_4_2_DEF;

        case 7: // Lech Poznań
            return T4_3_3;

        case 8:  // Legia Warszawa
        case 14: // Wisła Kraków
            return T4_4_2_ATT;

        case 15: // Wisła Płock
            return T5_3_2_ATT;

        default:
            return T4_4_2;
    }
}

int Manager::getRivalOffsideTrap(int rivalSetting)
{
    switch (rivalSetting) {
        case T4_4_2:
        case T3_5_2:
        case T4_4_2_ATT:
        case T4_3_3: {
            return 1; // offside traps yes
        }
        case T5_3_2:
        case T5_3_2_ATT: {
            return pRand->get(0, 1); // random yes/no
        }
        default: {
            return 0; // offside traps no
        }
    }
}

int Manager::getRivalContra(int rivalSetting)
{
    switch (rivalSetting) {
        case T4_4_2_DEF:
        case T5_3_2_DEF: {
            return 1; // counter-game yes
        }
        case T4_4_2_ATT:
        case T4_3_3:
        case T5_3_2_ATT: {
            return 0; // counter-game no
        }
        default: {
            return pRand->get(0, 1); // random 50/50
        }
    }
}

int Manager::getRivalAttitude(int rivalSetting)
{
    switch (rivalSetting) {
        case T4_4_2_DEF:
        case T5_3_2_DEF: {
            return pRand->get(3) == 1 ? INSTR_ATTIT_NORMAL : INSTR_ATTIT_DEFENSIVE; // 2/3 chance for defensive, attack never
        }
        case T4_4_2_ATT:
        case T4_3_3:
        case T5_3_2_ATT: {
            return pRand->get(3) == 1 ? INSTR_ATTIT_NORMAL : INSTR_ATTIT_ATTACK; // 2/3 chance for attack, defensive never
        }
        case T3_5_2:
        case T5_3_2:  {
            switch (pRand->get(1, 4)) {
                case 1:     return INSTR_ATTIT_DEFENSIVE; // 1/4 chance for defensive
                case 2:     return INSTR_ATTIT_ATTACK; // 1/4 chance for attack
                default:    return INSTR_ATTIT_NORMAL; // 2/4 chance for normal attitude
            }
        }
        default: {
            return pRand->get(3) == 1 ? INSTR_ATTIT_ATTACK : INSTR_ATTIT_NORMAL; // 2/3 chance for normal, defensive never
        }
    }
}

bool Manager::isGrantedFunds(const SClub &clubRef)
{
    int chance = getChanceForGrantedFunds(clubRef);
    return pRand->get(chance) == 1;
}

int Manager::getChanceForGrantedFunds(const SClub &clubRef)
{
    if (clubRef.finances[12] > 0 && clubRef.finances[12] <= 500000) {
        return 6; // 1/6 chance
    }
    else if (clubRef.finances[12] > 500000 && clubRef.finances[12] <= 1000000) {
        return 5; // 1/5 chance
    }
    else if (clubRef.finances[12] > 1000000 && clubRef.finances[12] <= 2000000) {
        return 4; // 1/4 chance
    }
    else if (clubRef.finances[12] > 2000000) {
        return 3; // 1/3 chance
    }

    return 0; // no chance, if clubRef.finances[12] <= 0
}
