// See LICENSE for license details.

#ifndef _RISCV_SC_IPIC_H
#define _RISCV_SC_IPIC_H

#include "common.h"
#include "config.h"
// #include <cstdlib>
#include <cstdint>
// #include <vector>
#include "decode.hxx"
// #include <type_traits>

namespace riscv_isa_sim {

class sim_t;
class processor_t;

enum ipic_constants {
  IPIC_CSR_M_BASE = 0x790,
  IPIC_CSR_MCICSR = IPIC_CSR_M_BASE,
  IPIC_CSR_MEOI   = IPIC_CSR_M_BASE + 1,
  IPIC_CSR_SOI    = IPIC_CSR_M_BASE + 2,
  IPIC_CSR_CISV   = IPIC_CSR_M_BASE + 3,
  IPIC_CSR_ISVR   = IPIC_CSR_M_BASE + 4,
  IPIC_CSR_IPR    = IPIC_CSR_M_BASE + 5,
  IPIC_CSR_IER    = IPIC_CSR_M_BASE + 6,
  IPIC_CSR_IMR    = IPIC_CSR_M_BASE + 7,
  IPIC_CSR_INVR   = IPIC_CSR_M_BASE + 8,
  IPIC_CSR_ISAR   = IPIC_CSR_M_BASE + 9,
  IPIC_CSR_IDX    = IPIC_CSR_M_BASE + 10,
  IPIC_CSR_ICSR   = IPIC_CSR_M_BASE + 11,
  IPIC_CSR_S_BASE = 0x590,
  IPIC_CSR_SCICSR = IPIC_CSR_S_BASE,
  IPIC_CSR_SEOI   = IPIC_CSR_S_BASE + 1,

  IPIC_CSR_GET_IRQ_STATE = 0x7c0,
  IPIC_CSR_SET_IRQ_STATE = 0x7c1,

  IPIC_IRQ_LINES = 32,
  IPIC_ISV_NONE  = 32, // 'no in service-mode interrupts' value
};

enum ipic_ics_bits {
  // these bits are accessible in M-mode and S-mode
  IPIC_ICS_IP  = (1 << 0), // pending
  IPIC_ICS_IE  = (1 << 1), // enable
  IPIC_ICS_IM  = (1 << 2), // mode
  IPIC_ICS_INV = (1 << 3), // line inversion
  // follow bits are accessible in M-mode only
  IPIC_ICS_SA  = (1 << 8), // supervisor access
  IPIC_ICS_IS  = (1 << 9), // in-service
  // external IRQ line number
  IPIC_ICS_LN_OFFS = 12, // LN offset
  IPIC_ICS_LN_BITS = 5, // LN bits
  IPIC_ICS_LN_MASK = (((1 << IPIC_ICS_LN_BITS) - 1) << IPIC_ICS_LN_OFFS), // LN mask
};

// this class implements a hart's IPIC
class ipic_implementation;

class ipic_t
{
public:
  enum emulation_mode {
    internal = 0,
    external,
  };

  ipic_t(sim_t *sim, processor_t *proc, emulation_mode mode = internal);
  ~ipic_t();

  // update state of ext irq lines
  void update_lines_state(reg_t v);
  // check IPIC inerrupt line state
  bool is_irq_active();

  emulation_mode get_mode() const {
    return mode;
  }

  // regsiter access functions

  // MCICSR/SCICSR
  reg_t get_mcicsr();
  void  set_mcicsr(reg_t);
  reg_t get_scicsr();
  void  set_scicsr(reg_t);
  // MEOI/SEOI
  void  set_meoi(reg_t);
  void  set_seoi(reg_t);
  // SOI
  void  set_soi(reg_t);
  // CISV
  reg_t get_cisv(); // RO
  // ISVR (aggregated)
  reg_t get_isvr(); // RO
  // IPR (aggregated)
  reg_t get_ipr();
  void  set_ipr(reg_t); // W1C
  // IER (aggregated)
  reg_t get_ier();
  void  set_ier(reg_t);
  // IMR (aggregated)
  reg_t get_imr();
  void  set_imr(reg_t);
  // INVR (aggregated)
  reg_t get_invr();
  void  set_invr(reg_t);
  // ISAR (aggregated)
  reg_t get_isar();
  void  set_isar(reg_t);
  // relative indexed access to interrupt control/status regs
  // IDX
  reg_t get_ridx();
  void  set_ridx(reg_t);
  // ICSR
  reg_t get_icsr();
  void  set_icsr(reg_t);

private:
  emulation_mode mode;
  ipic_implementation *impl;
};
}  // namespace riscv_isa_sim

#endif // _RISCV_SC_IPIC_H
