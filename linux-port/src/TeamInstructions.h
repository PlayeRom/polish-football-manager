
#ifndef TEAMINSTRUCTIONS_H
#define TEAMINSTRUCTIONS_H

#include <vector>
#include <string>
#include "Colors.h"

class TeamInstructions {
public:
    TeamInstructions(const Colors *pColors);
    TeamInstructions(const TeamInstructions& orig);
    virtual ~TeamInstructions();

    void draw(int passes, int rivalThreat, int pressing, int offsides, int contra, int attitude, int bar) const;

private:
    const Colors *pColors;

    void setColorBarInstruction(int bar, int barValue) const;
    void setColorForInstruction(int parameter, int paramValue, int bar, int barValue) const;
    void printInstruction(std::vector< std::pair<std::wstring, int> > values, int bar, int barValue) const;
};

#endif /* TEAMINSTRUCTIONS_H */

