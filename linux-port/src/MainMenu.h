
#ifndef MAINMENU_H
#define MAINMENU_H

#include "Input.h"
#include "Colors.h"
#include "Footballers.h"
#include "PlayerClub.h"
#include "Table.h"
#include "Rounds.h"
#include "News.h"
#include "Language.h"

using namespace std;

class MainMenu {
public:
    MainMenu(
        const Colors *pColors,
        const Input *pInput,
        Footballers *pFootballers,
        PlayerClub *pClub,
        Table *pTable,
        Rounds *pRounds,
        News *pNews,
        Language *pLang
    );

    void run();

private:
    const Colors *pColors;
    const Input *pInput;
    Footballers *pFootballers;
    PlayerClub *pClub;
    Table *pTable;
    Rounds *pRounds;
    News *pNews;
    Language *pLang;

    void draw();
    bool newGame();
    bool loadGame(bool isNewGame);
    bool checkSaveDir();
};

#endif /* MAINMENU_H */
