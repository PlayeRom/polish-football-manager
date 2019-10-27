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
    News *pNews
) {
    this->pClub = pClub;
    this->pColors = pColors;
    this->pInput = pInput;
    this->pFootballers = pFootballers;
    this->pTable = pTable;
    this->pRounds = pRounds;
    this->pNews = pNews;

    pTactic = new Tactic(pColors);
    pTeamComposition = new TeamComposition(pColors);
    pTeamInstr = new TeamInstructions(pColors);

    pMatch = new Match(
        pClub,
        pColors,
        pInput,
        pFootballers,
        pTable,
        pRounds,
        pTeamInstr
    );

    pLogger = new Logger();

    // filtry listy transferowej:
    filterPosition = 0; // 0 = wszyscy
    filterGoalkeeper = 1;
    filterDefense = 1;
    filterMiddlefield = 1;
    filterAttact = 1;
    filterForm = 1;
    filterMinPrice = 0;
    filterMaxPrice = 6000000;
}

Manager::~Manager()
{
    delete pTactic;
    delete pTeamComposition;
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
        mainMenu = displayMainMenu(pNews->messages, pTactic->getTeamSettings());
        pColors->textcolor(GREEN);
        switch (mainMenu) {
            case 'C': { // continue / match
                menuItemContinue();
                break;
            }
            case 'A': { // taktyka
                menuItemTactics();
                break;
            }
            case 'S': { // skład
                menuItemTeamComposition();
                break;
            }
            case 'R': { // trening
                menuItemTrenning();
                break;
            }
            case 'O': { // ostatni mecz
                menuItemLastMatch();
                break;
            }
            case 'P': { // najbliższy przeciwnik
                menuItemRival();
                break;
            }
            case 'K': { //kalendarz
                menuItemCalendar();
                break;
            }
            case 'T': { //tabela
                menuItemTable();
                break;
            }
            case 'L': { // lista transferowa
                menuItemTransfersList();
                break;
            }
            case 'F': { //finanse
                menuItemFinance();
                break;
            }
            case 'D': {// zarzad klubu
                menuItemManagement();
                break;
            }
            case 'M': { // manager - statystyki
                menuItemManagerStats();
                break;
            }
            case 'E': { // stare wiadomosci
                menuItemNewsOld();
                break;
            }
            case 'W': { //masz wiadomość
                menuItemNews();
                break;
            }
            case 'H': { // opcja dla meczu
                menuItemOptions();
                break;
            }
            case 'Q': { // wyjscie
                pColors->textcolor(LIGHTRED);
                wcout << endl << L"Czy chcesz wyjść z gry? (T/n): ";

                wchar_t yn = pInput->getch();
                mainMenu = (yn == L't' || yn == L'\n') ? 'Q' : 0;
                break;
            }
        }
    }//dla do mainMenu
    while (mainMenu != 'Q');

    pInput->clrscr();
    wcout << pColors->setColors(1, Colors::RESET);
}

void Manager::info()
{
    wstring dniT[7] = {L"Pn", L"Wt", L"Śr", L"Cz", L"Pt", L"So", L"N"};
    pInput->clrscr();

    pColors->textcolor(WHITE);

    const SClub &clubRef = pClub->get();

    if (clubRef.clubId > 0) {
        wcout << L" " << pClub->getClubName(clubRef.clubId - 1);
    }
    else {
        wcout << L" Bezrobotny";
    }
    pColors->textcolor(GREEN);

    wcout << L" " << clubRef.managerName << L" " << clubRef.managerSurname;
    if (clubRef.isNick) {
        wcout << L" \"" << clubRef.nick << L"\"";
    }
    pColors->textcolor(WHITE);

    wprintf(
        L" Data: %ls %02d.%02d.%d r.",
        dniT[clubRef.weekNumber - 1].c_str(),
        clubRef.day,
        clubRef.month,
        clubRef.year
    );
    /*wcout << L" Data: " << dniT[clubRef.weekNumber - 1] << L" " <<
            std::setfill(L'0') << std::setw(2) << clubRef.day << L"." <<
            std::setfill(L'0') << std::setw(2) << clubRef.month << L"." <<
            clubRef.year << L" r.";*/
}

wchar_t Manager::displayMainMenu(const int message[5], const wstring *pTactics)
{
    const SClub &clubRef = pClub->get();
    int tablePos = pTable->getPositionInTable(clubRef.clubId);
    pColors->textcolor(LIGHTGRAY);

    wcout << endl << endl << L" MENU" << endl;
    pColors->textcolor(LIGHTGREEN);

    if (!clubRef.isMatch) {
        wcout << endl << L" C Kontynuuj";
    }
    else {
        wcout << endl << L" C Mecz";
    }
    pColors->textcolor(GREEN);

    wcout << endl << L" A Taktyka: " << pTactics[clubRef.teamSetting - 1];
    wcout << endl << L" S Skład";
    wcout << endl << L" R Trening";
    pColors->textcolor(LIGHTBLUE);
    wcout << endl << L" O Ostatni mecz - raport";
    wcout << endl << L" P Przeciwnik: " << pClub->getClubName(clubRef.rivalData[0] - 1);
    wcout << endl << L" K Kalendarz";
    wcout << endl << L" T Tabela: " << tablePos << L" miejsce";
    wcout << endl << L" L Lista transferowa";
    pColors->textcolor(YELLOW);
    wcout << endl << L" F Finanse: " << std::fixed << std::setprecision(2) << clubRef.finances[11] << L" zł.";
    wcout << endl << L" D Zarząd klubu";
    wcout << endl << L" M Manager";
    if (message[0] == 1) {
        wcout << L"\033[5m"; // <- miganie, nie dziala na gnome
        pColors->textcolor(LIGHTRED);

        wcout << endl << L" W";
        pColors->textcolor(140);
        wcout << L" Masz wiadomość";
        wcout << L"\033[0m"; // <- przerwij miganie
    }
    else {
        pColors->textcolor(LIGHTGRAY);
        wcout << endl << L" E Stare wiadomości";
    };
    pColors->textcolor(BROWN);

    wcout << endl << endl << L" H Opcje dla meczu";
    pColors->textcolor(RED);

    wcout << endl << L" Q Wyjście" << endl;

    return pInput->getKeyBoardPressed();
}

void Manager::swapFootballers()
{
    int numer1, numer2;

    size_t playersCount = pFootballers->getSizePlayerTeam();

    pColors->textcolor(LIGHTGRAY);
    wcout << endl << L"Podaj numer Lp., spacja, drugi numer i Enter: ";
    pInput->getNumbersExchange(numer1, numer2);

    if (numer1 < 1 || numer1 > playersCount ||
        numer2 < 1 || numer2 > playersCount
    ) {
        numer1 = 1;
        numer2 = 1;
    }

    if (numer1 != numer2) {
        for (size_t i = 0; i < playersCount; i++) {
            SFootballer &footballer = pFootballers->getPlayerTeam(i);
            if (footballer.data[0] == numer1) {
                footballer.data[0] = numer2;
            }
            else if (footballer.data[0] == numer2) {
                footballer.data[0] = numer1;
            }
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
            // nie ma meczu wiec procesuj
            menuItemContinueProcessing(clubRef);
        }
    }
    else { // manager bezrobotny
        menuItemContinueUnemployed(clubRef);
    }
}

/**
 * Processing of passing days
 *
 * @param clubRef
 */
