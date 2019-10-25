
#ifndef LOGGER_H
#define LOGGER_H

#include <string>

using namespace std;

class Logger {
public:
    void addLog(const wchar_t* fmt, ...);
};

#endif /* LOGGER_H */
