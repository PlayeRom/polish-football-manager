
#include <stdio.h>
#include <stdexcept>
#include "Rounds.h"

Rounds::Rounds()
{
}

Rounds::Rounds(const Rounds& orig)
{
}

Rounds::~Rounds()
{
}

/**
 * Load rounds for initial file
 */
void Rounds::initial()
{
    rounds.clear();

    FILE *f = fopen(FILE_ROUNDS, "rb");
    if (!f) {
        char message[512];
        snprintf(message, 512, ERROR_OPEN_FILE, FILE_ROUNDS);
        throw std::invalid_argument(message);
    }

    SRound round;
    while (fread(&round, sizeof (SRound), 1, f) == 1) {
        rounds.push_back(round);
    }
    fclose(f);
}

/**
 * Load saved rouds from game file
 */
void Rounds::load()
{
    rounds.clear();

    FILE *f = fopen(FILE_SAVE_ROUNDS, "rb");
    if (!f) {
        char message[512];
        snprintf(message, 512, ERROR_OPEN_FILE, FILE_SAVE_ROUNDS);
        throw std::invalid_argument(message);
    }

    SRound round;
    while (fread(&round, sizeof (SRound), 1, f) == 1) {
        rounds.push_back(round);
    }
    fclose(f);
}

/**
 * Save round to game file
 */
void Rounds::save()
{
    FILE *f = fopen(FILE_SAVE_ROUNDS, "wb");
    fclose(f);

    f = fopen(FILE_SAVE_ROUNDS, "ab");
    for (size_t i = 0; i < rounds.size(); i++) {
        fwrite(&rounds[i], sizeof (SRound), 1, f);
    }
    fclose(f);
}

/**
 * Znjadz index podanego klubu w podanej kolejce
 *
 * @param roundNumber
 * @param clubId
 * @return
 */
int Rounds::getClubIndexInRoundByClubId(int roundNumber, int clubId)
{
    for (size_t index = 0; index < rounds.size(); index++) {
        if (roundNumber == rounds[index].number) {
            for (int i = 0; i < MAX_CLUBS; i++) {
                if (clubId == rounds[index].clubNumbers[i]) {
                    return i; // index klubu gracza w kolejce
                }
            }
        }
    }

    return 0; //error
}

int Rounds::getClubIdInRoundByClubIndex(int roundNumber, int clubIndex)
{
    for (size_t index = 0; index < rounds.size(); index++) {
        if (roundNumber == rounds[index].number) {
            return rounds[index].clubNumbers[clubIndex];
        }
    }

    return 0; // error
}

/**
 * Zwraca ID klubu przeciwnika, w podanej kolejce dla podanego klubu (clubId).
 *
 * @param roundNumber Numer kolejki
 * @param clubId ID klubu dla którego szukamy ID klubu przeciwnika
 * @return
 */
int Rounds::getNearestRivalId(int roundNumber, int clubId)
{
    for (size_t i = 0; i < rounds.size(); i++) {
        SRound &round = rounds[i];
        if (roundNumber == round.number) {
            // szukamy najpier siebie (clubId)
            for (int j = 0; j < MAX_CLUBS; j++) {
                if (clubId == round.clubNumbers[j]) {
                    // jak c jest nieparzyste, to wyjazd, a oponen jest c -1
                    // jak c jest parzyste, to dom, a oponent jest c +1
                    if (j % 2 == 0) {
                        return round.clubNumbers[j + 1] - 1;
                    }

                    return round.clubNumbers[j - 1] - 1;
                }
            }
            break;
        }
    }
}
