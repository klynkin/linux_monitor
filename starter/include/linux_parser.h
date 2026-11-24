#ifndef SYSTEM_PARSER_H
#define SYSTEM_PARSER_H

#include <fstream>
#include <regex>
#include <string>
#include <optional>

namespace LinuxParser {
// Paths
const std::string kProcDirectory{"/proc/"};
const std::string kCmdlineFilename{"/cmdline"};
const std::string kCpuinfoFilename{"/cpuinfo"};
const std::string kStatusFilename{"/status"};
const std::string kStatFilename{"/stat"};
const std::string kUptimeFilename{"/uptime"};
const std::string kMeminfoFilename{"/meminfo"};
const std::string kVersionFilename{"/version"};
const std::string kOSPath{"/etc/os-release"};
const std::string kPasswordPath{"/etc/passwd"};

// System
float MemoryUtilization();
long UpTime();
std::vector<int> Pids();
int TotalProcesses();
int RunningProcesses();
std::string OperatingSystem();
std::string Kernel();

// generic: one token
template <typename T>
void Extract(std::istringstream& s, T& out) { s >> out; }

// vector: every remaining token on the line
template <typename U>
void Extract(std::istringstream& s, std::vector<U>& out) {
  U item;
  while (s >> item) out.push_back(item);
}

template <typename T>
std::optional<T> ReadByKey(const std::string& key, const std::string& file_name) {

  std::string tmp_key;
  T tmp_value;

  std::string line;
  std::ifstream file_mem (file_name);

  bool no_key = key.empty();

  if (file_mem) {
    while (std::getline(file_mem, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream line_stream(line);
      if (line_stream) {

        if (no_key) {
          Extract(line_stream, tmp_value);
          return tmp_value;  
        }

        line_stream >> tmp_key;
        if (tmp_key == key) {
          Extract(line_stream, tmp_value);
          return tmp_value;
        }
      }
    }
  }    
  
  return std::nullopt;

}

// CPU
enum CPUStates {
  kUser_ = 0,
  kNice_,
  kSystem_,
  kIdle_,
  kIOwait_,
  kIRQ_,
  kSoftIRQ_,
  kSteal_,
  kGuest_,
  kGuestNice_
};
std::vector<std::string> CpuUtilization();
long Jiffies();
long ActiveJiffies();
long ActiveJiffies(int pid);
long IdleJiffies();

// Processes
std::string Command(int pid);
std::string Ram(int pid);
std::string Uid(int pid);
std::string User(int pid);
long int UpTime(int pid);
};  // namespace LinuxParser

#endif