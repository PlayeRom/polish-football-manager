
#ifndef ROUNDS_H
#define ROUNDS_H

#include <vector>
#include <stddef.h>
#include "Structs.h"

class Rounds {
public:
    Rounds();
    Rounds(const Rounds& orig);
    virtual ~Rounds();

    size_t getSize() const { return rounds.size(); }
    SRound& get(int index) { return rounds[index]; }
    void insert(const SRound &round) { rounds.insert(rounds.begin(), round); }

    void initial();
    void load();
    void save();

    int getClubIndexInRoundByClubId(int roundNumber, int clubId);
    int getClubIdInRoundByClubIndex(int roundNumber, int clubIndex);
    int getNearestRivalId(int roundNumber, int clubId);

private:
    std::vector<SRound> rounds;
};

#endif /* ROUNDS_H */

