
#include <cstdlib>
#include <random>

#include "Colors.h"
#include "Input.h"
#include "Rounds.h"
#include "PlayerClub.h"
#include "Logger.h"
#include "MainMenu.h"
#include "Language.h"
#include "ArgumentsReader.h"
#include "Random.h"

using namespace std;

int main(int argc, char** argv)
{
    setlocale(LC_ALL, ""); // for display Polish chars

    // To make the backspace key work when typing on the keyboard
    std::locale::global(std::locale(""));
    std::wcin.imbue(std::locale());
    std::wcout.imbue(std::locale());

    try {
        ArgumentsReader argsReader(argc, argv);
        if (argsReader.isExitFlag()) {
            return EXIT_FAILURE;
        }

        Random random;

        Language lang;
        lang.load(argsReader.getLanguage());

        Input input;
        Colors colors;
        Footballers footballers(&random);
        Table table(&colors, &lang);
        Rounds rounds;
        PlayerClub club;
        News news(&lang);

        MainMenu mainMenu(
            &colors,
            &input,
            &footballers,
            &club,
            &table,
            &rounds,
            &news,
            &lang,
            &random
        );
        mainMenu.run();
    }
    catch (std::exception &exc) {
        cerr << endl << exc.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
