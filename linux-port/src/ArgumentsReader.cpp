
#include <iostream>
#include <getopt.h>
#include <stdio.h>
#include "ArgumentsReader.h"

/**
 * Constructor
 *
 * @param argc Number of arguments
 * @param argv Arguments texts
 */
ArgumentsReader::ArgumentsReader(int argc, char** argv)
{
    isExit = false;
    parseArguments(argc, argv);
}

/**
 * Parse arguments given by executing the program
 *
 * @param argc
 * @param argv
 */
void ArgumentsReader::parseArguments(int argc, char** argv)
{
    static struct option long_options[] = {
        {"language", no_argument,       0,  'l' },
        {"help",     no_argument,       0,  'h' },
        {"version",  no_argument,       0,  'v' },
        {0,          0,                 0,  0 }
    };

    int option;
    while ((option = getopt_long(argc, argv, "l:hv", long_options, NULL)) != -1) {
        switch (option) {
            case 'v':
                printVersion();
                isExit = true;
                break;

            case 'l':
                arguments[ARG_LANGUAGE] = optarg;
                break;

            case 'h':
            case '?':
            default:
                printHelp();
                isExit = true;
                break;
        }
    }

    // optind is for the extra arguments which are not parsed
    for (; optind < argc; optind++) {
        arguments[ARG_PATH] = argv[optind];
    }
}

/**
 * Print help to cout
 */
void ArgumentsReader::printHelp()
{
    cout << "Usage: managerligipolskiej2002 [-l <language code>] [-h] [-v]" << endl;
    cout << "    -l <language code>, --language <language code> - the langauge code e.g. pl_PL" << endl;
    cout << "    -h, --help - show this help message and exit" << endl;
    cout << "    -v, --v - show version message and exit" << endl;
}

/**
 * Print version message to cout
 */
void ArgumentsReader::printVersion()
{
    cout << "Manager of the Polish League 2002 version 1.05" << endl;
}

/**
 * Check whether app should be exit
 *
 * @return
 */
bool ArgumentsReader::isExitFlag()
{
    return isExit;
}
