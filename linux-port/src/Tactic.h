
#ifndef TACTIC_H
#define TACTIC_H

#include <vector>
#include <string>
#include "Colors.h"
#include "Structs.h"
#include "Language.h"

// Tactic IDs
#define T4_4_2      1
#define T4_4_2_DEF  2
#define T4_4_2_ATT  3
#define T4_4_2_DIA  4
#define T3_4_3      5
#define T3_5_2      6
#define T3_5_2_DEF  7
#define T3_5_2_ATT  8
#define T4_2_4      9
#define T4_3_3      10
#define T4_5_1      11
#define T5_3_2      12
#define T5_3_2_DEF  13
#define T5_3_2_ATT  14

using namespace std;

class Tactic {
public:
    Tactic(const Colors *pColors, Language *pLang);

    void drawTeamSetting(int setting, bool isPlayerTeam = true) const;
    void drawChart(int setting, int who, const vector<SFootballer> &footballers, bool isRival = false);

    const std::wstring* getTeamSettings() const {
        static std::wstring settings[15] = {
            L"4-4-2",
            pLang->get(L"4-4-2 Defense"),
            pLang->get(L"4-4-2 Attack"),
            pLang->get(L"4-4-2 Diamond"),
            L"3-4-3",
            L"3-5-2",
            pLang->get(L"3-5-2 Defense"),
            pLang->get(L"3-5-2 Attack"),
            L"4-2-4",
            L"4-3-3",
            L"4-5-1",
            L"5-3-2",
            pLang->get(L"5-3-2 Defense"),
            pLang->get(L"5-3-2 Attack"),
            pLang->get(L"Error")
        };

        return settings;
    };

    const wstring getTeamSetting(int index) {
        return getTeamSettings()[index];
    }

private:
    const Colors* pColors;
    Language *pLang;

    void drawBoxes(int color, const wstring& sign, int strength, int max);
};

#endif /* TACTIC_H */
