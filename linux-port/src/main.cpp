
#include <cstdlib>

#include "Manager.h"
#include "Colors.h"
#include "Input.h"
#include "Rounds.h"
#include "PlayerClub.h"
#include "Logger.h"

using namespace std;

int main(int argc, char** argv)
{
    setlocale(LC_ALL, ""); // for display Polish chars

    try {
        Input input;
        Colors colors;
        Footballers footballers;
        Table table(&colors);
        Rounds rounds;
        PlayerClub club;
        News news;

        Manager manager(&club, &colors, &input, &footballers, &table, &rounds, &news);
        manager.runManager();
    }
    catch (std::exception &exc) {
        cerr << endl << exc.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

