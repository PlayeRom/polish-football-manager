
#ifndef ARGUMENTSREADER_H
#define ARGUMENTSREADER_H

#include <map>
#include <string>
#include "Structs.h"

using namespace std;

const string ARG_PATH = "path";
const string ARG_LANGUAGE = "language";

class ArgumentsReader
{
public:
    ArgumentsReader(int argc, char** argv);

    /**
     * Get output file of empty string
     *
     * @return
     */
    const std::string getLanguage() {
        map<string, string>::iterator it = arguments.find(ARG_LANGUAGE);
        if (it == arguments.end()) {
            return "en_US";
        }

        return it->second;
    }

    void printHelp();
    bool isExitFlag();

private:
    map<string, string> arguments;
    void parseArguments(int argc, char** argv);
    void printVersion();

    bool isExit;
};

#endif /* ARGUMENTSREADER_H */
