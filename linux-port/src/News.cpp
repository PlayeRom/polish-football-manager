
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "News.h"

News::News()
{
    pEmptyNews = new SNews();

    init();
}

News::~News()
{
    delete pEmptyNews;
}

void News::init()
{
    allManagerMessages.clear();

    FILE *f = fopen(FILE_MSG_MANAGER, "rb");
    if (!f) {
        char message[512];
        snprintf(message, 512, ERROR_OPEN_FILE, FILE_MSG_MANAGER);
        throw std::invalid_argument(message);
    }

    SNews news;
    while (fread(&news, sizeof(SNews), 1, f) == 1) {
        allManagerMessages.push_back(news);
    }
    fclose(f);
}

const SNews& News::getManagerNewsByNumber(int number) const
{
    for (size_t i = 0; i < allManagerMessages.size(); ++i) {
        if (allManagerMessages[i].num == number) {
            return allManagerMessages[i];
        }
    }

    return *pEmptyNews;
}

bool News::loadManagerMessages()
{
    FILE *f = fopen(FILE_SAVE_MANAGER_NEWS, "r");
    if (!f) {
        return false;
    }

    wchar_t buffer[MAX_NEWS_LENGTH];
    while (fgetws(buffer, MAX_NEWS_LENGTH, f) != NULL) {
        wstring line = buffer;
        if (line.size() > 1) {
            line[line.size() - 1] = L'\0'; // remove \n on end of line
            managerMessages.push_back(line);
        }
    }

    fclose(f);
}

void News::saveManagerMessages()
{
    FILE *f = fopen(FILE_SAVE_MANAGER_NEWS, "w");
    for (size_t i = 0; i < managerMessages.size(); ++i) {
        fwprintf(f, managerMessages[i].c_str());
        fwprintf(f, L"\n");
    }
    fclose(f);
}

void News::clearManagerMessages()
{
    managerMessages.clear();

    wofstream fileOutput(FILE_SAVE_MANAGER_NEWS, ios::out | ios::trunc);
    fileOutput.close();
}

void News::addManagerMessage(const SNews &news, const SClub& club, const wstring& message, int number /*= 0*/)
{
    wchar_t tmpBuffer[MAX_NEWS_LENGTH] = {0};
    wstring line;

    swprintf(tmpBuffer, MAX_NEWS_LENGTH, L"%02d.%02d.%d - ", club.day, club.month, club.year);
    line = tmpBuffer;

    swprintf(tmpBuffer, MAX_NEWS_LENGTH, news.message, message.c_str(), number);
    line += tmpBuffer;

    managerMessages.push_back(line);
}
