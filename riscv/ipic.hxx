// See LICENSE for license details.

#ifndef _RISCV_SC_IPIC_H
#define _RISCV_SC_IPIC_H

#include "common.h"
#include "config.h"
#include <cstdint>
#include "decode.hxx"

namespace riscv_isa_sim {

class processor_t;

enum ipic_constants {
  IPIC_CSR_CISV   = 0,
  IPIC_CSR_CICSR  = 1,
  IPIC_CSR_IPR    = 2,
  IPIC_CSR_ISVR   = 3,
  IPIC_CSR_EOI    = 4,
  IPIC_CSR_SOI    = 5,
  IPIC_CSR_IDX    = 6,
  IPIC_CSR_ICSR   = 7,
  // IPIC_CSR_IER_OFFS    = 6,
  // IPIC_CSR_IMR_OFFS    = 7,
  // IPIC_CSR_INVR_OFFS   = 8,
  // IPIC_CSR_ISAR_OFFS   = 9,
  // IPIC_CSR_IDX_OFFS    = 10,
  // IPIC_CSR_ICSR_OFFS   = 11,

  // IPIC_CSR_SCICSR_OFFS = 0,
  // IPIC_CSR_SEOI_OFFS   = 1,

  IPIC_CSR_M_BASE = 0x790,
  IPIC_CSR_MCICSR = IPIC_CSR_M_BASE + IPIC_CSR_CICSR,
  IPIC_CSR_MEOI   = IPIC_CSR_M_BASE + IPIC_CSR_EOI,
  IPIC_CSR_MSOI   = IPIC_CSR_M_BASE + IPIC_CSR_SOI,
  IPIC_CSR_MCISV  = IPIC_CSR_M_BASE + IPIC_CSR_CISV,
  IPIC_CSR_MISVR  = IPIC_CSR_M_BASE + IPIC_CSR_ISVR,
  IPIC_CSR_MIPR   = IPIC_CSR_M_BASE + IPIC_CSR_IPR,
  IPIC_CSR_MIDX   = IPIC_CSR_M_BASE + IPIC_CSR_IDX,
  IPIC_CSR_MICSR  = IPIC_CSR_M_BASE + IPIC_CSR_ICSR,
  IPIC_CSR_S_BASE = 0x590,
  IPIC_CSR_SCICSR = IPIC_CSR_S_BASE + IPIC_CSR_CICSR,
  IPIC_CSR_SEOI   = IPIC_CSR_S_BASE + IPIC_CSR_EOI,
  IPIC_CSR_SSOI   = IPIC_CSR_S_BASE + IPIC_CSR_SOI,
  IPIC_CSR_SCISV  = IPIC_CSR_S_BASE + IPIC_CSR_CISV,
  IPIC_CSR_SISVR  = IPIC_CSR_S_BASE + IPIC_CSR_ISVR,
  IPIC_CSR_SIPR   = IPIC_CSR_S_BASE + IPIC_CSR_IPR,
  IPIC_CSR_SIDX   = IPIC_CSR_S_BASE + IPIC_CSR_IDX,
  IPIC_CSR_SICSR  = IPIC_CSR_S_BASE + IPIC_CSR_ICSR,

  IPIC_CSR_GET_IRQ_STATE = 0x79e,
  IPIC_CSR_SET_IRQ_STATE = 0x79f,
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
  IPIC_ICS_LN_BITS = 6, // LN bits
  IPIC_ICS_LN_MASK = (((1 << IPIC_ICS_LN_BITS) - 1) << IPIC_ICS_LN_OFFS), // LN mask
};

// this class implements a hart's IPIC
class ipic_implementation;

class ipic_t
{
public:
  enum emulation_mode {
    none = 0,
    internal,
    external,
  };

  ipic_t(processor_t *proc, emulation_mode mode = internal);
  ~ipic_t();

  // update ext int flags
  void update_proc_irq_state();
  // get current IPIC mode
  emulation_mode get_mode() const {
    return mode;
  }

  // regsiter access functions
  reg_t get_csr(reg_t idx, reg_t samode);
  void  set_csr(reg_t idx, reg_t samode, reg_t val);

private:
  emulation_mode mode;
  ipic_implementation *impl;
  processor_t *proc;
};
}  // namespace riscv_isa_sim

#endif // _RISCV_SC_IPIC_H
