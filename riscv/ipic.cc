// See LICENSE for license details.

#include "ipic.hxx"
#include "processor.hxx"
#include "spike_client.hxx"
#include <vector>

namespace riscv_isa_sim {

class ipic_implementation
{
public:

  ipic_implementation(processor_t *p) : proc(p) {}
  virtual ~ipic_implementation() {}

  // check IPIC inerrupt line state
  virtual bool is_irq_active() = 0;

  // regsiter access functions
  virtual reg_t get_csr(reg_t idx, reg_t priv) = 0;
  virtual void  set_csr(reg_t idx, reg_t priv, reg_t val) = 0;

protected:
  processor_t *proc;
};

//----------------------------------------------------------
// external IPIC

class ext_ipic : public ipic_implementation
{
public:

  ext_ipic(processor_t *p) : ipic_implementation(p) {}
  ~ext_ipic() {}

  // check IPIC inerrupt line state
  bool is_irq_active() override {
    auto &vcs_agent = spike_vcs_TL::vcs_device_agent::instance();
    vcs_agent.end_of_clock();
    return vcs_agent.is_irq_active();
  }

  // regsiter access functions
  reg_t get_csr(reg_t idx, reg_t priv) override;
  void  set_csr(reg_t idx, reg_t priv, reg_t val) override;
};

reg_t ext_ipic::get_csr(reg_t idx, reg_t priv)
{
  uint32_t val;
  spike_vcs_TL::vcs_device_agent::instance().csr_load(idx, priv, &val);
  fprintf(stderr, "Get CSR: idx= %u, p= %u, v= %08X\n",
          (unsigned)idx, (unsigned)priv, (unsigned)val);
  return val;
}

void  ext_ipic::set_csr(reg_t idx, reg_t priv, reg_t val)
{
  fprintf(stderr, "Set CSR: idx= %u, p= %u, v= %08X\n",
          (unsigned)idx, (unsigned)priv, (unsigned)val);
  spike_vcs_TL::vcs_device_agent::instance().csr_store(idx, priv, val);
}

//----------------------------------------------------------
// internal IPIC

class internal_ipic : public ipic_implementation
{
public:

  enum int_ipic_constants {
    IPIC_IRQ_LINES   = 32, // number of supported ext lines
    IPIC_IRQ_LN_VOID = IPIC_IRQ_LINES, // 'not-connected' value for number of ext line
    IPIC_IRQ_VECTORS = 32, // number of implemented vectors
    IPIC_ISV_NONE    = 32, // 'no in service-mode interrupts' value
  };

  internal_ipic(processor_t *p);
  ~internal_ipic() {}

  // check IPIC inerrupt line state
  bool is_irq_active();

  // regsiter access functions
  reg_t get_csr(reg_t idx, reg_t priv) override;
  void  set_csr(reg_t idx, reg_t priv, reg_t val) override;

private:
  uint32_t isvr;
  uint32_t ipr;
  uint32_t ier;
  uint32_t imr;
  uint32_t invr;
  uint32_t isar;
  uint32_t intmap[IPIC_IRQ_VECTORS];

  uint32_t ridx;
  uint32_t ext_irq; // current state of irq lines (for detection of edges)
  std::vector<unsigned> in_service; // back = current in-service interrupt

  // update state of ext irq lines
  void update_lines_state();
  // get/set IPIC CSR by index
  reg_t get_csr_by_idx(reg_t priv, reg_t idx);
  void  set_csr_by_idx(reg_t priv, reg_t idx, reg_t val);

