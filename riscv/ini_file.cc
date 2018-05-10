#include "ini_file.h"

#include <fstream>
#include <iostream>

Ini_file::Ini_file()
{

}

Ini_file::Ini_file(std::istream& _strm)
{
  Ini_file::Ini_file_load(_strm);
}

void Ini_file::Ini_file_load_by_fname(file_path_type const& _file_name)
{
  std::ifstream file(_file_name.c_str());

  if (!file) {
    printf("Can't open file for read: '%s'\n", _file_name.data());
    return;
  }

  Ini_file_load(file);
}

void Ini_file::Ini_file_load(std::istream& _strm)
{
  std::string curr_section_name;
  unsigned line_no = 0;

  while (_strm) {
    std::string concat_line;

    for (;;) {
      std::string line;
      ++line_no;
      std::getline(_strm, line);
      size_t count_bs = 0;

      if (!line.empty() && line.back() == '\r') {
        line.erase(line.end() - 1);
      }

      for (auto p = line.crbegin(); p != line.crend(); ++p) {
        if (*p != '\\') {
          break;
        }

        ++count_bs;
      }

      if (count_bs % 2) {
        concat_line.append(line.cbegin(), line.cend() - 1);
      } else {
        concat_line.append(line);
        break;
      }
    }

    for (auto p = concat_line.begin(); p != concat_line.end(); ++p) {
      if (*p == '#' || *p == ';') {
        concat_line.erase(p, concat_line.end());
        break;
      }
    }

    bool found_section = false;

    for (auto pb = concat_line.cbegin(); pb != concat_line.cend(); ++pb) {
      if (*pb == ' ' || *pb == '\t') {
        continue;
      }

      if (*pb == '[') {
        ++pb;

        for (auto pe = pb; pe != concat_line.cend(); ++pe) {
          if (*pe == ']') {
            curr_section_name.assign(pb, pe);
            found_section = true;
            break;
          }
        }

        break;
      }

      break;
    }

    if (found_section) {
      continue;
    }

    std::string key;
    std::string value;

    for (auto pkb = concat_line.cbegin(); pkb != concat_line.cend(); ++pkb) {
      if (*pkb == ' ' || *pkb == '\t') {
        continue;
      }

      if (*pkb == '=') {
        break;
      }

      for (auto pke = pkb + 1; pke != concat_line.cend(); ++pke) {
        if (!(*pke == '=' || *pke == ' ' || *pke == '\t')) {
          continue;
        }

        for (auto pe = pke; pe != concat_line.cend(); ++pe) {
          if (*pe == ' ' || *pe == '\t') {
            continue;
          }

          if (*pe == '=') {
            key.assign(pkb, pke);

            for (auto pvb = pe + 1; pvb != concat_line.cend(); ++pvb) {
              if (*pvb == ' ' || *pvb == '\t') {
                continue;
              }

              if (*pvb == '#' || *pvb == ';') {
                break;
              }

              auto pve = pvb + 1;

              for (; pve != concat_line.cend(); ++pve) {
                if (*pve == '#' || *pve == ';') {
                  break;
                }
              }

              value.assign(pvb, pve);
              break;
            }

            break;
          }

          break;
        }

        break;
      }

  break;
  }

  if (!key.empty()) {
  (*this)[curr_section_name][key] = value;
  continue;
  }

  bool not_blank = false;

  for (auto p = concat_line.cbegin(); p != concat_line.cend(); ++p) {
  if (*p == ' ' || *p == '\t') {
    continue;
  }

  not_blank = true;
  break;
  }

  if (!not_blank) {
    continue;
  }

  std::cerr << "Warning: unknown ini-line: " << concat_line << std::endl;
  }
}

Ini_file::Ini_file(file_path_type const& _file_name)
{
  std::ifstream file(_file_name.c_str());

  if (!file) {
    printf("Can't open file for read: '%s'\n", _file_name.data());
    return;
  }

  new (this) Ini_file(file);
}

