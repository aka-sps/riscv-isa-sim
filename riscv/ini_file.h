#ifndef INI_FILE_HPP_
#define INI_FILE_HPP_

#include <string>
#include <unordered_map>
#include <iosfwd>


class Ini_file
  : public std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
  {
  public:
  typedef std::unordered_map<std::string, std::string> Ini_file_section;
  typedef std::string file_path_type;

  Ini_file();
  Ini_file(std::istream& _strm);
  Ini_file(file_path_type const& _file_name);
  void Ini_file_load(std::istream& _strm);
  void Ini_file_load_by_fname(file_path_type const& _file_name);

  char *
    get_param(std::string const& _section, std::string const& _parameter)const
  {
    auto const p_section = find(_section);

    if (p_section == cend()) {
      //throw std::runtime_error("Absent section '" + _section + "'!");
      printf("Absent section '%s' !\n", _section.data());
      return NULL;
    }

    auto const p_param = p_section->second.find(_parameter);

    if (p_param == p_section->second.cend()) {
      //throw std::runtime_error("Absent parameter '" + _parameter + "' in section '" + _section + "'!");
      printf("Absent parameter '%s' !\n", _parameter.data());
      return NULL;
    }

    return (char*)p_param->second.data();
  }
};


#endif  // INI_FILE_HPP_