  // CICSR
  reg_t get_cicsr(reg_t priv);
  void  set_cicsr(reg_t priv, reg_t val);
  // EOI
  void  eoi(reg_t priv);
  // SOI
  void  soi(reg_t priv);
  // CISV, RO
  reg_t get_cisv(reg_t priv);
  // ISR (aggregated), RO
  reg_t get_isvr(reg_t priv);
  // IPR (aggregated), W1C
  reg_t get_ipr(reg_t priv);
  void  set_ipr(reg_t priv, reg_t val);
  // relative indexed access to interrupt control/status regs
  // IDX
  reg_t get_ridx(reg_t priv);
  void  set_ridx(reg_t priv, reg_t val);
  // ICSR
  reg_t get_icsr(reg_t priv);
  void  set_icsr(reg_t priv, reg_t val);
};

internal_ipic::internal_ipic(processor_t *p)
  : ipic_implementation(p)
{
  // init ext irq lines for vectors
  for (int i = 0; i < IPIC_IRQ_VECTORS; ++i)
    intmap[i] = IPIC_IRQ_LN_VOID;
}

reg_t internal_ipic::get_csr(reg_t idx, reg_t priv)
{
  switch (idx) {
  case IPIC_CSR_CICSR:
    return get_cicsr(priv);
  case IPIC_CSR_CISV:
    return get_cisv(priv);
  case IPIC_CSR_ISVR:
    return get_isvr(priv);
  case IPIC_CSR_IPR:
    return get_ipr(priv);
  case IPIC_CSR_IDX:
    return get_ridx(priv);
  case IPIC_CSR_ICSR:
    return get_icsr(priv);
  default:
    return 0;
  }
}

void  internal_ipic::set_csr(reg_t idx, reg_t priv, reg_t val)
{
  switch (idx) {
  case IPIC_CSR_CICSR:
    set_cicsr(priv, val);
    break;
  case IPIC_CSR_IPR:
    break;
  case IPIC_CSR_EOI:
    eoi(priv);
    break;
  case IPIC_CSR_SOI:
    soi(priv);
    break;
  case IPIC_CSR_IDX:
    set_ridx(priv, val);
    break;
  case IPIC_CSR_ICSR:
    set_icsr(priv, val);
    break;
  default:
    break;
  }
}

