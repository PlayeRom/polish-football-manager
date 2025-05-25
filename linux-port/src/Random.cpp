
#include "Random.h"

/**
 * Random number from 1 do given number (included)
 * @param int included
 * @return int
 */
int Random::get(int included) const
{
    if (included < 1) {
        return 0;
    }

    return random(1, included);
}

/**
 * Random number from 0 to given value - 1
 * @param int to
 * @return int
 */
int Random::get0(int to) const
{
    if (to < 2) {
        return 0;
    }

    return random(0, to - 1);
}

/**
 * Random number from given range (included)
 *
 * @param int from
 * @param int to
 * @return int
 */
int Random::get(int from, int to) const
{
    return random(from, to);
}

/**
 * Random number from given range (included)
 *
 * @param int from
 * @param int to
 * @return int
 */
int Random::random(int from, int to) const
{
    random_device randDev;
    default_random_engine randomEngine(randDev());
    uniform_int_distribution<int> uniform_dist(from, to);
    return uniform_dist(randomEngine);
}