// See LICENSE for license details.
#ifndef _RISCV_REGDUMP_H
#define _RISCV_REGDUMP_H

#define REGDUMP_DEBUG 0

#include <stdio.h>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "decode.h"

/*class to dump registers (scalar, float, vector).
  to use need
  1) path for dump file in cmd --reg_dump-path
  2) string with sections to dump --re_dump<xfv>

  What srcs affected:

  1) path and sections string provided in spike.cc by parser
  2) reg_dump_t transfer to sim_t (sim.h and sim.cc) and after to processor_t (processor.h and processor.cc)
  3) in sim_t::main() if (debug && ctrlc_pressed) will dump into files

  Notes:
  1) for every core will be standalone file reg_dump-path<coreNumber>
  2) If there is no file to dump, but there are parametrs - data will be dumped to stdout

*/
class reg_dump_t
{
public:
  reg_dump_t(const char *path)  : wrapped_file (nullptr, &fclose)
  {
   if (!path)
   return;

    this->main_path=path;

    wrapped_file.reset(fopen(path, "w"));
    //if can not open - throw an exception
    if (! wrapped_file)
    {
      std::ostringstream oss;
      oss << "Failed to open reg dump file at `" << path << "': "
          << strerror (errno);
      throw std::runtime_error(oss.str());
    }
    //if possible to create a file, close it and delete
    else
    {
        wrapped_file.reset(nullptr);
        std::remove(path);
    }
  }
  void add_core_reg_dump()
  {
   if ( (this->main_path.size()==0) ||

        ((!is_need_to_print_vector()) &&
         (!is_need_to_print_scalar()) &&
         (!is_need_to_print_float()))
      )
      {
       return;
      }
    std::string a=std::to_string(files.size());
    std::string core_path=main_path+a;
    wrapped_file.reset(fopen(core_path.c_str(), "w"));
    if (!wrapped_file)
    {
      std::ostringstream oss;
      oss << "Failed to open reg dump file at `" << core_path.c_str() << "': "
          << strerror (errno);
      throw std::runtime_error(oss.str());
    }
    else
    {
    files.emplace_back(std::move(wrapped_file));
    }
  }

  void add_core_rtl_log()
  {
   if (this->main_path.size()==0)
      {
       return;
      }
    std::string a=std::to_string(files.size());
    std::string core_path=main_path+"_rtl_"+a;
    wrapped_file.reset(fopen(core_path.c_str(), "w"));
    if (!wrapped_file)
    {
      std::ostringstream oss;
      oss << "Failed to open reg dump file at `" << core_path.c_str() << "': "
          << strerror (errno);
      throw std::runtime_error(oss.str());
    }
    else
    {
     log_syntacore.emplace_back(std::move(wrapped_file));
    }
  }



  void parse_reg_dump_string (const char *s)
  {
    if (s==nullptr)
    {
      return;
    }
      const std::string str(s);
      std::size_t found = str.find("x");
      if (found!=std::string::npos)
      {
        need_to_print_scalar =true;
      }
      found = str.find("f");
      if (found!=std::string::npos)
      {
        need_to_print_float  =true;
      }
      found = str.find("v");
      if (found!=std::string::npos)
      {
        need_to_print_vector = true;
      }
  }
  FILE *get() { return wrapped_file ? wrapped_file.get() : stdout; }
  FILE *get(long unsigned int core_number)
  {
   if ((files.size()!=0)&&(files.size()>=core_number))
   {
      return files[core_number].get();
   }
   return stdout;
  }
  FILE *get_rtl_log(long unsigned int core_number)
  {
   if ((log_syntacore.size()!=0)&&(log_syntacore.size()>=core_number))
   {
      return log_syntacore[core_number].get();
   }
   return stdout;
  }
   bool is_need_to_print_scalar()
   {
       return need_to_print_scalar;
   }
   bool is_need_to_print_float()
   {
       return need_to_print_float;
   }
   bool is_need_to_print_vector()
   {
       return need_to_print_vector;
   }

private:
  std::unique_ptr<FILE, decltype(&fclose)> wrapped_file;
  bool need_to_print_scalar =false;
  bool need_to_print_float  =false;
  bool need_to_print_vector =false;
  std::string main_path;
  std::vector<std::unique_ptr<FILE, decltype(&fclose)>> files;
  std::vector<std::unique_ptr<FILE, decltype(&fclose)>> log_syntacore;
};

#endif
