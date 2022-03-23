// See LICENSE for license details.
#ifndef _RISCV_MEMORYDUMPFILE_H
#define _RISCV_MEMORYDUMPFILE_H

#include <stdio.h>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "decode.h"

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
  void add_start_end(reg_t start, reg_t end)
  {
    this->start.push_back(start);
    this->end.push_back(end);
  }

  void add_start_len(reg_t start, reg_t len)
  {
    this->start.push_back(start);
    this->end.push_back(start+len);
  }

  FILE *get() { return wrapped_file ? wrapped_file.get() : stderr; }
private:
  std::unique_ptr<FILE, decltype(&fclose)> wrapped_file;
  std::vector<reg_t> start;
  std::vector<reg_t> end;
};

#endif