  // get/set IPIC CSR by index
reg_t internal_ipic::get_csr_by_idx(reg_t priv, reg_t idx)
{
  reg_t cicsr = 0;
  idx %= IPIC_IRQ_VECTORS;
  uint32_t mask = 1 << idx;
  if (priv == 3) { // M-mode
    cicsr |= intmap[idx] << IPIC_ICS_LN_OFFS;
    if (ipr & mask)
      cicsr |= IPIC_ICS_IP;
    if (ier & mask)
      cicsr |= IPIC_ICS_IE;
    if (imr & mask)
      cicsr |= IPIC_ICS_IM;
    if (invr & mask)
      cicsr |= IPIC_ICS_INV;
    if (isar & mask)
      cicsr |= IPIC_ICS_SA;
    if (isvr & mask)
      cicsr |= IPIC_ICS_IS;
  } else if (priv == 1 && (isar & mask)) { // SV-mode, access enabled
    if (ipr & mask)
      cicsr |= IPIC_ICS_IP;
    if (ier & mask)
      cicsr |= IPIC_ICS_IE;
  }
  return cicsr;
}
void  internal_ipic::set_csr_by_idx(reg_t priv, reg_t idx, reg_t v)
{
  idx %= IPIC_IRQ_VECTORS;

  uint32_t mask = 1 << idx;

  if (priv != 3 && !(priv == 1 && (isar & mask)))
    return;

  unsigned line_num = (v >> IPIC_ICS_LN_OFFS) & ((1 << IPIC_ICS_LN_BITS) - 1);

  if (priv == 3) {
    if (v & IPIC_ICS_IM)
      imr |= mask;
    else
      imr &= ~mask;
    if (v & IPIC_ICS_INV)
      invr |= mask;
    else
      invr &= ~mask;
    if (v & IPIC_ICS_SA)
      isar |= mask;
    else
      isar &= ~mask;
    intmap[idx] = line_num;
  }

  if (v & IPIC_ICS_IE)
    ier |= mask;
  else
    ier &= ~mask;

  // update pending state
  ipr &= ~mask;
  if (!(v & IPIC_ICS_IP) && line_num < IPIC_IRQ_LN_VOID) {
    uint32_t line_mask = 1 << intmap[idx];
    if (imr & mask) {
      // edge detection
    } else {
      // level detection
      if (invr & mask) {
        if ((ext_irq & line_mask) == 0)
          ipr |= mask;
      } else {
        if (ext_irq & line_mask)
          ipr |= mask;
      }
    }
  }

  fprintf(stderr, "set ICSR: idx= %u, v= %08X, line= %u, ipr= %08X, ext_irq= %08X\n",
          (unsigned)idx, (unsigned)v, intmap[idx], ipr, (unsigned)ext_irq);
  fprintf(stderr, "set ICSR: ier= %08X, imr= %08X, invr= %08X, isar= %08X, ipr= %08X\n",
          ier, imr, invr, isar, ipr);
}

// update state of ext irq lines
void internal_ipic::update_lines_state()
{
  uint32_t v = spike_vcs_TL::vcs_device_agent::instance().irq_state();
  uint32_t changes = ext_irq ^ v;

  fprintf(stderr, "update lines: cur= %08X, new= %08X, changes= %08X, cur_ipr= %08X\n",
          (unsigned)ext_irq, (unsigned)v, (unsigned)changes, (unsigned)ipr);

  // for eache changed line check state
  for (unsigned i = 0; i < IPIC_IRQ_VECTORS; ++i) {
    if (intmap[i] >= IPIC_IRQ_LN_VOID)
      continue;
    uint32_t mask = 1 << i;
    uint32_t line_mask = (1 << intmap[i]);
    if ((imr & mask) && (changes & line_mask)) { // edge detection
      uint32_t state = v & line_mask;
      if (invr & mask)
        state = !state;
      if (state) {
        ipr |= mask;
        fprintf(stderr, "update lines: changed: vec= %u, line= %u, new ipr= %08X\n",
                i, (unsigned)intmap[i], (unsigned)ipr);
      }
    } else if ((imr & mask) == 0) { // level sensitive
      auto prev_ipr = ipr;
      uint32_t state = v & line_mask;
      if (invr & mask)
        state = !state;
      if (state)
        ipr |= mask;
      else
        ipr &= ~mask;
      if (prev_ipr != ipr)
        fprintf(stderr, "update lines: changed: vec= %u, line= %u, new ipr= %08X\n",
                i, (unsigned)intmap[i], (unsigned)ipr);
    }
  }
  // update current ext irq line's state
  ext_irq = v;
  fprintf(stderr, "update lines: new ipr= %08X, ier= %08X\n", (unsigned)ipr, (unsigned)ier);
}
// check IPIC inerrupt line state
bool internal_ipic::is_irq_active()
{
  update_lines_state();

  // mask lower prio irqs
  reg_t cisv = get_cisv(3);
  uint32_t en_mask = ~0;
  if (cisv != IPIC_ISV_NONE)
    en_mask = (1 << cisv) - 1;
  fprintf(stderr, "is_irq_active: cisv= %u, en_mask= %08X, ipr= %08X, ier= %08X\n",
          (unsigned)cisv, (unsigned)en_mask, (unsigned)ipr, (unsigned)ier);
  return (ipr & ier & en_mask) != 0;
}
reg_t internal_ipic::get_cicsr(reg_t priv)
{
  if (!in_service.empty()) {
    return get_csr_by_idx(priv, in_service.back());
  }
  return 0;
}
void  internal_ipic::set_cicsr(reg_t priv, reg_t v)
{
  if (!in_service.empty()) {
    set_csr_by_idx(priv, in_service.back(), v);
  }
}
// EOI
void  internal_ipic::eoi(reg_t priv)
{
  if (!in_service.empty()) {
    uint32_t mask = 1 << in_service.back();
    if (priv == 3 || (priv == 1 && (isar & mask))) {
      isvr &= ~mask;
      in_service.pop_back();
    }
  }
}
// SOI
void  internal_ipic::soi(reg_t priv)
{
  // mask irqs with lower prio
  reg_t cisv = get_cisv(3);
  uint32_t en_mask = ~0;
  if (cisv != IPIC_ISV_NONE)
    en_mask = (1 << cisv) - 1;
  if (priv == 1) // mask irqs by access mode
    en_mask &= isar;
  uint32_t active_int = ipr & ier & en_mask;
  if (active_int) {
    // get irq with highest prio
    unsigned int_num = 0;
    for (; int_num < IPIC_IRQ_VECTORS; ++int_num) {
      if (active_int & (1 << int_num))
        break;
    }
    in_service.push_back(int_num);
    uint32_t mask = 1 << int_num;
    ipr &= ~mask;
    isvr |= mask;
  }
}
// CISV
reg_t internal_ipic::get_cisv(reg_t priv) // RO
{
  if (!in_service.empty()) {
    auto idx = in_service.back();
    uint32_t mask = 1 << idx;
    if (priv == 3 || (priv == 1 && (isar & mask)))
      return idx;
  }
  return IPIC_ISV_NONE;
}
// ISVR (aggregated)
reg_t internal_ipic::get_isvr(reg_t priv) // RO
{
  if (priv == 3)
    return isvr;
  else if (priv == 1)
    return isvr & isar;
  return 0;
}
// IPR (aggregated)
reg_t internal_ipic::get_ipr(reg_t priv)
{
  if (priv == 3)
    return ipr;
  else if (priv == 1)
    return ipr & isar;
  return 0;
}
void  internal_ipic::set_ipr(reg_t priv, reg_t v) // W1C
{
  // clear pending bits
  if (priv == 3)
    ipr &= ~v;
  else if (priv == 1)
    ipr &= ~(v & isar);
}
// rel indexed access to interrupt control/status regs
// IDX
reg_t internal_ipic::get_ridx(reg_t priv)
{
  return ridx;
}
void  internal_ipic::set_ridx(reg_t priv, reg_t v)
{
  ridx = v % IPIC_IRQ_VECTORS;
}
// ICSR
reg_t internal_ipic::get_icsr(reg_t priv)
{
  return get_csr_by_idx(priv, ridx);
}
void  internal_ipic::set_icsr(reg_t priv, reg_t v)
{
  set_csr_by_idx(priv, ridx, v);
}

//----------------------------------------------------------
// IPIC

ipic_t::ipic_t(processor_t *p, emulation_mode _mode)
  : mode(_mode), proc(p)
{
  if (mode == emulation_mode::internal) {
    // fprintf(stderr, "ipic: init internal\n");
    impl = new internal_ipic(p);
  } else if (mode == emulation_mode::external) {
    // fprintf(stderr, "ipic: init external\n");
    impl = new ext_ipic(p);
  } else {
    impl = nullptr;
  }
}

ipic_t::~ipic_t()
{
  if (impl)
    delete impl;
}

void ipic_t::update_proc_irq_state()
{
  if (impl) {
    if (impl->is_irq_active()) {
      proc->get_state().mip |= MIP_MXIP;
      fprintf(stderr, "set mip: %08X\n", (unsigned)proc->get_state().mip);
    } else {
      proc->get_state().mip &= ~MIP_MXIP;
      fprintf(stderr, "clr mip: %08X\n", (unsigned)proc->get_state().mip);
    }
  }
}

reg_t ipic_t::get_csr(reg_t idx, reg_t samode)
{
  if (impl) {
    reg_t priv = get_field(proc->get_state().mstatus, MSTATUS_PRV);
    if (priv > samode)
      priv = samode;
    return impl->get_csr(idx, priv);
  }
  return 0;
}

void  ipic_t::set_csr(reg_t idx, reg_t samode, reg_t val)
{
  if (impl) {
    reg_t priv = get_field(proc->get_state().mstatus, MSTATUS_PRV);
    if (priv > samode)
      priv = samode;
    impl->set_csr(idx, samode, val);
  }
}

} // namespace riscv_isa_sim
