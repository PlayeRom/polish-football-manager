
#ifndef PLAYERCLUB_H
#define PLAYERCLUB_H

#include "Structs.h"

using namespace std;

class PlayerClub {
public:
    PlayerClub();
    PlayerClub(const PlayerClub& orig);
    virtual ~PlayerClub();

    void init(const wchar_t* name, const wchar_t* surname, const wchar_t* nick, int clubId);
    void initNewClub(int clubId);
    void save() const;
    void load();

    SClub& get() { return club; }

    const wstring getClubName(int index) const;

private:
    SClub club;
};

#endif /* PLAYERCLUB_H */

