
#ifndef SQUAD_H
#define SQUAD_H

#include <vector>
#include "Colors.h"
#include "Structs.h"
#include "Language.h"

using namespace std;

class Squad {
public:
    Squad(const Colors *pColors, Language *pLang);

    void draw(const vector<SFootballer> &footballers, int setting, int mode, int clubNumber, bool isTrenning = false, int sort = 0);

    const wstring getMorale(int value);
    const wstring getFootballerPosition(int pos);

private:
    const Colors *pColors;
    Language *pLang;

    void sortFootballers(int sort, vector<SFootballer> &footballers);
    int getTextColor(int setting, int mode, int settingCount, bool isTrenning, int sort, int color);
    void drawSpecialEvents(const SFootballer &footballer, int color);
    const wstring getTrainingName(int watTrain, int &color);
};

#endif /* SQUAD_H */
