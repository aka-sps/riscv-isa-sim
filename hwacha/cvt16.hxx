#ifndef _CVT16_H
#define _CVT16_H

#include <cstdint>

namespace riscv_isa_sim {
uint_fast32_t cvt_hs(uint_fast16_t);
uint_fast16_t cvt_sh(uint_fast32_t, int);
}  // namespace riscv_isa_sim

#endif
