// See LICENSE for license details.
#ifndef _RISCV_MEMORYDUMP_H
#define _RISCV_MEMORYDUMP_H

#define MEMORYDUMP_DEBUG 0

#include <stdio.h>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "decode.h"

/*class to define specific sections in memory to dump.
  to use need
  1) path for dump file in cmd --memory_dump-path
  2) string with sections to dump --memory_dump-sections=<a:m,b:n,...>
  2.1) start adress with 0x beginning, end adress with 0x beginning
  2.2) start adress with 0x beginning, lenght of section  with +0x beginning
  Number of sections not limited.

  What srcs affected:

  1) path and sections string provided in spike.cc by parser
  2) memory_dump_t transfer to sim_t (sim.h and sim.cc) and after to processor_t (processor.h and processor.cc)
  3) static void commit_memory_dump(processor_t * p) in (execute.cc) to dump a memory throuht mmu_t from processor_t.
  4) dump will be in void processor_t::step(size_t n) if sc_exit found out

  Notes:
  1) There is no memory check for dump for existence, so if you will dump a memory which does not exist will be load/store exception
  2) If there is no file to dump, but have sections parametrs - data will be dumped to stderr
  3) Function provide_memory_adresses_to_dump() provide a dump only once due flag dump_once, because of there is more than 1 core, every core will try dump memory.
  3) Function provide_memory_adresses_to_dump() provide a dump only once due flag dump_once, because of there is more than 1 core, every core will try dump memory.
  4) define MEMORYDUMP_DEBUG to 1 to print what actual adresses were dumped

  TODO: Implement a symbol parse from elf (now only adresses in hex)
*/
class memory_dump_t
{
public:
  memory_dump_t(const char *path) : wrapped_file (nullptr, &fclose)
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
    //if 0 -> need to dump, if non zero - no need to dump
    //possible to have dump each file to different cores
    dump_once=0;
  }

  void parse_memory_dump_string (const char *s)
  {
    if (s==nullptr)
    {
      return;
    }
    const std::string str(s);
    std::istringstream stream(str);
    reg_t cnt=0;
    while (true)
    {
      std::string section;
      std::getline(stream, section, ',');
      if (section.empty())
      {
       break;
      }
      cnt++;
      std::size_t found = section.find(":");
      std::string start = "";
      std::string end = "";
      //a= absolute adress
      //l= lenght of block, possible only for second paramert
      //s= symbol, will be found in .elf later
      std::string type_of_adresses = "";
  if ((found!=std::string::npos) && (found!=section.length()-1) && (found!=0))
      {
       start=section.substr(0,found);
       end=section.substr(found+1,section.length());
      }
      else
      {
              throw std::runtime_error("Inspect memory dump sections (delimenter, start, end symbol/adress");
      }
      std::size_t position_of_0x = start.find("0x");
      std::size_t position_of_plus0x = start.find("+0x");
      if (position_of_0x==0)
      {
        type_of_adresses="a";
      }
      else
      {
        type_of_adresses="s";
      }
      position_of_0x = end.find("0x");
      position_of_plus0x = end.find("+0x");
      if (position_of_0x==0)
      {
        type_of_adresses+="a";
      }
      else
      {
        if (position_of_plus0x==0)
        {
          type_of_adresses+="l";
          end=end.substr(1,end.length());

        }
        else
        {
          type_of_adresses+="s";
        }
      }
      memory_dump_vector.emplace_back(start,end,type_of_adresses);
    }
  }

  void set_memory_dump_vector(std::vector<std::tuple<std::string, std::string,std::string >> *some_memory_dump_vector)
  {
      for (reg_t i=0; i<memory_dump_vector.size(); i++)
      {
         this->memory_dump_vector.emplace_back(std::get<0>(memory_dump_vector[i]),std::get<1>(memory_dump_vector[i]),std::get<2>(memory_dump_vector[i]));
      }
  }
  void print_memory_dump_vector()
  {
      printf("number of memory regions to dump is %lu\n", memory_dump_vector.size());
      for (reg_t i=0; i<memory_dump_vector.size(); i++)
      {
          printf("start %s, end %s, type %s\n", std::get<0>(memory_dump_vector[i]).c_str(), std::get<1>(memory_dump_vector[i]).c_str(), std::get<2>(memory_dump_vector[i]).c_str());
      }
  }

  std::vector<std::tuple<reg_t,reg_t >> provide_memory_adresses_to_dump()
  {
    if (dump_once==0)
    {
      dump_once++;
      for (reg_t i=0; i<memory_dump_vector.size(); i++)
      {
        if ((std::get<2>(memory_dump_vector[i])=="aa") || (std::get<2>(memory_dump_vector[i])=="al"))
        {
          reg_t addr_start = strtoull(std::get<0>(memory_dump_vector[i]).c_str(), 0, 0);
          reg_t addr_end = strtoull(std::get<1>(memory_dump_vector[i]).c_str(), 0, 0);

          if (std::get<2>(memory_dump_vector[i])=="al")
          {
            addr_end+=addr_start;
          }
          this->adresses_to_dump.emplace_back(addr_start,addr_end);
        }
      }
    }
    return this->adresses_to_dump;
  }

  FILE *get() { return wrapped_file ? wrapped_file.get() : stderr; }

private:
  std::vector<std::tuple<std::string, std::string,std::string >> memory_dump_vector;
  std::vector<std::tuple<reg_t,reg_t >> adresses_to_dump;
  std::unique_ptr<FILE, decltype(&fclose)> wrapped_file;
  reg_t dump_once;
};

#endif
