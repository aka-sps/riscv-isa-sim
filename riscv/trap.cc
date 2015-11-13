// See LICENSE for license details.

#include "trap.hxx"

#include "processor.hxx"
#include <cstdio>

namespace riscv_isa_sim {
const char* trap_t::name()
{
  const char* fmt = uint8_t(which) == which ? "trap #%u" : "interrupt #%u";
  sprintf(_name, fmt, uint8_t(which));
  return _name;
}

void mem_trap_t::side_effects(state_t* state)
{
  state->mbadaddr = badvaddr;
}
}  // namespace riscv_isa_sim