void Manager::menuItemContinueProcessing(SClub &clubRef)
{
    pColors->textcolor(LIGHTGRAY);
    clubRef.isBlockTransferFunds = 0;

    pFootballers->changeTransferList();

    if (pNews->messages[0] == 1) { //czy wogóle jest jakaś wiadomość
        for (int i = 1; i < 10; i++) {
            const SNews& news = pNews->getManagerNewsByNumber(pNews->messages[i]);
            if (news.num > 0) {
                pNews->addManagerMessage(news, clubRef, pNews->stringForMessage[i - 1].c_str());
            }
        }

        pNews->saveManagerMessages();

        memset(pNews->messages, 0, MAX_MESSAGES * sizeof(int));
    }

    bool isProcessing = true;
    while (isProcessing) {
        clubRef.day++;
        clubRef.weekNumber++;
        int holidaysCounter = 0;
        if (clubRef.weekNumber == 8) {
            clubRef.weekNumber = 1;
            for (int i = 0; i < 28; i++) {
                if (clubRef.training[i] == TRENNING_HOLIDAY) {
                    holidaysCounter++; // zlicz ilosc dnu wolnych od treningu
                }
            }

            if (holidaysCounter == 0) {
                // 0 dni wolnych, morale w dol
                for (size_t i = 0; i < pFootballers->getSizePlayerTeam(); i++) {
                    SFootballer &footballer = pFootballers->getPlayerTeam(i);
                    pFootballers->moraleDown(footballer, 3);
                }

                pFootballers->savePlayerTeam();
            }
            else if (holidaysCounter > 1) {
                // sa dni wolne, morale w gore
                for (size_t i = 0; i < pFootballers->getSizePlayerTeam(); i++) {
                    SFootballer &footballer = pFootballers->getPlayerTeam(i);
                    pFootballers->moraleUp(footballer, holidaysCounter - 1);
                }

                pFootballers->savePlayerTeam();
            }
        }
        if ((clubRef.month == 1 ||
            clubRef.month == 3 ||
            clubRef.month == 5 ||
            clubRef.month == 7 ||
            clubRef.month == 8 ||
            clubRef.month == 10) && clubRef.day == 32
        ) {
            clubRef.day = 1;
            clubRef.month++;
        }
        if (clubRef.month == 2 && clubRef.day == 29) {
            clubRef.day = 1;
            clubRef.month++;
        }
        if ((clubRef.month == 4 || clubRef.month == 6 || clubRef.month == 9 || clubRef.month == 11) &&
            clubRef.day == 31
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
            int numer1 = 0;
            pNews->messages[0] = 1;
            int tablePosRandom = (rand() % 2) + 1; //random(2)+1;
            for (int i = 1; i < MAX_MESSAGES; i++) {
                if (pNews->messages[i] == 0 && numer1 == 0) {
                    int tablePos = pTable->getPositionInTable(clubRef.clubId);

                    if (tablePos == 0) {
                        pNews->messages[i] = 43;
                        numer1 = 1;
                    }
                    else if (tablePos < 4) {
                        pNews->messages[i] = 31;
                        numer1 = 1;
                    }
                    else if (tablePos > 3 && tablePos < 10) {
                        pNews->messages[i] = 32;
                        numer1 = 1;
                    }
                    else if (tablePos > 9 && tablePos < 14) {
                        pNews->messages[i] = 33;
                        numer1 = 1;
                    }
                    else if (tablePos > 13) {
                        pNews->messages[i] = 34;
                        numer1 = 1;
                    }

                    if (clubRef.finances[12] <= 500000 && clubRef.finances[11] <= 0) {
                        pNews->messages[i + 1] = 35;
                    }
                    else if (clubRef.finances[12] <= 500000 && clubRef.finances[11] > 0) {
                        pNews->messages[i + 1] = 36;
                    }
                    else if (clubRef.finances[12] <= 1000000 && clubRef.finances[11] <= 0) {
                        pNews->messages[i + 1] = 37;
                    }
                    else if (clubRef.finances[12] <= 1000000 && clubRef.finances[11] > 0) {
                        pNews->messages[i + 1] = 38;
                    }
                    else if (clubRef.finances[12] <= 2000000 && clubRef.finances[11] <= 0) {
                        pNews->messages[i + 1] = 39;
                    }
                    else if (clubRef.finances[12] <= 2000000 && clubRef.finances[11] > 0) {
                        pNews->messages[i + 1] = 40;
                    }
                    else if (clubRef.finances[12] > 2000000 && clubRef.finances[11] <= 0) {
                        pNews->messages[i + 1] = 41;
                    }
                    else if (clubRef.finances[12] > 2000000 && clubRef.finances[11] > 0) {
                        pNews->messages[i + 1] = 42;
                    }
                    //***** manager miesiąca *****
                    if (clubRef.month == 4 ||
                        clubRef.month == 5 ||
                        clubRef.month == 6 ||
                        clubRef.month == 7 ||
                        clubRef.month == 9 ||
                        clubRef.month == 10 ||
                        clubRef.month == 11 ||
                        clubRef.month == 12
                    ) {
                        int clubId = pTable->getClubNumberInPosition(tablePosRandom);
                        if (clubId == clubRef.clubId) {
                            pNews->messages[i + 2] = clubId + 60;
                            pNews->stringForMessage[i + 1] = clubRef.managerSurname;
                            clubRef.managerStats[2]++;
                            clubRef.managerStats[3] += (clubRef.managerStats[2] * 10);
                        }
                        else {
                            pNews->messages[i + 2] = clubId + 44;
                        }
                    }
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
                clubRef.isMatch = 1; // nastał dzien rozergrania meczu
                isProcessing = false;
            }
        }

        //ustalam kolejkę end
        int lata = 0;
        float transfer = 0; // <- nazwy sa z czapy
        float cena = 0;
        float placa = 0;
        float premia = 0;
        for (int i = clubRef.weekNumber - 1; i < 28; i += 7) { //sprawdzamy dany dzien
            if (clubRef.training[i] == 1) transfer++; //kond
            if (clubRef.training[i] == 2) cena++; //podania
            if (clubRef.training[i] == 3) placa++; //stałe
            if (clubRef.training[i] == 4) premia++; //taktyka
            if (clubRef.training[i] == 5) lata++;
        }
        clubRef.trained[0] += transfer * 0.2;
        if (transfer == 0) {
            clubRef.trained[0] -= 0.1;
        }
        clubRef.trained[1] += cena * 0.2;
        if (cena == 0) {
            clubRef.trained[1] -= 0.1;
        }
        clubRef.trained[2] += placa * 0.2;
        if (placa == 0) {
            clubRef.trained[2] -= 0.1;
        }
        clubRef.trained[3] += premia * 0.2;
        if (premia == 0) {
            clubRef.trained[3] -= 0.1;
        }

        int playerTeamSize = pFootballers->getSizePlayerTeam();

        int randMinus = rand() % 12; // losuje czy odjąć 1/12
        if (lata == 0 && randMinus == 0) {
            clubRef.treBOPN--;
            int footballerId = (rand() % playerTeamSize) + 1; // losuj ktory pilkarz

            for (size_t index = 0; index < playerTeamSize; index++) {
                SFootballer &footballer = pFootballers->getPlayerTeam(index);
                if (footballer.data[0] == footballerId) { //--
                    randMinus = (rand() % 4) + 3; // losujemy ktora umiejetnosc spada
                    footballer.data[randMinus]--;
                    if (footballer.data[randMinus] < 1) {
                        footballer.data[randMinus] = 1;
                        clubRef.treBOPN++;
                    }

                    break;
                }
            }

            pFootballers->savePlayerTeam();
        }//dla lata==0

        int randPlus = rand() % 6; //losuje czy dodać trening BOPN (1/6 szans)
        if (lata > 0 && randPlus == 0) {
            int footballerId = (rand() % playerTeamSize) + 1; // losuj ktory pilkarz

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
        } // dla if (lata>0)

        //dni kontraktów

        int newFootballerId = 0;
        int lostContractCounter = 0;
        clubRef.finances[6] = 0;
        for (size_t index = 0; index < pFootballers->getSizePlayerTeam(); index++) {
            SFootballer &footballer = pFootballers->getPlayerTeam(index);
            //************ sprzedaż zawodnika *****************
            //lista transferowa
            int kupno = (rand() % 20);
            if (footballer.data[17] == 1 && kupno == 0) {
                pInput->clrscr();
                int mode = 0;
                while (!mode) {
                    lata = (rand() % 16) + 1;
                    if (lata != clubRef.clubId) {
                        mode = 1;
                    }
                }
                pColors->textcolor(LIGHTGRAY);
                int random = (rand() % 10) - 5;
                transfer = random * 10000.0;
                cena = transfer + footballer.finances[0];
                if (cena > footballer.finances[3]) {
                    cena = footballer.finances[3];
                }
                wprintf(
                    L"\n\r%ls jest zainteresowana kupnem %ls%ls" \
                    L"\n\rWartość %ls wynosi %.2f zł." \
                    L"\n\r%ls żąda %.2f zł." \
                    L"\n\r%ls jest gotowa zapłacić %.2f zł.",
                    pClub->getClubName(lata).c_str(),
                    footballer.name,
                    footballer.surname,
                    footballer.surname,
                    footballer.finances[0], // wartosc $ zawodnika
                    pClub->getClubName(clubRef.clubId - 1).c_str(),
                    footballer.finances[3], // ile jakiś klub $ żąda za kupno zawodnka
                    pClub->getClubName(lata).c_str(),
                    cena
                );
                wcout << endl << endl << L"Czy akceptujesz propozycję? (T/n): ";
                wchar_t yn = pInput->getch();
                if (yn == L't' || yn == L'\n') {
                    pColors->textcolor(GREEN);
                    wprintf(
                        L"\n\r\n\rKontrakt %ls wygasa. %ls zostaje sprzedany do %ls.",
                        footballer.surname,
                        footballer.surname,
                        pClub->getClubName(lata).c_str()
                    );
                    footballer.data[18] = 1;
                    footballer.data[17] = 0;
                    footballer.data[22] = lata + 1;
                    clubRef.finances[3] += cena;
                    clubRef.finances[5] = clubRef.finances[0] +
                            clubRef.finances[1] +
                            clubRef.finances[2] +
                            clubRef.finances[3] +
                            clubRef.finances[4];
                    clubRef.finances[11] = clubRef.finances[5] - clubRef.finances[10];
                    clubRef.managerStats[11]++;
                    clubRef.totalRevenuesTransfers += cena;

                    pClub->save();

                    pInput->getch();
                }
                else {
                    pColors->textcolor(RED);
                    wcout << endl << endl << pClub->getClubName(clubRef.clubId - 1) << L" odrzuca propozycję.";
                    pInput->getch();
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

            cena = footballer.data[footballer.data[2] + 2];

            if (cena == 20) placa = 36000.0;
            else if (cena >= 17) placa = 27000.0;
            else if (cena >= 15) placa = 18000.0;
            else if (cena >= 12) placa = 14000.0;
            else if (cena >= 10) placa = 9000.0;
            else if (cena >= 5) placa = 7000.0;
            else placa = 6000.0;

            footballer.finances[0] += footballer.data[3] + footballer.data[4] + footballer.data[5] + footballer.data[6];
            footballer.finances[0] *= placa;
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
                pNews->messages[0] = 1;
                for (int i = 1; i < MAX_MESSAGES; i++) {
                    if (pNews->messages[i] == 0) {
                        pNews->messages[i] = 6; // LEKARZ: %ls doznał kontuzji. Za około %d dni będzie do dyspozycji.
                        pNews->stringForMessage[i - 1] = footballer.surname;
                        pNews->numbersForMessage[i - 1] = (rand() % 83) + 8; // za ile dni wyleczy kontuzje
                        footballer.data[15] = pNews->numbersForMessage[i - 1];
                        footballer.data[9]--; //spadek formy
                        if (footballer.data[9] < 1) {
                            footballer.data[9] = 1;
                        }
                        break;
                    }
                }
            }

            if (footballer.data[15] > 0) {
                // zmniejszam licznik ilosci dni do wyleczenia kontuzji
                footballer.data[15]--;
                footballer.data[1] = PLAYERS_TRENNING_NONE; //nie trenuje

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

                    pNews->messages[0] = 1;
                    for (int i = 1; i < MAX_MESSAGES; i++) {
                        if (pNews->messages[i] == 0) {
                            if (footballer.data[15] == 0) {
                                pNews->messages[i] = 4; // LEKARZ: %ls całkowicie wyleczył kontuzję, zaczął trenować i może grać.
                            }
                            else if (footballer.data[15] == 7) {
                                pNews->messages[i] = 5; // LEKARZ: %ls częściowo wyleczył kontuzję i może grać, ale jeszcze nie trenuje.
                            }

                            pNews->stringForMessage[i - 1] = footballer.surname;
                            break;
                        }
                    }
                }
            }
            //kontuzja end

            // gdy zadonik stracil kontrak, przpisz numery porzadkowe zawodnikow
            if (footballer.data[18] == 0) { // kontrakt wygasl
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
                pNews->messages[0] = 1;
                for (int i = 1; i < MAX_MESSAGES; i++) {
                    if (pNews->messages[i] == 0) {
                        pNews->messages[i] = 3;
                        pNews->stringForMessage[i - 1] = footballer.surname;
                        break;
                    }
                }
                footballer.data[0] = pFootballers->getSizeTransfers() + 1;
                if (footballer.data[22] == clubRef.clubId) {
                    footballer.data[22] = 0;
                }

                //transferPlayers.push_back(footballer);
                pFootballers->pushBackTransfersTeam(footballer);
            }
            else if (footballer.data[18] == 7) { // przypomnienie ze za 7 dni konczy sie kontrakt
                isProcessing = false;
                pNews->messages[0] = 1;
                for (int i = 1; i < MAX_MESSAGES; i++) {
                    if (pNews->messages[i] == 0) {
                        pNews->messages[i] = 2;
                        pNews->stringForMessage[i - 1] = footballer.surname;
                        break;
                    }
                }
            }
        }
        pFootballers->savePlayerTeam();
        pFootballers->saveTransfers();

        //******************** komunikaty kibiców ******************
        int random = (rand() % 6);
        if (random == 0 && clubRef.playerGoals != clubRef.rivalGoals) {
            if (clubRef.playerGoals > clubRef.rivalGoals) {
                random = (rand() % 6) + 8;
            }
            else if (clubRef.playerGoals < clubRef.rivalGoals) {
                random = (rand() % 6) + 14;
            }

            isProcessing = false;
            pNews->messages[0] = 1;
            for (int i = 1; i < MAX_MESSAGES; i++) {
                if (pNews->messages[i] == 0) {
                    pNews->messages[i] = random;
                    pNews->stringForMessage[i - 1] = clubRef.isNick ? clubRef.nick : clubRef.managerSurname;

                    if (random == 9) {
                        pNews->stringForMessage[i - 1] = clubRef.isNick ? clubRef.nick : clubRef.managerSurname;
                    }

                    break;
                }
            }
        }

        //****************** zadyma **************
        if (clubRef.isRiot) {
            isProcessing = false;
            pNews->messages[0] = 1;
            clubRef.isRiot = 0;
            for (int i = 1; i < MAX_MESSAGES; i++) {
                if (pNews->messages[i] == 0) {
                    pNews->messages[i] = 44;
                    pNews->stringForMessage[i - 1] = pClub->getClubName(clubRef.clubId - 1);
                    clubRef.finances[9] += 300000.0;
                    break;
                }
            }
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
            pNews->messages[0] = 1;
            for (int i = 1; i < MAX_MESSAGES; i++) {
                if (pNews->messages[i] == 0) {
                    pNews->messages[i] = 78; // %ls MISTRZEM POLSKI!!
                    pNews->stringForMessage[i - 1] = pClub->getClubName(clubId - 1);
                    break;
                }
            }
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
            pNews->messages[0] = 1;
            for (int i = 1; i < MAX_MESSAGES; i++) {
                if (pNews->messages[i] == 0) {
                    pNews->messages[i] = 79; // PREZES: Wraz z zarządem składamy panu gratulacje i podziękowanie za wielkie osiągnięcia jakie pan dokonał.
                    break;
                }
            }
        }
        else if (((clubRef.month == 12 && clubRef.day == 1) || (clubRef.month == 6 && clubRef.day == 25)) && clubPos > 13) { // spadek do 2 ligi
            isProcessing = false;
            pNews->messages[0] = 1;
            for (int i = 1; i < MAX_MESSAGES; i++) {
                if (pNews->messages[i] == 0) {
                    pNews->messages[i] = 77; // PREZES: Wraz z zarządem oświadczamy, iż został pan zwolniony ze stanowiska managera klubu %ls.
                    pNews->stringForMessage[i - 1] = pClub->getClubName(clubRef.clubId - 1);
                    clubRef.clubId = 0;
                    break;
                }
            }
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
        pColors->textcolor(GREEN);
        wcout << endl << endl << L"Czy chcesz przejść do meczu? (T/n): ";
        wchar_t matchYN = pInput->getch();
        if (matchYN == L't' || matchYN == L'\n') {
            pMatch->runMecz();
        }
    }
    else {
        pColors->textcolor(LIGHTRED);
        wcout << endl << endl << L"Nie można rozegrać meczu! W składzie występują niedozwoleni zawodnicy!";
        pInput->getch();
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

    pColors->textcolor(LIGHTGRAY);
    wcout << endl << endl <<
        L"Otrzymujesz dwie propozycje pracy na stanowisku managera od klubów:" << endl <<
        L"1. " << pClub->getClubName(club1 - 1) << endl <<
        L"2. " << pClub->getClubName(club2 - 1);
    wcout << endl << endl << L"W którym klubie chcesz pracować (wpisz odpowiedni numer): ";
    int select = pInput->getNumber();
    if (select == 1 || select == 2) {
        int clubId = (select == 1) ? club1 : club2;

        pFootballers->initialPlayerTeam(clubId);
        pFootballers->savePlayerTeam();

        pNews->messages[0] = 1;
        pNews->messages[1] = 1;
        pNews->stringForMessage[0] = clubRef.managerSurname;

        pClub->initNewClub(clubId);

        pRounds->initial();
        pRounds->save();

        pClub->save();

        pColors->textcolor(GREEN);
        wcout << endl << L"Podjęto pracę w klubie " << pClub->getClubName(clubRef.clubId - 1);
    }
    else {
        pColors->textcolor(RED);
        wcout << endl << L"Wpisano zły numer!";
    }

    pInput->getKeyBoardPressed();
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
            pTeamComposition->draw(pFootballers->getPlayersTeam(), clubRef.teamSetting, 11, clubRef.clubId);

            menu = pInput->getKeyBoardPressed();
            switch (menu) {
                case 'U': {
                    bool loop = false;
                    while (!loop) {
                        pInput->clrscr();
                        pColors->textbackground(BLACK);
                        pColors->textcolor(LIGHTRED);

                        wcout << L"Obecne ustawienie: " << pTactic->getTeamSetting(clubRef.teamSetting - 1);
                        pColors->textcolor(LIGHTGRAY);

                        wcout << endl << L"Wybierz ustawienie zespołu:";
                        for (int i = 0; i < 14; i++) {
                            wcout << endl;
                            if (i < 9) {
                                wcout << L" ";
                            }
                            wcout << i + 1 << L". " << pTactic->getTeamSetting(i);
                        }
                        wcout << endl << endl << L"Wpisz odpowiednią cyfrę: ";
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
                    pColors->textcolor(WHITE);
                    wcout << endl << L"REZERWOWI";
                    pTeamComposition->draw(pFootballers->getPlayersTeam(), clubRef.teamSetting, 16, clubRef.clubId);
                    pColors->textcolor(LIGHTGRAY);
                    wcout << endl << endl << L"Naciśnij dowolny klawisz...";
                    pInput->getKeyBoardPressed();
                    break;
                }
                case 'I': {
                    int belka = 1;
                    wchar_t menuInstr = 0;
                    do {
                        info();
                        pColors->textcolor(WHITE);
                        wcout << endl << endl << L"INSTRUKCJE DLA DRUŻYNY" << endl;

                        pTeamInstr->draw(
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
                        pColors->textbackground(BLACK);

                        menuInstr = pInput->getKeyBoardPressed();
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

void Manager::menuItemTeamComposition()
{
    SClub &clubRef = pClub->get();

    if (clubRef.clubId > 0) {
        bool isNext = false;
        int mode = 0;
        int sort = 0;
        wchar_t menu = 0;
        do {
            info();
            pColors->textcolor(WHITE);
            wcout << endl << L"SKŁAD" << getSortTitle(sort);

            pColors->textcolor(LIGHTGRAY);
            if (sort == 0) {
                mode = isNext ? 40 : 20;
            }

            pTeamComposition->draw(pFootballers->getPlayersTeam(), clubRef.teamSetting, mode, clubRef.clubId, false, sort);

            size_t playersCount = pFootballers->getSizePlayerTeam();

            pColors->textcolor(GREEN);
            wcout << endl;
            if (sort == 0) {
                if (playersCount > 20) {
                    wcout << L"D Dalej   ";
                }
                wcout << L"P Zamiana zawodników   G Szczegóły   S Sortuj";
            }
            else {
                wcout << L"T Tradycyjny widok   G Szszegóły   S Sortuj";
            }

            pColors->textcolor(RED);
            wcout << L"   Q Powrót do MENU";

            pColors->textcolor(LIGHTGRAY);
            menu = pInput->getKeyBoardPressed();
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

                    pColors->textcolor(GREEN);
                    wprintf(
                        L"\n\rSortuj według:" \
                        L"\n\r B Umiejętności B" \
                        L"\n\r O Umiejętności O" \
                        L"\n\r P Umiejętności P" \
                        L"\n\r N Umiejętności N" \
                        L"\n\r M Morale" \
                        L"\n\r F Formy" \
                        L"\n\r K Kondycji" \
                        L"\n\r G Goli" \
                        L"\n\r W Kolejności wygaśnięcia kontraktu"
                    ); //\n\r C Ceny\n\r E Pensji");

                    pColors->textcolor(RED);
                    wcout << endl << L" Q Powrót" << endl;

                    pColors->textcolor(GREEN);
                    wchar_t menuSort = pInput->getKeyBoardPressed();
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
                case 'G': { // szczególy zawodnika
                    menuItemTeamCompositionFootballerDetails();
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

void Manager::menuItemTeamCompositionFootballerDetails()
{
    pColors->textcolor(LIGHTGRAY);
    wcout << endl << L"Podaj numer Lp.: ";
    int footballerId = pInput->getNumber();

    for (size_t index = 0; index < pFootballers->getSizePlayerTeam(); index++) {
        SFootballer &footballer = pFootballers->getPlayerTeam(index);
        if (footballer.data[0] == footballerId) {
            wchar_t menuDetails = 0;
            do {
                info();

                pColors->textcolor(LIGHTGRAY);
                wcout << endl << endl << L"Nazwisko i imię: " << footballer.surname << L" " << footballer.name;
                wstring positionPlayer;
                switch (footballer.data[2]) {
                    case PLAYERS_POS_B: positionPlayer = L"Bramkarz"; break;
                    case PLAYERS_POS_O: positionPlayer = L"Obrońca"; break;
                    case PLAYERS_POS_P: positionPlayer = L"Pomocnik"; break;
                    case PLAYERS_POS_N: positionPlayer = L"Napastnik"; break;
                }
                wprintf(
                    L"\n\r\n\rPozycja: %ls" \
                    L"\n\rUmiejętności: B-%d, O-%d, P-%d, N-%d     Gole: %d",
                    positionPlayer.c_str(),
                    footballer.data[3],
                    footballer.data[4],
                    footballer.data[5],
                    footballer.data[6],
                    footballer.data[16]
                );

                wstring morale = pTeamComposition->getMorale(footballer.data[7]);
                wprintf(
                    L"\n\r\n\rMorale: %ls    Forma: %d    Kondycja: %d%%",
                    morale.c_str(),
                    footballer.data[9],
                    footballer.data[11]
                );
                wcout << endl << endl;
                wcout << L"Żółte kartki: ";
                pColors->textcolor(YELLOW);
                wcout << footballer.data[13];
                pColors->textcolor(LIGHTGRAY);
                wcout << L"    Czerwone kartki: ";
                pColors->textcolor(RED);
                wcout << footballer.data[14];

                if (footballer.data[15] > 0) {
                    pColors->textcolor(LIGHTBLUE);
                    wcout << endl << endl << L"Ilość dni do wyleczenia kontuzji: " << footballer.data[15];
                }
                pColors->textcolor(LIGHTGRAY);
                wprintf(
                    L"\n\r\n\rKontrakt wygaśnie za: %d dni(dzień)" \
                    L"\n\rCena zawodnika: %.2f zł." \
                    L"\n\rPłaca miesięczna: %.2f zł.",
                    footballer.data[18],
                    footballer.finances[0],
                    footballer.finances[1]
                );
                if (footballer.data[2] == PLAYERS_POS_P ||
                    footballer.data[2] == PLAYERS_POS_N
                ) {
                    wprintf(L"\n\rPremia za gola: %.2f zł.", footballer.finances[2]);
                }
                if (footballer.data[17] == 1) {
                    pColors->textcolor(LIGHTBLUE);
                    wprintf(L"\n\r\n\rZawodnik na sprzedaż za %.2f zł.", footballer.finances[3]);
                    pColors->textcolor(LIGHTGRAY);
                }
                pColors->textcolor(GREEN);
                wcout << endl << endl <<
                    L"K Nowy kontrakt" << endl <<
                    L"T Na sprzedaż/Anuluj" << endl <<
                    L"W Wydalenie z klubu";
                pColors->textcolor(RED);
                wcout << endl << L"Q Powrót" << endl;
                pColors->textcolor(GREEN);
                menuDetails = pInput->getKeyBoardPressed();
                switch (menuDetails) {
                    case 'W': { // remove footballer from the club
                        info();
                        pColors->textcolor(LIGHTGRAY);
                        wprintf(
                            L"\n\r\n\rCzy na pewno chcesz wyrzucić %ls%ls z klubu? (T/n): ",
                            footballer.name,
                            footballer.surname
                        );
                        wchar_t yn = pInput->getch();
                        if (yn == L't' || yn == L'\n') {
                            wprintf(L"\n\rJutro wygaśnie kontrakt %ls%ls.", footballer.name, footballer.surname);
                            footballer.data[18] = 1;
                            pInput->getch();
                        }
                        break;
                    }
                    case 'T': { // footballer on sale or cancel it
                        if (footballer.data[17] == 0) {
                            info();
                            pColors->textcolor(WHITE);
                            wprintf(L"\n\r\n\rSprzedaż: %ls %ls", footballer.name, footballer.surname);
                            pColors->textcolor(LIGHTGRAY);
                            wprintf(L"\n\r\n\rPodaj cenę (%.2f): ", footballer.finances[0]);
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
                        pColors->textcolor(WHITE);
                        wprintf(L"\n\r\n\rKontrakt dla: %ls %ls", footballer.name, footballer.surname);
                        pColors->textcolor(LIGHTGRAY);
                        wprintf(L"\n\r\n\rPodaj płacę miesięczną (%.2f): ", footballer.finances[1]);
                        float placa = pInput->getFloat();
                        float premia = 0;
                        if (footballer.data[2] == PLAYERS_POS_P ||
                            footballer.data[2] == PLAYERS_POS_N
                        ) {
                            wprintf(L"\n\rPodaj premię za gola (%.2f): ", footballer.finances[2]);
                            premia = pInput->getFloat();
                        }

                        wprintf(L"\n\rPodaj przez ile lat ma obowiązywać kontrakt (1, 2 lub 3): ");
                        int lata = pInput->getNumber();

                        if (footballer.data[2] == PLAYERS_POS_P ||
                            footballer.data[2] == PLAYERS_POS_N
                        ) {
                            float transfer = premia - footballer.finances[2];

                            if (transfer == 0) weight++;
                            if (transfer >= 50 && transfer < 100) weight += 2;
                            if (transfer >= 100) weight += 3;
                            if (transfer < 0) weight--;
                        }
                        else  {
                            weight++;
                        }
                        float transfer = placa - footballer.finances[1];
                        if (transfer < 0) weight--;
                        if (transfer < -1000) weight -= 3;
                        if (transfer == 0) weight++;
                        if (transfer >= 100 && transfer < 500) weight += 2;
                        if (transfer >= 500 && transfer < 1000) weight += 3;
                        if (transfer >= 1000) weight += 4;

                        if (lata == 1) weight += 2;
                        if (lata == 2) weight++;
                        if (lata < 1 || lata > 3) weight = -3;

                        int sumSkills = footballer.data[3] + footballer.data[4] + footballer.data[5] + footballer.data[6];
                        if (footballer.data[2] == PLAYERS_POS_B && sumSkills > 30) weight -= 2;
                        if (sumSkills >= 30) weight--;
                        if (sumSkills >= 50) weight -= 2;
                        if (sumSkills < 20) weight++;
                        if (sumSkills < 10) weight += 2;
                        if (footballer.data[17] == 1) weight++;

                        if (weight >= 4) {
                            pColors->textcolor(GREEN);
                            wprintf(L"\n\r%ls: Zgadzam się na proponowane warunki kontraktu.", footballer.surname);
                            pColors->textcolor(LIGHTGRAY);
                            wcout << endl << endl << L"Akceptujesz? (T/n): ";
                            wchar_t yn = pInput->getche();
                            if (yn == L't' || yn == L'\n') {
                                footballer.finances[1] = placa;
                                if (footballer.data[2] == PLAYERS_POS_P ||
                                    footballer.data[2] == PLAYERS_POS_N
                                ) {
                                    footballer.finances[2] = premia;
                                }
                                footballer.data[18] = lata * 365;
                                if (footballer.data[17] == 1) {
                                    footballer.data[17] = 0;
                                }
                                if (mode >= 500 && mode < 1000) { // TODO: tutaj jest jakis blad, tak wysoki mode nigdzie nie jest ustawiany?
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
                            pColors->textcolor(RED);
                            wcout << endl << footballer.surname << L": Odrzucam proponowane warunki kontraktu.";
                            pInput->getch();
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

void Manager::menuItemTrenning()
{
    SClub &clubRef = pClub->get();

    if (clubRef.clubId > 0) {
        wchar_t trenningMenu = 0;
        wstring weekDays[7] = {L"Pn", L"Wt", L"Śr", L"Cz", L"Pt", L"So", L"N "};
        wstring whatTrain[10] = {
            L"Kondycja",
            L"Podania",
            L"Stałe fra.",
            L"Taktyka",
            L"Bramkarze",
            L"Obrona",
            L"Pomoc",
            L"Atak",
            L"Wolne",
            L" "
        };

        do {
            info();
            pColors->textcolor(LIGHTGRAY);
            wcout << endl;
            wcout << BOX_LIGHT_DOWN_RIGHT;
            for (int j = 0; j < 3; j++) {
                for (int i = 0; i < 10; i++) {
                    wcout << BOX_LIGHT_HORIZONTAL;
                }
                wcout << BOX_LIGHT_DOWN_HORIZONTAL;
            }
            wcout << BOX_LIGHT_HORIZONTAL;
            pColors->textcolor(WHITE);
            wcout << L"TRENING";
            pColors->textcolor(LIGHTGRAY);
            wcout << BOX_LIGHT_HORIZONTAL << BOX_LIGHT_HORIZONTAL;
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

            //pColors->textcolor(GREEN);
            wcout << BOX_LIGHT_VERTICAL;
            for (int i = 0; i < 7; i++) {
                pColors->textcolor(GREEN);
                wcout << L"  " << i + 1 << L" - " << weekDays[i] << L"  ";
                pColors->textcolor(LIGHTGRAY);
                wcout << BOX_LIGHT_VERTICAL;
            }
            wcout << endl;

            pColors->textcolor(LIGHTGRAY);
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

            for (int i = 0; i < 28; i++) {
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

            wprintf(
                L"\n\rIntensywność:               Pkt:" \
                L"\n\r B, O, P, N: .........%3d%% %5d" \
                L"\n\r Kondycja: ...........%3d%% %5.1f" \
                L"\n\r Podania: ............%3d%% %5.1f" \
                L"\n\r Stałe fragmenty gry: %3d%% %5.1f" \
                L"\n\r Taktyka: ............%3d%% %5.1f" \
                L"\n\r Wolne: ..............%3d%%",
                ilex[4] * 3 * 100 / 21, clubRef.treBOPN,
                ilex[0] * 100 / 21, clubRef.trained[0],
                ilex[1] * 100 / 21, clubRef.trained[1],
                ilex[2] * 100 / 21, clubRef.trained[2],
                ilex[3] * 100 / 21, clubRef.trained[3],
                ilex[8] * 3 * 100 / 21
            );
            pColors->textcolor(GREEN);
            wcout << endl <<
                L" 1-7 Zmiana planu dnia tygodnia" << endl <<
                L" D   Dobór zawodników do treningu indywidualnego";

            pColors->textcolor(RED);
            wcout << endl << L" Q Powrót do MENU" << endl;
            trenningMenu = pInput->getKeyBoardPressed();
            switch (trenningMenu) {
                case L'1':
                case L'2':
                case L'3':
                case L'4':
                case L'5':
                case L'6':
                case L'7': {
                    menuItemTrenningWeekDay(trenningMenu, ilex);
                    break;
                }
                case 'D': { // kto ma trenować B,O,P,N
                    menuItemTrenningIndividual();
                    break;
                }
            }
        }
        while (trenningMenu != 'Q');
    }
}

void Manager::menuItemTrenningWeekDay(wchar_t trenningMenu, int *ilex)
{
    SClub &clubRef = pClub->get();
    int day = wcstol(&trenningMenu, NULL, 10);

    pColors->textcolor(LIGHTGRAY);
    wprintf(
        L"Zmiana planu treningu dla %ls\n\r"\
        L"1. Trening indywidualny (B, O, P, N)\n\r"\
        L"2. Trening drużynowy\n\r"\
        L"3. Dzień wolny\n\r",
        getTrenningDayName(day).c_str()
    );
    wchar_t tranningPlanMenu = pInput->getKeyBoardPressed();
    switch (tranningPlanMenu) {
        case '1': {//B,O,P,N
            for (int i = day - 1, j = 1; i < 28; i += 7, ++j) {
                if (j == 1) clubRef.training[i] = TRENNING_B;
                if (j == 2) clubRef.training[i] = TRENNING_O;
                if (j == 3) clubRef.training[i] = TRENNING_P;
                if (j == 4) clubRef.training[i] = TRENNING_N;
            }
            pClub->save();
            break;
        }
        case  '3': { //wolne
            for (int i = day - 1, j = 1; i < 28; i += 7, ++j) {
                if (j == 1) clubRef.training[i] = TRENNING_HOLIDAY;
                if (j == 2) clubRef.training[i] = TRENNING_EMPTY;
                if (j == 3) clubRef.training[i] = TRENNING_EMPTY;
                if (j == 4) clubRef.training[i] = TRENNING_EMPTY;
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

            int slotNumber = 0; // <- ile obasdzono rodzi treningu, max 3 z 4 dostepnych
            while (slotNumber != 4) {
                slotNumber++;
                if (slotNumber < 4) {
                    pInput->clrscr();
                    pColors->textcolor(WHITE);
                    wcout << endl << L"TRENING DRUŻYNOWY";

                    pColors->textcolor(LIGHTGRAY);
                    wcout << endl << L"Musisz obsadzić 3 rodzaje treningu.";

                    pColors->textcolor(GREEN);
                    for (int i = 0; i < 4; i++) {
                        if (ilex[i] == 1) wcout << endl << L"Wybrano: 1. Kondycja";
                        if (ilex[i] == 2) wcout << endl << L"Wybrano: 2. Podania";
                        if (ilex[i] == 3) wcout << endl << L"Wybrano: 3. Stałe fragmenty gry";
                        if (ilex[i] == 4) wcout << endl << L"Wybrano: 4. Taktyka";
                    }
                    pColors->textcolor(LIGHTGRAY);
                    wprintf(
                        L"\n\rCo chcesz trenować (ilość pozostałych: %d):" \
                        L"\n\r1. Kondycja" \
                        L"\n\r2. Podania" \
                        L"\n\r3. Stałe fragmenty gry" \
                        L"\n\r4. Taktyka\n\r",
                        4 - slotNumber
                    );
                    pColors->textcolor(GREEN);
                    ilex[slotNumber - 1] = pInput->getNumber();
                    if (ilex[slotNumber - 1] < TRENNING_CONDITIONS ||
                        ilex[slotNumber - 1] > TRENNING_TACTICKS
                    ) {
                        slotNumber--; // bledny wybor
                    }
                }

                int index = day - 1 + (7 * (slotNumber - 1));
                switch (ilex[slotNumber - 1]) {
                    case TRENNING_CONDITIONS: {
                        clubRef.training[index] = TRENNING_CONDITIONS;
                        break;
                    }
                    case TRENNING_PASSES: {
                        clubRef.training[index] = TRENNING_PASSES;
                        break;
                    }
                    case TRENNING_FREE_KICKS: {
                        clubRef.training[index] = TRENNING_FREE_KICKS;
                        break;
                    }
                    case TRENNING_TACTICKS: {
                        clubRef.training[index] = TRENNING_TACTICKS;
                        break;
                    }
                }

                if (slotNumber == 4) {
                    clubRef.training[day - 1 + (7 * 3)] = TRENNING_EMPTY;
                }
            }//dla while
            pClub->save();
            break;
        }
    }
}

void Manager::menuItemTrenningIndividual()
{
    SClub &clubRef = pClub->get();
    wchar_t menuPersonalTrenning = 0;
    bool isNext = false;
    int mode = 0;
    do {
        info();
        pColors->textcolor(WHITE);
        wcout << endl << L"DOBÓR ZAWODNIKÓW DO TRENINGU INDYWIDUALNEGO";
        mode = isNext ? 40 : 20;

        pTeamComposition->draw(pFootballers->getPlayersTeam(), clubRef.teamSetting, mode, clubRef.clubId, true);

        size_t playersCount = pFootballers->getSizePlayerTeam();

        pColors->textcolor(GREEN);
        wcout << endl;
        if (playersCount > 20) {
            wcout << L"D Dalej  ";
        }
        wcout << L"T Zmiana treningu";

        pColors->textcolor(RED);
        wcout << L"  Q Powrót";
        pColors->textcolor(GREEN);

        menuPersonalTrenning = pInput->getKeyBoardPressed();
        switch (menuPersonalTrenning) {
            case 'D': { // dalej
                isNext = (!isNext && playersCount > 20) ? true : false;
                break;
            }
            case 'T': { // zmiana treningu
                pColors->textcolor(LIGHTGRAY);
                wcout << L" Podaj nr. Lp.: ";
                int footballerId = pInput->getNumber();

                wcout << L" Jaki trening? (1-B, 2-O, 3-P, 4-N): ";
                int trenning = pInput->getNumber();

                if (trenning < PLAYERS_TRENNING_B ||
                    trenning > PLAYERS_TRENNING_N
                ) {
                    break;
                }

                for (size_t i = 0; i < pFootballers->getSizePlayerTeam(); i++) {
                    SFootballer &footballer = pFootballers->getPlayerTeam(i);
                    if (footballer.data[0] == footballerId) {
                        footballer.data[1] = trenning;
                    }
                    if (footballer.data[15] > 0) {
                        footballer.data[1] = PLAYERS_TRENNING_NONE;
                    }
                }
                pFootballers->savePlayerTeam();
                break;
            }
        }
    }
    while (menuPersonalTrenning != 'Q');
}

void Manager::menuItemLastMatch()
{
    SClub &clubRef = pClub->get();
    if (clubRef.lastMatchRivalId <= 0) {
        pInput->clrscr();

        pColors->textcolor(LIGHTGRAY);
        wcout << L"RAPORT OSTATNIEGO MECZU";

        pColors->textcolor(RED);
        wcout << endl << endl << L"Nie rozegrano jeszcze żadnego meczu" << endl << endl;
        pColors->textcolor(LIGHTGRAY);
        wcout << endl << endl << L"Naciśnij dowolny klawisz...";
        pInput->getKeyBoardPressed();
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
    lastMatch.textcolor = LIGHTGRAY;
    wcscpy(lastMatch.text, L"Koniec meczu");
    lastMatchArray.push_back(lastMatch);

    int startFrom = 0;
    const int maxCount = 19;
    const int pages = ceil(lastMatchArray.size() / (float)maxCount);

    wchar_t keyPressed = 0;
    do {
        pInput->clrscr();
        pColors->textcolor(LIGHTGRAY);
        wcout << L"RAPORT OSTATNIEGO MECZU: ";
        pColors->textcolor(LIGHTBLUE);
        wcout << L" " << pClub->getClubName(clubRef.clubId - 1) << L" " << clubRef.playerGoals << L" ";
        pColors->textcolor(RED);
        wcout << L" " << clubRef.rivalGoals << L" " << pClub->getClubName(clubRef.lastMatchRivalId - 1) << L" ";
        wcout << endl;

        // wczytujemy max 19 raportow
        for (size_t index = startFrom, counter = 0;
            index < lastMatchArray.size() && counter < maxCount;
            ++index, ++counter
        ) {
            const SLastMatch &lastMatch = lastMatchArray[index];
            pColors->textcolor(lastMatch.textcolor);
            wcout << endl << lastMatch.text;
        }

        wcout << endl;

        pColors->textbackground(BLACK);
        pColors->textcolor(GREEN);
        wcout << endl << L"<- Poprzednia strona (" << (startFrom / maxCount) + 1 << "/" << pages << ") Następna strona ->";

        pColors->textcolor(RED);
        wcout << endl << L"Q Wyjście ";

        pColors->textcolor(GREEN);
        keyPressed = pInput->getKeyBoardPressed();
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
            pColors->textcolor(WHITE);
            wcout << L"NAJBLIŻSZY PRZECIWNIK - ";

            int opponenClubId = pRounds->getNearestRivalId(clubRef.roundNumber, clubRef.clubId);
            wcout << pClub->getClubName(opponenClubId);

            int tablePos = pTable->getPositionInTable(clubRef.rivalData[0]);

            wcout << L" " << tablePos << L" miejsce";
            wcout << L" " << (clubRef.rivalData[1] == 0 ? L"(Dom)" : L"(Wyjazd)");

            pTactic->drawTeamSetting(clubRef.rivalData[2], false);

            pTactic->drawChart(clubRef.rivalData[2], clubRef.rivalData[0], pFootballers->getRivals(), true);
            pTactic->drawChart(clubRef.teamSetting, clubRef.clubId, pFootballers->getPlayersTeam());

            pTeamComposition->draw(pFootballers->getRivals(), clubRef.rivalData[2], 11, clubRef.rivalData[0]);

            menu = pInput->getKeyBoardPressed();
            switch (menu) {
                case 'R': {
                    pInput->clrscr();

                    pColors->textcolor(WHITE);
                    wcout << endl << L"REZERWOWI";

                    pTeamComposition->draw(pFootballers->getRivals(), clubRef.rivalData[2], 16, clubRef.rivalData[0]);

                    pColors->textcolor(LIGHTGRAY);
                    wcout << endl << endl << L"Naciśnij dowolny klawisz...";
                    pInput->getKeyBoardPressed();
                    break;
                }
                case 'I': {
                    pInput->clrscr();
                    pColors->textcolor(WHITE);
                    wcout << endl << L"Instrukcje dla drużyny:";

                    pTeamInstr->draw(
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
            //runda wosenna, pokaz kolejki od 16 do 30
            roundNum = MAX_CLUBS - 1;
            mode = roundNum * 2;
            isSpring = true;
        }
        else {
            //runda jesienna, pokaz kolejki od 1 do 15
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

            pColors->textcolor(WHITE);
            wcout << endl << L"KALENDARZ - " << (isSpring ? L"runda wosenna" : L"runda jesienna");

            pColors->textcolor(LIGHTGRAY);
            if (clubRef.controlMatchesAmount <= 0 && !isSpring) {
                // mamy mecze kontrolne, dodaj je do rundy
                roundNum = clubRef.controlMatchesAmount - 1;
            }

            while (roundNum != mode) {
                roundNum++;
                int myClubIndex = 0;
                int oponentIndex = 0;

                myClubIndex = pRounds->getClubIndexInRoundByClubId(roundNum, clubRef.clubId);

                pColors->textcolor(roundNum <= 0 ? MAGENTA : LIGHTGRAY);

                bool isHome = myClubIndex % 2 == 0;
                if (isHome) { // u sibie
                    oponentIndex = myClubIndex + 1; // oponent jest następny (+1)
                }
                else { // wyjazd
                    oponentIndex = myClubIndex - 1; // oponent jest poprzedni (-1)
                }

                for (size_t index = 0; index < pRounds->getSize(); index++) {
                    const SRound &round = pRounds->get(index);
                    if (round.number == roundNum) {
                        wprintf(
                            L"\n\r%02d.%02d.%dr. %-19ls",
                            round.day,
                            round.month,
                            round.year + clubRef.season,
                            pClub->getClubName(round.clubNumbers[oponentIndex] - 1).c_str()
                        );

                    }
                }

                wprintf(L" %-9ls", (isHome) ? L"Dom" : L"Wyjazd");
                wprintf(L" %-11ls", (roundNum <= 0) ? L"Kontrolny" : L"I liga");

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
            pColors->textcolor(GREEN);
            if (isSpring) {
                wcout << endl << endl << L"D Pokaż rundę jesienną";
            }
            else {
                wcout << endl << endl << L"D Pokaż rundę wiosenną";
            }

            wcout << L"   M Mecze kontrolne";

            pColors->textcolor(RED);
            wcout << L"   Q Powrót do MENU";

            pColors->textcolor(GREEN);
            menu = pInput->getKeyBoardPressed();
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
        pColors->textcolor(LIGHTGRAY);
        wcout << endl << L"Podaj dzień, spacja, miesiąc - meczu kontrolnego: ";
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

        pColors->textcolor(LIGHTGRAY);
        wcout << endl << endl << L"Wybierz sparringpartnera:";
        for (int i = 0; i < MAX_CLUBS; i++) {
            if (i + 1 != clubRef.clubId) {
                wprintf(L"\n\r%2d. %ls", i + 1, pClub->getClubName(i).c_str());
            }
        }
        wcout << endl << endl << L"Wpisz odpowiedni numer: ";
        int rivalClubId = pInput->getNumber();
        if (rivalClubId < 0 ||
            rivalClubId > 16 ||
            rivalClubId == clubRef.clubId ||
            clubRef.controlMatchesAmount < -3 // max 4 mecze kontrolne?
        ) {
            canPlayControlMatch = false;
        }

        if (!canPlayControlMatch) {
            pColors->textcolor(RED);
            wcout << endl << L"Odrzucono propozycję.";
            pInput->getch();
        }
        else if (canPlayControlMatch) {
            clubRef.controlMatchesAmount--;
            clubRef.roundNumber--;
            clubRef.isRivalSet = 0;

            SRound round;
            round.day = wishDay;
            round.month = wishMonth;
            round.year = 2002;
            round.number = clubRef.controlMatchesAmount; //numer kolejki, liczba ujemna
            memset(round.clubNumbers, 0, MAX_CLUBS * sizeof(int));
            round.clubNumbers[0] = rivalClubId;
            round.clubNumbers[1] = clubRef.clubId;

            for (int i = 0; i < MAX_CLUBS; i++) {
                for (int j = 0; j < 19; j++) {
                    round.kol1[i][j] = pClub->getClubName(round.clubNumbers[i] - 1)[j];
                }
            }

            pRounds->insert(round);
            pRounds->save();
            pClub->save();
        }
    } // dla miesiąca 7
    else {
        pColors->textcolor(LIGHTRED);
        wcout << endl << L"Nie można rozegrać meczu kontrolnego." << endl;
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

        pColors->textcolor(GREEN);
        wcout << endl << L" W Wyniki    P Pozostałe mecze";

        pColors->textcolor(RED);
        wcout << L"    Q Powrót do MENU";

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

        menu = pInput->getKeyBoardPressed();
        switch (menu) {
            case 'W': { // wyniki
                maxRounds = 1;
                isShowPlayed = true; // tryb - mecze rozegrane
                roundNumber++; // zwiększ eszcze o jeden, bo zaraz w pętli while zmniejszysz o 1
                if (clubRef.roundNumber <= 0) {
                    break;
                }
                // break; // <- NO break!
            }
            case 'P': { // pozostale mecze
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
                        wcout << L"  Wyniki meczów I ligi - ";
                    }
                    else {
                        wcout << L"  Pozostałe mecze I ligi - ";
                    }
                    wcout << roundNumber << L" kolejka";

                    pColors->textcolor(LIGHTGRAY);

                    for (int i = 0; i < MAX_CLUBS; i++) {
                        for (size_t index = 0; index < pRounds->getSize(); index++) {
                            const SRound &round = pRounds->get(index);
                            if (roundNumber == round.number) {
                                pColors->textcolor(round.clubNumbers[i] == clubRef.clubId ? GREEN : LIGHTGRAY);

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
                    pColors->textcolor(GREEN);
                    wcout << endl << endl << L" Dalej - dowolny klawisz";

                    pColors->textcolor(RED);
                    wcout << endl << L" Q Wyjście ";

                    wchar_t yn = pInput->getKeyBoardPressed();
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

            wcout << endl << L"LISTA TRANSFEROWA";
            if (clubRef.finances[13] > 0) {
                pColors->textcolor(LIGHTGREEN);
            }
            else {
                pColors->textcolor(LIGHTRED);
            }
            wprintf(L"   Fundusze na transfery: %.2f zł.", clubRef.finances[13]);

            pColors->textcolor(GREEN);
            wcout << endl << L" Lp.   Zawodnik         Po.  ";

            pColors->textcolor(LIGHTBLUE);
            wcout << L"B  ";

            pColors->textcolor(MAGENTA);
            wcout << L"O  ";

            pColors->textcolor(LIGHTCYAN);
            wcout << L"P  ";

            pColors->textcolor(LIGHTGREEN);
            wcout << L"N  ";

            pColors->textcolor(GREEN);
            wcout << L"Morale  For. Kon.    Cena";

            if (loop >= transferPlayersCount) {
                loop = 0;
                mode = 20;
            }

            while (loop != mode) {
                loop++;

                for (size_t index = 0; index < pFootballers->getSizeTransfers(); index++) {
                    SFootballer &footballer = pFootballers->getTransfer(index);
                    if (footballer.data[0] == loop && footballer.data[22] != clubRef.clubId) {
                        bool isFilterPosistion = (filterPosition == footballer.data[2] || filterPosition == 0);
                        if (isFilterPosistion &&
                            (footballer.data[3] >= filterGoalkeeper &&
                            footballer.data[4] >= filterDefense &&
                            footballer.data[5] >= filterMiddlefield &&
                            footballer.data[6] >= filterAttact &&
                            footballer.data[9] >= filterForm &&
                            footballer.finances[0] >= filterMinPrice &&
                            footballer.finances[0] <= filterMaxPrice)
                        ) {
                            pColors->textcolor(WHITE);
                            pColors->textbackground(BLUE);
                        }
                        else {
                            pColors->textcolor(LIGHTGRAY);
                            pColors->textbackground(BLACK);
                        }
                        wprintf(L"\n\r%3d.  ", footballer.data[0]);
                        wprintf(
                            L"%3ls%-15ls %lc  %2d %2d %2d %2d  %-7ls  %2d  %3d%% %8.0f zł.",
                            footballer.name,
                            footballer.surname,
                            pTeamComposition->getFootballerPosition(footballer.data[2]),
                            footballer.data[3],
                            footballer.data[4],
                            footballer.data[5],
                            footballer.data[6],
                            pTeamComposition->getMorale(footballer.data[7]).c_str(),
                            footballer.data[9],
                            footballer.data[11],
                            footballer.finances[0]
                        );
                        pColors->textcolor(LIGHTGRAY);
                        pColors->textbackground(BLACK);

                        break;
                    }
                }
            }
            pColors->textcolor(GREEN);

            wcout << endl << L"D Dalej   F Filrty   K Kupno zawodnika   ";
            pColors->textcolor(RED);
            wcout << L"Q Powrót do MENU";
            pColors->textcolor(GREEN);

            menu = pInput->getKeyBoardPressed();
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

        pColors->textcolor(LIGHTGRAY);

        wcout << endl << L"Podaj numer Lp.: ";
        int footballerId = pInput->getNumber();

        for (size_t index = 0; index < pFootballers->getSizeTransfers(); index++) {
            SFootballer &footballer = pFootballers->getTransfer(index);
            if (footballer.data[0] == footballerId) {
                pInput->clrscr();
                int weight = 0;
                float cena = 0;
                wprintf(
                    L"\n\rFUNDUSZE NA TRANSFERY: %.2f zł." \
                    L"\n\r\n\rKupno zawodnika: %ls%ls",
                    clubRef.finances[13],
                    footballer.name,
                    footballer.surname
                );

                wcout << endl << L"Klub: " ;
                if (footballer.data[22] > 0) {
                    wcout << pClub->getClubName(footballer.data[22] - 1);
                }
                else {
                    wcout << L"żaden";
                }

                if (footballer.data[22] > 0) {
                    // podaj cene tylko wtedy gdy zawodnik nalezy do jakiegos klubu
                    // bo to kasa dla klubu za wypupienie
                    wcout << endl << endl;
                    wprintf(L"Podaj cenę (%.2f): ", footballer.finances[0]);
                    cena = pInput->getFloat();
                    if (cena > clubRef.finances[13] || cena < 0) {
                        weight -= 10;
                    }
                }
                else {
                    wcout << endl;
                }
                wprintf(L"\n\rPodaj płacę miesięczną (%.2f): ", footballer.finances[1]);
                float placa = pInput->getFloat();
                float premia = 0;
                if (footballer.data[2] == PLAYERS_POS_P ||
                    footballer.data[2] == PLAYERS_POS_N
                ) {
                    wprintf(L"\n\rPodaj premię za gola (%.2f): ", footballer.finances[2]);
                    premia = pInput->getFloat();
                }
                wcout << endl << L"Podaj przez ile lat ma obowiązywać kontrakt (1,2 lub 3): ";
                int lata = pInput->getNumber();
                if (footballer.data[22] > 0) {
                    float transfer = cena - footballer.finances[0];

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
                    float transfer = premia - footballer.finances[2];

                    if (transfer < 0) weight--;
                    if (transfer == 0) weight++;
                    if (transfer >= 50 && transfer < 100) weight += 2;
                    if (transfer >= 100) weight += 3;
                }
                else {
                    weight++;
                }

                float transfer = placa - footballer.finances[1];

                if (transfer < 0) weight--;
                if (transfer < -1000) weight -= 5;
                if (transfer == 0) weight++;
                if (transfer >= 100 && transfer < 500) weight += 2;
                if (transfer >= 500 && transfer < 1000) weight += 3;
                if (transfer >= 1000) weight += 4;

                if (lata == 1) weight += 2;
                if (lata == 2) weight++;
                if (lata < 1 || lata > 3) weight = -5;

                int sumSkills = footballer.data[3] + footballer.data[4] + footballer.data[5] + footballer.data[6];

                if (footballer.data[2] == PLAYERS_POS_B && sumSkills > 30) weight -= 2;
                if (sumSkills >= 30) weight--;
                if (sumSkills >= 50) weight -= 2;
                if (sumSkills < 20) weight++;
                if (sumSkills < 10) weight += 2;
                if (footballer.data[17] == 1) weight++;

                if (weight >= 6) {
                    pColors->textcolor(GREEN);

                    wcout << endl << footballer.surname << L": Zgadzam się na proponowane warunki kontraktu.";
                    pColors->textcolor(LIGHTGRAY);

                    wcout << endl << endl << L"Akceptujesz? (T/n): ";
                    wchar_t yn = pInput->getche();
                    if (yn == L't' || yn == L'\n') {
                        footballer.data[22] = clubRef.clubId;
                        footballer.finances[1] = placa;
                        footballer.finances[0] = cena;
                        if (footballer.data[2] == PLAYERS_POS_P ||
                            footballer.data[2] == PLAYERS_POS_N
                        ) {
                            footballer.finances[2] = premia;
                        }
                        footballer.data[18] = lata * 365;
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
                        clubRef.finances[8] += cena;
                        clubRef.finances[12] -= cena;
                        clubRef.finances[13] = clubRef.finances[12] / 2;
                        clubRef.finances[10] = clubRef.finances[6] + clubRef.finances[7] + clubRef.finances[8] + clubRef.finances[9];
                        clubRef.finances[11] = clubRef.finances[5] - clubRef.finances[10];
                        clubRef.managerStats[10]++;
                        clubRef.totalExpensesTransfers += cena;

                        // TODO: czy tu trzeba usunac zawodnika z listy transferowej bo bedzie zdublowany?
                        pFootballers->pushBackPlayerTeam(footballer);
                        pFootballers->savePlayerTeam();

                        pClub->save();
                    }
                }
                else {
                    pColors->textcolor(RED);
                    wcout << endl << footballer.surname << L": Odrzucam proponowane warunki kontraktu.";
                    pInput->getch();
                }

                break;
            }
        }
        pFootballers->saveTransfers();
    }
    else { // brak funduszy
        pColors->textcolor(LIGHTRED);
        wcout << endl << L"Brak funduszy na transfery!" << endl;
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
        pColors->textcolor(WHITE);
        wcout << endl << L"FILTRY DLA LISTY TRANSFEROWEJ";

        pColors->textcolor(LIGHTGRAY);
        wprintf(L"\n\rFundusze na transfery: %.2f zł.", clubRef.finances[13]);

        pColors->textcolor(GREEN);

        wstring pozycja = getFilterByPosition(filterPosition);

        setFilterColors(belka, 1);
        wprintf(L"\n\r\n\r   Względem pozycji: %ls ", pozycja.c_str());

        setFilterColors(belka, 2);
        wprintf(L"\n\r   Względem umiejętności: B: %d ", filterGoalkeeper);

        setFilterColors(belka, 3);
        wprintf(L"\n\r   Względem umiejętności: O: %d ", filterDefense);

        setFilterColors(belka, 4);
        wprintf(L"\n\r   Względem umiejętności: P: %d ", filterMiddlefield);

        setFilterColors(belka, 5);
        wprintf(L"\n\r   Względem umiejętności: N: %d ", filterAttact);

        setFilterColors(belka, 6);
        wprintf(L"\n\r   Względem formy: %d ", filterForm);

        setFilterColors(belka, 7);
        wprintf(L"\n\r M Względem ceny minimalnej:  od %.2f zł. ", filterMinPrice);

        setFilterColors(belka, 8);
        wprintf(L"\n\r X Względem ceny maksymalnej: do %.2f zł. ", filterMaxPrice);

        pColors->textcolor(RED);
        pColors->textbackground(BLACK);
        wcout << endl << L" Q Powrót" << endl;

        menuFilters = pInput->getKeyBoardPressed();
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
                    if (++filterMiddlefield == 21) {
                        filterMiddlefield = 1;
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
                    if (--filterMiddlefield == 0) {
                        filterMiddlefield = 20;
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
                pColors->textcolor(LIGHTGRAY);
                wprintf(L"\n\rPodaj minimalną cenę zawodnika: ");
                filterMinPrice = pInput->getFloat();
                break;
            }
            case 'X': {
                pColors->textcolor(LIGHTGRAY);
                wprintf(L"\n\rPodaj maksymalną cenę zawodnika: ");
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

void Manager::menuItemFinance()
{
    SClub &clubRef = pClub->get();
    if (clubRef.clubId > 0) {
        info();
        pColors->textcolor(YELLOW);
        wcout << endl << L"FINANSE";
        pColors->textcolor(LIGHTGREEN);
        wprintf(
            L"\n\rPrzychody w tym miesiącu:                  Przychody w zeszłym miesiącu:" \
            L"\n\r%12.2f zł. - Bilety               %12.2f zł. - Bilety" \
            L"\n\r%12.2f zł. - Dochód z TV          %12.2f zł. - Dochód z TV" \
            L"\n\r%12.2f zł. - Handel               %12.2f zł. - Handel" \
            L"\n\r%12.2f zł. - Sprzedaż zawodników  %12.2f zł. - Sprzedaż zawodników" \
            L"\n\r%12.2f zł. - Reklamy              %12.2f zł. - Reklamy" \
            L"\n\r%12.2f zł. - RAZEM                %12.2f zł. - RAZEM",
            clubRef.finances[0],
            clubRef.financesLastMonth[0],
            clubRef.finances[1],
            clubRef.financesLastMonth[1],
            clubRef.finances[2],
            clubRef.financesLastMonth[2],
            clubRef.finances[3],
            clubRef.financesLastMonth[3],
            clubRef.finances[4],
            clubRef.financesLastMonth[4],
            clubRef.finances[5],
            clubRef.financesLastMonth[5]
        );
        pColors->textcolor(LIGHTRED);
        wprintf(
            L"\n\rStraty w tym miesiącu:                     Straty w zeszłym miesiącu:" \
            L"\n\r%12.2f zł. - Płace                %12.2f zł. - Płace" \
            L"\n\r%12.2f zł. - Premie               %12.2f zł. - Premie" \
            L"\n\r%12.2f zł. - Kupno zawodników     %12.2f zł. - Kupno zawodników" \
            L"\n\r%12.2f zł. - Kary ligowe          %12.2f zł. - Kary ligowe" \
            L"\n\r%12.2f zł. - RAZEM                %12.2f zł. - RAZEM",
            clubRef.finances[6],
            clubRef.financesLastMonth[6],
            clubRef.finances[7],
            clubRef.financesLastMonth[7],
            clubRef.finances[8],
            clubRef.financesLastMonth[8],
            clubRef.finances[9],
            clubRef.financesLastMonth[9],
            clubRef.finances[10],
            clubRef.financesLastMonth[10]
        );
        pColors->textcolor(clubRef.finances[11] > 0 ? GREEN : RED);
        wprintf(L"\n\r\n\rZysk w tym miesiącu: %.2f zł.", clubRef.finances[11]);
        pColors->textcolor(LIGHTGRAY);
        wprintf(L"   Zysk w zeszłym miesiącu: %.2f zł.", clubRef.financesLastMonth[11]);
        wprintf(
            L"\n\r\n\rKasa klubu: %.2f zł." \
            "\n\rFundusze na transfery: %.2f zł.",
            clubRef.finances[12],
            clubRef.finances[13]
        );
        wcout << endl << endl << L"Powrót do MENU - dowolny klawisz...";
        pInput->getch();
    }
}

void Manager::menuItemManagement()
{
    SClub &clubRef = pClub->get();
    if (clubRef.clubId > 0) {
        wchar_t menu = 0;
        do {
            info();
            wcout << endl << endl << L"ZARZĄD KLUBU";
            pColors->textcolor(LIGHTGRAY);

            int tablePos = pTable->getPositionInTable(clubRef.clubId);

            if (tablePos == 0) {
                pNews->messages[8] = 43;
            }
            else if (tablePos < 4) {
                pNews->messages[8] = 31;
            }
            else if (tablePos > 3 && tablePos < 10) {
                pNews->messages[8] = 32;
            }
            else if (tablePos > 9 && tablePos < 14) {
                pNews->messages[8] = 33;
            }
            else if (tablePos > 13) {
                pNews->messages[8] = 34;
            }

            if (clubRef.finances[12] <= 500000 && clubRef.finances[11] <= 0)        pNews->messages[9] = 35;
            else if (clubRef.finances[12] <= 500000 && clubRef.finances[11] > 0)    pNews->messages[9] = 36;
            else if (clubRef.finances[12] <= 1000000 && clubRef.finances[11] <= 0)  pNews->messages[9] = 37;
            else if (clubRef.finances[12] <= 1000000 && clubRef.finances[11] > 0)   pNews->messages[9] = 38;
            else if (clubRef.finances[12] <= 2000000 && clubRef.finances[11] <= 0)  pNews->messages[9] = 39;
            else if (clubRef.finances[12] <= 2000000 && clubRef.finances[11] > 0)   pNews->messages[9] = 40;
            else if (clubRef.finances[12] > 2000000 && clubRef.finances[11] <= 0)   pNews->messages[9] = 41;
            else if (clubRef.finances[12] > 2000000 && clubRef.finances[11] > 0)    pNews->messages[9] = 42;

            for (int i = 8; i < MAX_MESSAGES; i++) {
                wcout << endl << endl;
                const SNews& news = pNews->getManagerNewsByNumber(pNews->messages[i]);
                if (news.num > 0) {
                    wprintf(news.message, pNews->stringForMessage[i - 1].c_str(), pNews->numbersForMessage[i - 1]);
                }
            }
            pNews->messages[8] = 0;
            pNews->messages[9] = 0;

            pColors->textcolor(GREEN);
            wcout << endl << endl << L" T Prośba o dodatkowe fundusze na transfery";

            pColors->textcolor(RED);
            wcout << endl << L" Q Powrót do MENU" << endl;
            menu = pInput->getKeyBoardPressed();
            switch (menu) {
                case 'T': {
                    int chance = 1;
                    if (clubRef.finances[12] <= 0)                                              chance = 1; // bez szans
                    else if (clubRef.finances[12] > 0 && clubRef.finances[12] <= 500000)        chance = (rand() % 6); // 1/6 szansy
                    else if (clubRef.finances[12] > 500000 && clubRef.finances[12] <= 1000000)  chance = (rand() % 5); // 1/5 szansy
                    else if (clubRef.finances[12] > 1000000 && clubRef.finances[12] <= 2000000) chance = (rand() % 4); // 1/4 szansy
                    else if (clubRef.finances[12] > 2000000)                                    chance = (rand() % 3); // 1/3 szansy

                    if (chance == 0 && !clubRef.isBlockTransferFunds) {
                        pColors->textcolor(GREEN);
                        wcout << endl << endl <<
                            L"Zarząd: Zgadzamy się na pańską prośbę. Mamy nadzieję, iż rozsądnie wykorzysta pan fundusze i wzmocni zespół.";
                        clubRef.finances[13] = clubRef.finances[12];
                        clubRef.isBlockTransferFunds = 1;

                        pClub->save();
                    }
                    else {
                        pColors->textcolor(RED);
                        clubRef.isBlockTransferFunds = 1;
                        wcout << endl << endl << L"Zarząd: Odrzucamy pańską prośbę.";

                        pClub->save();
                    }
                    pInput->getch();
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
    wcout << endl << endl << L"MANAGER - STAYSTKI";
    pColors->textcolor(LIGHTGRAY);
    wprintf(
        L"\n\r" \
        L"\n\rWygrane ligi:           %4d"\
        L"\n\rPrzyznane nagrody:      %4d"\
        L"\n\rPunkty managera:        %4d"\
        L"\n\rRozegrane mecze:        %4d"\
        L"\n\rMecze wygrane:          %4d"\
        L"\n\rMecze zremisowane:      %4d" \
        L"\n\rMecze przegrane:        %4d" \
        L"\n\rGole zdobyte:           %4d" \
        L"\n\rGole stracone:          %4d" \
        L"\n\rKupionych zawodników:   %4d - łącznie %.2f zł." \
        L"\n\rSprzedanych zawodników: %4d - łącznie %.2f zł.",
        clubRef.managerStats[0],
        clubRef.managerStats[2],
        clubRef.managerStats[3],
        clubRef.managerStats[4],
        clubRef.managerStats[5],
        clubRef.managerStats[6],
        clubRef.managerStats[7],
        clubRef.managerStats[8],
        clubRef.managerStats[9],
        clubRef.managerStats[10],
        clubRef.totalExpensesTransfers,
        clubRef.managerStats[11],
        clubRef.totalRevenuesTransfers);

    wcout << endl << endl << L"Powrót do MENU - dowolny klawisz...";

    pInput->getKeyBoardPressed();
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
        pColors->textcolor(GREEN);
        wcout << endl << endl << L"STARE WIADOMOŚCI" << endl;

        // wczytujemy maxCount wiadomosci
        for (int index = startFrom, counter = maxCount - 1;
            index >= 0 && counter >= 0;
            --index, --counter
        ) {
            wcout << endl << pNews->getManagerMessage(index) << endl;
        }

        wcout << endl;

        pColors->textbackground(BLACK);
        pColors->textcolor(GREEN);
        wcout << endl << L"<- Nowsze wiadomości (" << (startFrom / maxCount) + 1 << "/" << pages << ") Starsze wiadomości ->";

        pColors->textcolor(RED);
        wcout << endl << L"Q Wyjście ";

        pColors->textcolor(GREEN);
        ch = pInput->getKeyBoardPressed();
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
    const SClub& clubRef = pClub->get();

    info();
    pColors->textcolor(GREEN);
    wcout << endl << endl << L"WIADOMOŚCI";
    pColors->textcolor(LIGHTGRAY);
    if (pNews->messages[0] == 1) { //czy wogóle jest jakaś wiadomość
        for (int i = 1; i < MAX_MESSAGES; i++) {
            wcout << endl << endl;
            const SNews& news = pNews->getManagerNewsByNumber(pNews->messages[i]);
            if (news.num > 0) {
                wprintf(news.message, pNews->stringForMessage[i - 1].c_str(), pNews->numbersForMessage[i - 1]);

                pNews->addManagerMessage(news, clubRef, pNews->stringForMessage[i - 1].c_str(), pNews->numbersForMessage[i - 1]);
            }
        }
        pNews->saveManagerMessages();
        pInput->getKeyBoardPressed();
    }

    memset(pNews->messages, 0, MAX_MESSAGES * sizeof(int));
}

void Manager::menuItemOptions()
{
    SClub &clubRef = pClub->get();

    bool isOptionCorrect = false;
    int count = 0;
    do {
        pInput->clrscr();
        pColors->textbackground(BLACK);

        if (count > 0) {
            pColors->textcolor(RED);
            wcout << L"Błąd. Niepoprawna cyfra. Wpisz 1 lub 2.";
        }

        pColors->textcolor(LIGHTGRAY);
        wcout << endl
            << L"Jeżeli czas wyświetlania komunikatów w czasie meczu jest nieadekwatny do" << endl
            << L"ustawień Szybkości, zmień poniższą opcję na Ręczne." << endl << endl
            << L"Wyświetlanie kolejnych komunikatów w czasie meczu odbywać ma się:" << endl
            << L"1. Automatycznie (zalecane)" << endl
            << L"2. Ręcznie (w razie problemów)" << endl << endl
            << L"Wpisz odpowiednią cyfrę (1 lub 2): ";

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
        case 3:     return L" - sortowany wg umiejętności B";
        case 4:     return L" - sortowany wg umiejętności O";
        case 5:     return L" - sortowany wg umiejętności P";
        case 6:     return L" - sortowany wg umiejętności N";
        case 7:     return L" - sortowany wg morale";
        case 9:     return L" - sortowany wg formy";
        case 11:    return L" - sortowany wg kondycji";
        case 16:    return L" - sortowany wg goli";
        case 18:    return L" - sortowany wg kolejności wygaśnięcia kontraktu";
        default:    return L"";
    }
}

wstring Manager::getTrenningDayName(int dayNumber)
{
    switch (dayNumber) {
        case 1:  return L"Poniedziałku";
        case 2:  return L"Wtorku";
        case 3:  return L"Środy";
        case 4:  return L"Czwartku";
        case 5:  return L"Piątku";
        case 6:  return L"Soboty";
        case 7:  return L"Niedzieli";
        default: return L"";
    }
}

wstring Manager::getFilterByPosition(int pos)
{
    switch (pos) {
        case 0:  return L"Wszyscy";
        case 1:  return L"Bramkarze";
        case 2:  return L"Obrońcy";
        case 3:  return L"Pomocnicy";
        case 4:  return L"Napastnicy";
        default: return L"Błąd";
    }
}

void Manager::setFilterColors(int bar, int barValue)
{
    if (bar == barValue) {
        pColors->textbackground(BLUE);
        pColors->textcolor(LIGHTGRAY);
    }
    else {
        pColors->textbackground(BLACK);
        pColors->textcolor(GREEN);
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

        int counter = -1;
        if (clubRef.playerGoals <= clubRef.rivalGoals) {
            // przegrana
            for (int i = 1; i < MAX_MESSAGES; i++) {
                if (pNews->messages[i] == 0) {
                    counter = 0;
                    if (AnaO < -20 && clubRef.inst[4] == 0) {
                        pNews->messages[i + counter] = 20;
                        counter++;
                    }
                    if (PnaP < -10 && clubRef.inst[0] != 4) {
                        pNews->messages[i + counter] = 21;
                        counter++;
                    }
                    if (OnaA < 0 && clubRef.inst[3] == 0) {
                        pNews->messages[i + counter] = 22;
                        counter++;
                    }
                    if (OnaA > 20 && clubRef.inst[3] == 1) {
                        pNews->messages[i + counter] = 23;
                        counter++;
                    }
                    if (PnaP > 10 && clubRef.inst[0] != 2) {
                        pNews->messages[i + counter] = 24;
                        counter++;
                    }
                    if (AnaO > 0 && clubRef.inst[4] == 1) {
                        pNews->messages[i + counter] = 25;
                        counter++;
                    }
                    if (clubRef.inst[2] == 0) {
                        pNews->messages[i + counter] = 26;
                        counter++;
                    }
                    if (clubRef.inst[2] == 1) {
                        pNews->messages[i + counter] = 27;
                        counter++;
                    }
                    if (clubRef.inst[1] != 3) {
                        pNews->messages[i + counter] = 28;
                        counter++;
                    }
                    if (counter > 0) {
                       pNews->messages[0] = 1;
                    }

                    break;
                }
            }
        }
        else { // wygrana meczu
            int los = rand() % 2;
            for (int i = 1; i < MAX_MESSAGES; i++) {
                if (pNews->messages[i] == 0) {
                    // ASYSTENT: Gratuluję zwycięstwa. Dobrał pan właściwą taktykę.
                    // ASYSTENT: Gratuluję wygranej. Nie mam zastrzeżeń co do zastosowanej taktyki.
                    pNews->messages[i] = los == 0 ? 29 : 30;
                    pNews->messages[0] = 1;
                    break;
                }
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
            clubRef.rivalData[3] = clubIndexInRound + 1; // wiec rywal jest na prawo odemnie
            clubRef.rivalData[1] = 0;
        }
        else { // gram na wyjezdzie
            clubRef.rivalData[3] = clubIndexInRound - 1; // wiec rywal jest na lewo odemnie
            clubRef.rivalData[1] = 1;
        }

        // wez znajdz i przypisz do clubRef.rivalData[0], numer klubu rywala z kolejki
        clubRef.rivalData[0] = pRounds->getClubIdInRoundByClubIndex(clubRef.roundNumber, clubRef.rivalData[3]);
        clubRef.rivalData[2] = getRivalSetting(clubRef.rivalData[0]);

        clubRef.rivalInst[0] = (rand() % 4) + 1; //podania zawsze los
        clubRef.rivalInst[1] = (rand() % 3 == 0) // traktowanie rywala
            ? 3
            : 1; //nigdy delikatne

        clubRef.rivalInst[2] = (rand() % 4 == 0) //pressing 3/4 że tak
            ? 0
            : 1;

        clubRef.rivalInst[3] = getRivalOffsideTrap(clubRef.rivalData[2]);
        clubRef.rivalInst[4] = getRivalContra(clubRef.rivalData[2]);

        // nastawienie rywala: normlane, obronne, atak
        clubRef.rivalInst[5] = getRivalAttitude(clubRef.rivalData[2]);

        pClub->save();

        for (size_t i = 0; i < pFootballers->getSizeRivals(); i++) {
            SFootballer &footballer = pFootballers->getRival(i);
            footballer.data[7] = rand() % 4; // morale od srednie do super
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
        case T4_3_3:
            return 1; //pułapki ofsajdowe tak

        case T5_3_2:
        case T5_3_2_ATT:
            return rand() % 2; // losowo tak/nie

        default:
            return 0; //pułapki ofsajdowe nie
    }
}

int Manager::getRivalContra(int rivalSetting)
{
    switch (rivalSetting) {
        case T4_4_2_DEF:
        case T5_3_2_DEF:
            return 1; // counter-game yes

        case T4_4_2_ATT:
        case T4_3_3:
        case T5_3_2_ATT:
            return 0; // counter-game no

        default:
            return rand() % 2; // random 50/50
    }
}

int Manager::getRivalAttitude(int rivalSetting)
{
    switch (rivalSetting) {
        case T4_4_2_DEF:
        case T5_3_2_DEF:
            return (rand() % 3 == 0) ? INSTR_ATTIT_NORMAL : INSTR_ATTIT_DEFENSIVE; // 2/3 że obronne, atak wcale

        case T4_4_2_ATT:
        case T4_3_3:
        case T5_3_2_ATT:
            return (rand() % 3 == 0) ? INSTR_ATTIT_NORMAL : INSTR_ATTIT_ATTACK; //2/3 że atak, obronne wcale

        case T3_5_2:
        case T5_3_2:  {
            switch ((rand() % 4)) {
                case 0:     return INSTR_ATTIT_DEFENSIVE; // 1/4 ze nastawienie obronne
                case 1:     return INSTR_ATTIT_ATTACK; // 1/4 ze nastawienie atak
                default:    return INSTR_ATTIT_NORMAL; // 2/4 że normalne
            }
        }
        default:
            return (rand() % 3 == 0) ? INSTR_ATTIT_ATTACK : INSTR_ATTIT_NORMAL; // 2/3 że normalne, obronne wcale
    }
}
