
#ifndef FOOTBALLERS_H
#define FOOTBALLERS_H

#include <vector>
#include <stddef.h>
#include "Structs.h"
#include "Random.h"

class Footballers {
public:
    Footballers(const Random *pRand);

    size_t getSizePlayerTeam() const { return playerTeam.size(); }
    size_t getSizeRivals() const { return rivals.size(); }
    size_t getSizeTransfers() const { return transfers.size(); }

    SFootballer& getPlayerTeam(int index) { return playerTeam[index]; }
    SFootballer& getRival(int index) { return rivals[index]; }
    SFootballer& getTransfer(int index) { return transfers[index]; }

    void pushBackPlayerTeam(const SFootballer &footballer) { playerTeam.push_back(footballer); }
    void pushBackRivalsTeam(const SFootballer &footballer) { rivals.push_back(footballer); }
    void pushBackTransfersTeam(const SFootballer &footballer) { transfers.push_back(footballer); }

    std::vector<SFootballer>& getPlayersTeam() { return playerTeam; }
    std::vector<SFootballer>& getRivals() { return rivals; }

    void initialPlayerTeam(int clubId);
    void initialRivals();
    void initialTransfers();
    void loadPlayerTeam();
    void loadRivals();
    void loadTransfers();
    void savePlayerTeam();
    void saveRivals();
    void saveTransfers();

    void changeTransferList();
    void moraleUp(SFootballer &footballer, int modifierValue);
    void moraleDown(SFootballer &footballer, int modifierValue);
    bool isNotAllowedTeamPlayer();

    void sortPlayersTeam();

private:
    const Random *pRand;

    std::vector<SFootballer> playerTeam; // zawodnicy w klubie którym zarzadza gracz, plik save/footballers_manager.cfg
    std::vector<SFootballer> rivals; // zawodnicy rywala, plik save/footballers_rival.cfg
    std::vector<SFootballer> transfers; // zawdnicy na liscie transferowej, plik save/footballers_transfer.cfg
};

#endif /* FOOTBALLERS_H */
