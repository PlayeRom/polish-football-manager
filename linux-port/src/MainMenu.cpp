
#include <sys/stat.h>
#include <iostream>
#include <iomanip>
#include "MainMenu.h"
#include "ConioColors.h"
#include "Footballers.h"
#include "Manager.h"

MainMenu::MainMenu(
    const Colors *pColors,
    const Input *pInput,
    Footballers *pFootballers,
    PlayerClub *pClub,
    Table *pTable,
    Rounds *pRounds,
    News *pNews
) {
    this->pColors = pColors;
    this->pInput = pInput;
    this->pFootballers = pFootballers;
    this->pClub = pClub;
    this->pTable = pTable;
    this->pRounds = pRounds;
    this->pNews = pNews;
}

/**
 * Main function for run the main menu and game
 *
 * @return void
 */
void MainMenu::run()
{
    wchar_t menu = 0;
    Manager manager(pClub, pColors, pInput, pFootballers, pTable, pRounds, pNews);

    do {
        draw();

        menu = pInput->getKeyBoardPressed();

        bool isNewGame = false;
        switch (menu) {
            case 'N': {
                isNewGame = newGame();
                if (!isNewGame) {
                    break;
                }

                // break; // no break!
            }
            case 'W': { // wczytaj grę
                loadGame(isNewGame);

                manager.runManager();
            }
        }
    }
    while (menu != 'Q');
}

/**
 * Draw the main menu of game
 *
 * @return void
 */
void MainMenu::draw()
{
    pInput->clrscr();

    pColors->textbackground(BLACK);
    pColors->textcolor(WHITE);
    wcout << L"PlayeRom" << endl;

    pColors->textcolor(LIGHTGRAY);
    wcout << L" presents:";

    pColors->textbackground(WHITE);
    pColors->textcolor(RED);
    wcout << endl << endl << L" MANAGER LIGI  ";

    pColors->textbackground(RED);
    pColors->textcolor(LIGHTGRAY);
    wcout << endl << L" POLSKIEJ 2002 ";

    pColors->textbackground(BLACK);

    pColors->textcolor(DARKGRAY);
    wcout << L"  v.1.04";

    pColors->textcolor(GREEN);
    wcout << endl << endl <<
        L"N Nowa gra" << endl <<
        L"W Wczytaj grę";

    pColors->textcolor(RED);
    wcout << endl << L"Q Wyjście" << endl;

    pColors->textcolor(GREEN);
}

/**
 * Run the new game
 *
 * @return bool
 */
bool MainMenu::newGame()
{
    pColors->textcolor(GREEN);
    wcout << endl << endl << L"Czy na pewno chcesz rozpocząć nową grę? (T/n): ";
    wchar_t yn = pInput->getKeyBoardPressed();
    if (yn != L'T' && yn != L'\n') {
        return false;
    }

    checkSaveDir();

    pFootballers->initialRivals();
    pFootballers->saveRivals();

    pInput->clrscr();
    pColors->textcolor(LIGHTGRAY);

    wchar_t name[MAX_USER_NAME] = {0};
    wchar_t surname[MAX_USER_SURNAME] = {0};
    wchar_t nick[MAX_USER_NICK] = {0};

    wcout << endl << L"Podaj swoje imię: ";
    pInput->getText2Buffer(name, MAX_USER_NAME);

    wcout << L"Podaj swoje nazwisko: ";
    pInput->getText2Buffer(surname, MAX_USER_SURNAME);

    wcout << endl << L"Czy chcesz podać swoją ksywę? (T/n): ";
    yn = pInput->getKeyBoardPressed();
    if (yn == L'T' || yn == L'\n') {
        wcout << endl << L"Podaj swój nickname: ";
        pInput->getText2Buffer(nick, MAX_USER_NICK);
    }

    int clubId = 0;
    bool isClubSelected = false;
    while (!isClubSelected) {
        pInput->clrscr();

        wcout << endl << L"Wybierz klub, którego będziesz Managerem:";
        for (int i = 0; i < MAX_CLUBS; i++) {
            wcout << endl << std::setfill(L' ') << std::setw(2) << i + 1 << ". " << pClub->getClubName(i);
        }
        wcout << endl << endl << L"Wpisz odpowiedni numer: ";
        //scanf(L"%d", &clubRef.clubId);

        clubId = pInput->getNumber();;
        isClubSelected = (clubId >= 1 && clubId <= MAX_CLUBS);
    }

    pFootballers->initialPlayerTeam(clubId);
    pFootballers->savePlayerTeam();

    pTable->createTable();
    pTable->save();

    pFootballers->initialTransfers();

    pFootballers->saveTransfers();

    pClub->init(name, surname, nick, clubId);
    pClub->save();

    pRounds->initial();
    pRounds->save();

    // PREZES: Panie %ls, witam pana w nowym miejscu pracy i mam nadzieję iż spełni pan nasze oczekiwania.
    pNews->setTmpMsgData(1, pClub->get().managerSurname);

    // ASYSTENT: Radzę panu ustalić kilka meczy kontrolnych. Pozwoli to panu lepiej przygotować się do rozgrywek ligowych.
    pNews->setTmpMsgData(7);

    pNews->clearManagerMessages();

    FILE *f = fopen(FILE_SAVE_LAST_MATCH_REPORTS, "w");
    fclose(f);

    return true;
}

/**
 * Load the game
 *
 * @param isNewGame
 * @return
 */
bool MainMenu::loadGame(bool isNewGame)
{
    if (isNewGame) {
        return false; // nothing loaded
    }

    pClub->load();
    pFootballers->loadPlayerTeam();
    pFootballers->loadRivals();
    pFootballers->loadTransfers();
    pTable->load();
    pRounds->load();
    pNews->loadManagerMessages();
    return true;
}

/**
 * Check whether save dir exist. If not create it.
 * @retrun bool
 */
bool MainMenu::checkSaveDir()
{
    struct stat st;
    if (stat("save", &st) == 0) {
        return true; // dir exist
    }

    // crate save dir
    if (mkdir("save", 0775) != 0) {
        return true;
    }

    return false;
}