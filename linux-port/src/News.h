
#ifndef NEWS_H
#define NEWS_H

#include <vector>
#include <string>
#include "Structs.h"

using namespace std;

class News {
public:
    News();
    News(const News& orig);
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

private:
    vector<SNews> allManagerMessages; // all manager messages, loaded once during boot app
    vector<wstring> managerMessages; // manager messages incoming duriong the game
    const SNews *pEmptyNews;
};

#endif /* NEWS_H */

