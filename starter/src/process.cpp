#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// The CPU utilization is calculated once here and stored, so that sorting
// compares plain numbers instead of re-reading /proc for every comparison.
Process::Process (const int& pid) : pid_ {pid} {
    // Seconds this process has existed
    long uptime = LinuxParser::UpTime(pid_);
    if (uptime > 0) {
        // Seconds of CPU time it has used (jiffies -> seconds)
        float cpu_time = static_cast<float>(LinuxParser::ActiveJiffies(pid_)) /
                         sysconf(_SC_CLK_TCK);

        cpu_utilization_ = cpu_time / uptime;
    }
}

int Process::Pid() {
    return pid_;
}

// Return this process's CPU utilization
float Process::CpuUtilization() const{
    return cpu_utilization_;
}

// Return the command that generated this process
string Process::Command() { 
    return LinuxParser::Command(pid_);
}

// Return this process's memory utilization
string Process::Ram() { 
    return LinuxParser::Ram(pid_);
}

// Return the user (name) that generated this process
string Process::User() { 
    return LinuxParser::User(pid_);
}

// Return the age of this process (in seconds)
long int Process::UpTime() {
    return LinuxParser::UpTime(pid_);
}

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const { 
    // Reversed on purpose: "less" means "uses more CPU", so std::sort puts
    // the busiest processes first.
    return cpu_utilization_ > a.cpu_utilization_;
}