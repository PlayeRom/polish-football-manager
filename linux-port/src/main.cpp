
#include <cstdlib>

#include "Colors.h"
#include "Input.h"
#include "Rounds.h"
#include "PlayerClub.h"
#include "Logger.h"
#include "MainMenu.h"

using namespace std;

int main(int argc, char** argv)
{
    setlocale(LC_ALL, ""); // for display Polish chars

    srand(time(NULL));

    try {
        Input input;
        Colors colors;
        Footballers footballers;
        Table table(&colors);
        Rounds rounds;
        PlayerClub club;
        News news;

        MainMenu mainMenu(
            &colors,
            &input,
            &footballers,
            &club,
            &table,
            &rounds,
            &news
        );
        mainMenu.run();
    }
    catch (std::exception &exc) {
        cerr << endl << exc.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
