
#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <vector>
#include "Structs.h"
#include "Colors.h"
#include "Rounds.h"
#include "PlayerClub.h"
#include "Language.h"

class Table {
public:
    Table(const Colors *pColors, Language *pLang);

    void createTable();
    void save();
    void load();
    int getPositionInTable(int clubNumber);
    int getClubNumberInPosition(int tablePosition);
    void drawTable(int clubId, const PlayerClub *club);
    void updateAfterMatch(const SRound &round, int roundIndex, SClub &clubRef);

    STable& get(int index) { return table[index]; }
    int getSize() const { return MAX_CLUBS; }
    void overrideTable(vector<STable> newTable);

private:
    STable table[MAX_CLUBS];
    const Colors *pColors;
    Language *pLang;

    void drawTableHorizontalInner();
    void drawTableHorizontalBottom();
};

#endif /* TABLE_H */
