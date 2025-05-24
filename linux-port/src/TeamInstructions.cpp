
#include <iostream>
#include "TeamInstructions.h"
#include "ConioColors.h"
#include "Structs.h"

using namespace std;

TeamInstructions::TeamInstructions(const Colors *pColors, Language *pLang)
{
    this->pColors = pColors;
    this->pLang = pLang;
}

/**
 * Rysuj opcje z instrukcjami dla drużyny
 *
 * @param passes
 * @param rivalThreat
 * @param pressing
 * @param offsides
 * @param contra
 * @param attitude
 * @param bar
 */
void TeamInstructions::draw(
    int passes,
    int rivalThreat,
    int pressing,
    int offsides,
    int contra,
    int attitude,
    int bar
) const {
    vector< pair<wstring, int> > data[MAX_TEAM_INSTRUCTIONS] = {
        {
            {pLang->get(L"Attitude"), attitude},
            {pLang->get(L"Normal"), INSTR_ATTIT_NORMAL},
            {pLang->get(L"Defence"), INSTR_ATTIT_DEFENSIVE},
            {pLang->get(L"Attack"), INSTR_ATTIT_ATTACK}
        },
        {
            {pLang->get(L"Passing the ball"), passes},
            {pLang->get(L"Mixed"), INSTR_PASSES_MIXES},
            {pLang->get(L"Short"), INSTR_PASSES_SHORT},
            {pLang->get(L"Medium"), INSTR_PASSES_MIDDLE},
            {pLang->get(L"Long"), INSTR_PASSES_LONG}
        },
        {
            {pLang->get(L"Treatment of rival"), rivalThreat},
            {pLang->get(L"Normal"), INSTR_TREATMENT_NORMAL},
            {pLang->get(L"Soft"), INSTR_TREATMENT_SOFT},
            {pLang->get(L"Hard"), INSTR_TREATMENT_HARD}
        },
        {
            {pLang->get(L"Pressing"), pressing},
            {pLang->get(L"No"), INSTR_NO},
            {pLang->get(L"Yes"), INSTR_YES},
        },
        {
            {pLang->get(L"Offside traps"), offsides},
            {pLang->get(L"No"), INSTR_NO},
            {pLang->get(L"Yes"), INSTR_YES},
        },
        {
            {pLang->get(L"Counter game"), contra},
            {pLang->get(L"No"), INSTR_NO},
            {pLang->get(L"Yes"), INSTR_YES},
        }
    };

    for (int i = 0; i < MAX_TEAM_INSTRUCTIONS; i++) {
        printInstruction(data[i], bar, i + 1);
    }

    pColors->textBackground(BLACK);
}

void TeamInstructions::setColorBarInstruction(int bar, int barValue) const
{
    if (bar == barValue) {
        pColors->textBackground(BLUE);
        pColors->textColor(LIGHTGRAY);
        return;
    }

    pColors->textBackground(BLACK);
    pColors->textColor(GREEN);
}

void TeamInstructions::setColorForInstruction(int parameter, int paramValue, int bar, int barValue) const
{
    if (bar == barValue) { // nad parametrem ustawiona jest belka
        pColors->textColor(parameter == paramValue ? LIGHTGRAY : BLACK);
        return;
    }

    pColors->textColor(parameter == paramValue ? GREEN : DARKGRAY);
}

void TeamInstructions::printInstruction(vector< pair<wstring, int> > values, int bar, int barValue) const
{
    for (int i = 0; i < values.size(); i++) {
        if (i == 0) {
            setColorBarInstruction(bar, barValue);
            wprintf(L"\n\r %-18ls -", values[i].first.c_str());
            continue;
        }

        setColorForInstruction(values[0].second, values[i].second, bar, barValue);
        wcout << L" " << values[i].first;
    }
    wcout << L" ";
}
