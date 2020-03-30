
#ifndef NEWS_H
#define NEWS_H

#include <vector>
#include <string>
#include "Structs.h"
#include "Language.h"

using namespace std;

class News {
public:
    News(Language *pLang);
    virtual ~News();

    void init();
    const SNews& getManagerNewsByNumber(int number) const;

    bool loadManagerMessages();
    void saveManagerMessages();
    void clearManagerMessages();
    void addDisplayManagerMessages(const SClub &clubRef, bool isCout);

    const wstring& getManagerMessage(int index) const { return managerMessages[index]; }
    const size_t getSizeManagerMessages() const { return managerMessages.size(); }

    void setTmpMsgData(int msgId);
    void setTmpMsgData(int msgId, wstring strForMsg);
    void setTmpMsgData(int msgId, wstring strForMsg, int noForMsg);

    bool isMessage() const { return messages.size() > 0; }

private:
    vector<SNews> allManagerMessages; // all manager messages, loaded once during boot app
    vector<wstring> managerMessages; // manager messages incoming duriong the game
    const SNews *pEmptyNews;
    Language *pLang;

    struct STmpMessage {
        int messageId;
        wstring strForMsg;
        int numberForMsg;

        STmpMessage(): messageId(0), strForMsg(L""), numberForMsg(0) {}
    };

    vector<STmpMessage> messages;

    void addManagerMessage(wstring messageLine) { managerMessages.push_back(messageLine); };
    void addManagerMessage(const SNews &news, const SClub& club, int index);
    void coutManagerMessage(const SNews &news, int index);
    const SNews& getManagerNewsByIndex(int msgIndex) const;
};

#endif /* NEWS_H */
