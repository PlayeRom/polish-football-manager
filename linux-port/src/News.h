
#ifndef NEWS_H
#define NEWS_H

#include <vector>
#include <string>
#include "Structs.h"

#define MAX_MESSAGES 10

using namespace std;

class News {
public:
    News();
    virtual ~News();

    void init();
    const SNews& getManagerNewsByNumber(int number) const;

    bool loadManagerMessages();
    void saveManagerMessages();
    void clearManagerMessages();
    void addManagerMessage(wstring messageLine) { managerMessages.push_back(messageLine); };
    void addManagerMessage(const SNews &news, const SClub& club, const wstring& message, int number = 0);

    const wstring& getManagerMessage(int index) const { return managerMessages[index]; }
    const size_t getSizeManagerMessages() const { return managerMessages.size(); }

    /**
     *  wiadomosc
     * 0 - czy w ogole masz wiadomość (1 - tak, 0 - nie)
     * 1-9 - przechowuje ID wiadomosci
     * Maksymalnie mozesz otrzymac 9 wiadomosci jednoczesnie
     */
    int messages[MAX_MESSAGES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    wstring stringForMessage[4]; // string for message
    int numbersForMessage[MAX_MESSAGES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

private:
    vector<SNews> allManagerMessages; // all manager messages, loaded once during boot app
    vector<wstring> managerMessages; // manager messages incoming duriong the game
    const SNews *pEmptyNews;
};

#endif /* NEWS_H */

