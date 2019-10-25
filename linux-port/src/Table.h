
#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <vector>
#include "Structs.h"
#include "Colors.h"
#include "Rounds.h"
#include "PlayerClub.h"

class Table {
public:
    Table(const Colors *pColors);
    Table(const Table& orig);
    virtual ~Table();

    void createTable();
    void save();
    void load();
    int getPositionInTable(int clubNumber);
    int getClubNumberInPosition(int tablePosition);
    void drawTable(int clubId, const PlayerClub *club);
    void updateAfterMatch(const SRound &round, int sumaP, SClub &clubRef);

    STable& get(int index) { return table[index]; }
    int getSize() const { return MAX_CLUBS; }
    void overrideTable(vector<STable> newTable);

private:
    STable table[MAX_CLUBS];
    const Colors *pColors;

    void drawTableHorizontalInner();
    void drawTableHorizontalBottom();
};

#endif /* TABLE_H */
