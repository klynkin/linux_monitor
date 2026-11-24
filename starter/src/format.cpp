#include <string>

#include "format.h"
#include <iomanip>
#include <sstream>

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) { 

    int hours, minutes, secs;

    hours = seconds/3600;
    minutes = (seconds - 3600*hours)/60;
    secs = (seconds - 3600*hours) - minutes*60;
    std::ostringstream out;
    out << std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2) << minutes <<
    ":" << std::setw(2) << secs;

    return out.str();
}