
#ifndef TEAMINSTRUCTIONS_H
#define TEAMINSTRUCTIONS_H

#include <vector>
#include <string>
#include "Colors.h"
#include "Language.h"

class TeamInstructions {
public:
    TeamInstructions(const Colors *pColors, Language *pLang);

    void draw(int passes, int rivalThreat, int pressing, int offsides, int contra, int attitude, int bar) const;

private:
    const Colors *pColors;
    Language *pLang;

    void setColorBarInstruction(int bar, int barValue) const;
    void setColorForInstruction(int parameter, int paramValue, int bar, int barValue) const;
    void printInstruction(std::vector< std::pair<std::wstring, int> > values, int bar, int barValue) const;
};

#endif /* TEAMINSTRUCTIONS_H */

