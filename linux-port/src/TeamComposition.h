
#ifndef TEAMCOMPOSITION_H
#define TEAMCOMPOSITION_H

#include <vector>
#include "Colors.h"
#include "Structs.h"

class TeamComposition {
public:
    TeamComposition(const Colors *pColors);

    void draw(const std::vector<SFootballer> &footballers, int setting, int mode, int clubNumber, bool isTrenning = false, int sort = 0);

    std::wstring getMorale(int value);
    wchar_t getFootballerPosition(int pos);

private:
    const Colors *pColors;

    void sortFootballers(int sort, std::vector<SFootballer> &footballers);
    int getTextColor(int setting, int mode, int settingCount, bool isTrenning, int sort, int color);
    void drawSpecialEvents(const SFootballer &footballer, int color);
    std::wstring getTrenningName(int watTrain, int &color);
};

#endif /* TEAMCOMPOSITION_H */
