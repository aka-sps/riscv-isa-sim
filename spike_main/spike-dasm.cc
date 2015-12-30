// See LICENSE for license details.

// This little program finds occurrences of strings like
//  DASM(ffabc013)
// in its input, then replaces them with the disassembly
// enclosed hexadecimal number, interpreted as a RISC-V
// instruction.

#include "extension.hxx"
#include "../../riscv-fesvr/fesvr/option_parser.hxx"
#include "../riscv/disasm.hxx"

#include <iostream>
#include <string>
#include <cstdint>

using namespace std;

int main(int argc, char** argv)
{
  using namespace riscv_isa_sim;
  string s;
  disassembler_t d;

  std::function<extension_t*()> extension;
  using namespace riscv_fesvr;
  option_parser_t parser;
  parser.option(0, "extension", 1, [&](const char* s){extension = find_extension(s);});
  parser.parse(argv);

  if (extension) {
    for (auto disasm_insn : extension()->get_disasms())
      d.add_insn(disasm_insn);
  }

  while (getline(cin, s))
  {
    for (size_t start = 0; (start = s.find("DASM(", start)) != string::npos; )
    {
      size_t end = s.find(')', start);
      if (end == string::npos)
        break;

      char* endp;
      size_t numstart = start + strlen("DASM(");
      int64_t bits = strtoull(&s[numstart], &endp, 16);
      size_t nbits = 4 * (endp - &s[numstart]);
      if (nbits < 64)
        bits = bits << (64 - nbits) >> (64 - nbits);

      string dis = d.disassemble(bits);
      s = s.substr(0, start) + dis + s.substr(end+1);
      start += dis.length();
    }

    cout << s << '\n';
  }

  return 0;
}
