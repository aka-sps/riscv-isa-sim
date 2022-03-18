// See LICENSE for license details.
#ifndef _RISCV_LOGFILE_H
#define _RISCV_LOGFILE_H

#include <stdio.h>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "decode.h"

// Header-only class wrapping a log file. When constructed with an
// actual path, it opens the named file for writing. When constructed
// with the null path, it wraps stderr.
class log_file_t
{
public:
  log_file_t(const char *path)
    : wrapped_file (nullptr, &fclose)
  {
    if (!path)
      return;

    wrapped_file.reset(fopen(path, "w"));
    if (! wrapped_file) {
      std::ostringstream oss;
      oss << "Failed to open log file at `" << path << "': "
          << strerror (errno);
      throw std::runtime_error(oss.str());
    }
  }

  FILE *get() { return wrapped_file ? wrapped_file.get() : stderr; }
private:
  std::unique_ptr<FILE, decltype(&fclose)> wrapped_file;
};

/*apy-sc*/
class memory_dump_file_t
{
public:
  memory_dump_file_t(const char *path)
    : wrapped_file (nullptr, &fclose)
  {
    if (!path)
      return;

    wrapped_file.reset(fopen(path, "w"));
    if (! wrapped_file) {
      std::ostringstream oss;
      oss << "Failed to open memory dump file at `" << path << "': "
          << strerror (errno);
      throw std::runtime_error(oss.str());
    }
  }
  int add(reg_t start, reg_t end)
  {
    this->start.push_back(start);
    this->end.push_back(end);
  }

  FILE *get() { return wrapped_file ? wrapped_file.get() : stderr; }
private:
  std::unique_ptr<FILE, decltype(&fclose)> wrapped_file;
  std::vector<reg_t> start;
  std::vector<reg_t> end;
};

#endif
