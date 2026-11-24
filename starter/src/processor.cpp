#include "processor.h"
#include "linux_parser.h"

// Return the aggregate CPU utilization
float Processor::Utilization() const{
  long active = LinuxParser::ActiveJiffies();
  long total = active + LinuxParser::IdleJiffies();

  if (total <= 0) {
    return 0.0;
  }

  return static_cast<float>(active) / total;
}