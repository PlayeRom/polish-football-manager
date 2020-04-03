
#ifndef RANDOM_H
#define RANDOM_H

#include <random>

using namespace std;

class Random {
public:
    int get(int included) const;
    int get0(int to) const;
    int get(int from, int to) const;

private:
    int random(int from, int to) const;
};

#endif /* RANDOM_H */
