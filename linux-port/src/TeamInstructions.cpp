
#include <iostream>
#include "TeamInstructions.h"
#include "ConioColors.h"
#include "Structs.h"

using namespace std;

TeamInstructions::TeamInstructions(const Colors *pColors)
{
    this->pColors = pColors;
}

TeamInstructions::TeamInstructions(const TeamInstructions& orig)
{
}

TeamInstructions::~TeamInstructions()
{
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
            {L"Nastawienie", attitude},
            {L"Normalne", INSTR_ATTIT_NORMAL},
            {L"Obronne", INSTR_ATTIT_DEFENSIVE},
            {L"Atak", INSTR_ATTIT_ATTACK}
        },
        {
            {L"Podania", passes},
            {L"Mieszane", INSTR_PASSES_MIXES},
            {L"Krótkie", INSTR_PASSES_SHORT},
            {L"Średnie", INSTR_PASSES_MIDDLE},
            {L"Długie", INSTR_PASSES_LONG}
        },
        {
            {L"Traktowanie rywala", rivalThreat},
            {L"Normalne", INSTR_TREATMENT_NORMAL},
            {L"Delikatne", INSTR_TREATMENT_SOFT},
            {L"Twarde", INSTR_TREATMENT_HARD}
        },
        {
            {L"Pressing", pressing},
            {L"Nie", INSTR_NO},
            {L"Tak", INSTR_YES},
        },
        {
            {L"Pułapki ofsajdowe", offsides},
            {L"Nie", INSTR_NO},
            {L"Tak", INSTR_YES},
        },
        {
            {L"Gra z kontry", contra},
            {L"Nie", INSTR_NO},
            {L"Tak", INSTR_YES},
        }
    };

    for (int i = 0; i < MAX_TEAM_INSTRUCTIONS; i++) {
        printInstruction(data[i], bar, i + 1);
    }

    pColors->textbackground(BLACK);
}

void TeamInstructions::setColorBarInstruction(int bar, int barValue) const
{
    if (bar == barValue) {
        pColors->textbackground(BLUE);
        pColors->textcolor(LIGHTGRAY);
        return;
    }

    pColors->textbackground(BLACK);
    pColors->textcolor(GREEN);
}

void TeamInstructions::setColorForInstruction(int parameter, int paramValue, int bar, int barValue) const
{
    if (bar == barValue) { // nad parametrem ustawiona jest belka
        pColors->textcolor(parameter == paramValue ? LIGHTGRAY : BLACK);
        return;
    }

    pColors->textcolor(parameter == paramValue ? GREEN : DARKGRAY);
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
