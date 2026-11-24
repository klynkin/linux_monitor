#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include "linux_parser.h"
#include <algorithm>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}


// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 

  std::string file_mem = kProcDirectory + kMeminfoFilename;

  std::optional<float> MemTotalString = ReadByKey<float>("MemTotal", file_mem);
  std::optional<float> MemTotalAvailable = ReadByKey<float>("MemAvailable", file_mem);

  if (MemTotalString && MemTotalAvailable) {
 
    return (MemTotalString.value() - MemTotalAvailable.value())/MemTotalString.value(); 
  }

  return 0;
}

// Read and return the system uptime
long LinuxParser::UpTime() { 

  std::string file_uptime = kProcDirectory + kUptimeFilename;

  std::optional<float> uptime = ReadByKey<float>({}, file_uptime);

  if (uptime) {
    return uptime.value();
  }

  return 0;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 

  long result = 0;
  std::string file_jiffies = kProcDirectory + kStatFilename;

  std::optional<vector<int>> jiffies = ReadByKey<vector<int>>({"cpu"}, file_jiffies);

  if (jiffies) {
    for (auto sum : jiffies.value()) {
      result += sum;
    }
  }

  return result; 
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) { 

  std::string uptime;
  std::string stime;
  std::string line;
  std::string tmp;
  std::string file_jiffies = kProcDirectory + to_string(pid) + kStatFilename;

  std::ifstream filestream(file_jiffies);
  if (filestream) {
    while (std::getline(filestream, line)) {
      auto first = line.begin() + line.find('(');
      auto last  = line.begin() + line.rfind(')');
      line.erase(std::remove(first, last, ' '), last);

      std::istringstream linestream(line);

      for (int i = 0; i < 13; i ++) {
        linestream >> tmp;
      }

      linestream >> uptime >> stime;
    }

  }

  return atoi(uptime.c_str()) + atoi(stime.c_str()); 
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  long result = 0;
  std::string file_jiffies = kProcDirectory + kStatFilename;

  std::optional<vector<int>> jiffies = ReadByKey<vector<int>>({"cpu"}, file_jiffies);

  if (jiffies) {
    const auto& v = jiffies.value();
    if (v.size() > kSteal_) {
      for (auto state : {kUser_, kNice_, kSystem_, kIRQ_, kSoftIRQ_, kSteal_}) {
        result += v[state];
      }
    }
  }

  return result;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  long result = 0;
  std::string file_jiffies = kProcDirectory + kStatFilename;

  std::optional<vector<int>> jiffies = ReadByKey<vector<int>>({"cpu"}, file_jiffies);

  if (jiffies) {
    const auto& v = jiffies.value();
    if (v.size() > kIOwait_) {
      for (auto state : {kIdle_, kIOwait_}) {
        result += v[state];
      }
    }
  }

  return result;
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {   
  
  std::string file_jiffies = kProcDirectory + kStatFilename;
  std::optional<vector<string>> jiffies =
      ReadByKey<vector<string>>({"cpu"}, file_jiffies);

  if (jiffies) {
    return jiffies.value();
  }

  return {};
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  
  std::string file_jiffies = kProcDirectory + kStatFilename;
  std::optional<int> processes = ReadByKey<int>({"processes"}, file_jiffies);

  if (processes) {
    return processes.value();
  }

  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  
  std::string file_jiffies = kProcDirectory + kStatFilename;
  std::optional<int> processes = ReadByKey<int>({"procs_running"}, file_jiffies);

  if (processes) {
    return processes.value();
  }

  return 0;

}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::string line;
  std::string file_cmdline = kProcDirectory + to_string(pid) + kCmdlineFilename;

  std::optional<std::string> cmdline = ReadByKey<std::string>({}, file_cmdline);

  if (cmdline) {
    return cmdline.value();
  }

  return {};
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) { 

  std::string line;
  std::string file_mem = kProcDirectory + to_string(pid) + kStatusFilename;

  std::optional<long> meminfo = ReadByKey<long>({"VmRSS"}, file_mem);

  if (meminfo) {


    return std::to_string(meminfo.value()/1024);
  }

  return {};

 }

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  std::string file_status = kProcDirectory + to_string(pid) + kStatusFilename;

  // "Uid:  1000  1000  1000  1000" -> the first value is the real UID
  std::optional<std::string> uid = ReadByKey<std::string>({"Uid"}, file_status);

  if (uid) {
    return uid.value();
  }

  return {};
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  std::string uid = Uid(pid);
  if (uid.empty()) {
    return {};
  }

  std::string line;
  std::ifstream filestream(kPasswordPath);
  if (filestream) {
    while (std::getline(filestream, line)) {
      std::string name, password, line_uid;
      std::istringstream linestream(line);
      std::getline(linestream, name, ':');
      std::getline(linestream, password, ':');
      std::getline(linestream, line_uid, ':');
      if (line_uid == uid) {
        return name;
      }
    }
  }

  return {};
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  std::string line;
  std::string file_stat = kProcDirectory + to_string(pid) + kStatFilename;

  std::ifstream filestream(file_stat);
  if (filestream && std::getline(filestream, line)) {
    // Skip past the process name: everything after the last ')' is plain
    // space-separated values, starting with field 3 (state).
    auto name_end = line.rfind(')');
    if (name_end == std::string::npos) {
      return 0;
    }
    std::istringstream linestream(line.substr(name_end + 1));

    // starttime is field 22, so discard fields 3..21 (19 tokens) first
    std::string tmp;
    for (int i = 0; i < 19; i++) {
      linestream >> tmp;
    }

    long starttime = 0;  // in clock ticks since boot
    if (linestream >> starttime) {
      return UpTime() - starttime / sysconf(_SC_CLK_TCK);
    }
  }

  return 0;
}
