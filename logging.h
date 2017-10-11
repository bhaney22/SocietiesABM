#ifndef _SOC_LOGGING_
#define _SOC_LOGGING_

#include <iostream>
#include <ctime>
#include "globals.h"


/*
 * TODO: change this so we can go to an ostream that isn't cout.
 */
class Log
{
private:
    int msgLevel;
public:
    Log(const std::string &funcName, int lineNum, int level) :
        msgLevel(level)
    {
        if (msgLevel > glob.verboseLevel) { return; }
        char t[100];
        time_t rawtime;
        struct tm *tim;
        time(&rawtime);
        tim = localtime(&rawtime);
        strftime(t, 100, "%c", tim);
        std::cout << t << ": " << funcName << " [" << lineNum << "] " << "Log(" << level << "): ";
    }

    template <class T>
    Log &operator<<(const T &v)
    {
        if (msgLevel <= glob.verboseLevel) {
            std::cout << v;
        }
        return *this;
    }

    ~Log()
    {
        if (msgLevel > glob.verboseLevel) { return; }
        std::cout << std::endl;
    }
};

// Call this line to generate a log message at level l.
#define LOG(l) Log(__FUNCTION__, __LINE__, l)


#endif
