// See LICENSE for license details.

#include "ipic.hxx"
#include "processor.hxx"
#include "sim.hxx"
#include "spike_client.hxx"
#include <vector>

namespace riscv_isa_sim {

class ipic_implementation
{
public:

  ipic_implementation(sim_t *s, processor_t *p) : sim(s), proc(p) {}
  virtual ~ipic_implementation() {}

  // check IPIC inerrupt line state
  virtual bool is_irq_active() = 0;

  // regsiter access functions

  // MCICSR/SCICSR
  virtual reg_t get_mcicsr() = 0;
  virtual void  set_mcicsr(reg_t) = 0;
  virtual reg_t get_scicsr() = 0;
  virtual void  set_scicsr(reg_t) = 0;
  // MEOI/SEOI
  virtual void  set_meoi(reg_t) = 0;
  virtual void  set_seoi(reg_t) = 0;
  // SOI
  virtual void  set_soi(reg_t) = 0;
  // CISV
  virtual reg_t get_cisv() = 0; // RO
  // ISVR (aggregated)
  virtual reg_t get_isvr() = 0; // RO
  // IPR (aggregated)
  virtual reg_t get_ipr() = 0;
  virtual void  set_ipr(reg_t) = 0; // WC
  // IER (aggregated)
  virtual reg_t get_ier() = 0;
  virtual void  set_ier(reg_t) = 0;
  // IMR (aggregated)
  virtual reg_t get_imr() = 0;
  virtual void  set_imr(reg_t) = 0;
  // INVR (aggregated)
  virtual reg_t get_invr() = 0;
  virtual void  set_invr(reg_t) = 0;
  // ISAR (aggregated)
  virtual reg_t get_isar() = 0;
  virtual void  set_isar(reg_t) = 0;
  // relative indexed access to interrupt control/status regs
  // IDX
  virtual reg_t get_ridx() = 0;
  virtual void  set_ridx(reg_t) = 0;
  // ICSR
  virtual reg_t get_icsr() = 0;
  virtual void  set_icsr(reg_t) = 0;

protected:
  sim_t *sim;
  processor_t *proc;
};

//----------------------------------------------------------
// external IPIC

class ext_ipic : public ipic_implementation
{
public:

  ext_ipic(sim_t *s, processor_t *p) : ipic_implementation(s, p) {}
  ~ext_ipic() {}

  // check IPIC inerrupt line state
  bool is_irq_active() override {
    return spike_vcs_TL::vcs_device_agent::instance().is_irq_active();
  }

  // regsiter access functions

  // MCICSR/SCICSR
  reg_t get_mcicsr()      override;
  void  set_mcicsr(reg_t) override;
  reg_t get_scicsr()      override;
  void  set_scicsr(reg_t) override;
  // MEOI/SEOI
  void  set_meoi(reg_t)   override;
  void  set_seoi(reg_t)   override;
  // SOI
  void  set_soi(reg_t)    override;
  // CISV, RO
  reg_t get_cisv()        override;
  // ISR (aggregated), RO
  reg_t get_isvr()        override;
  // IPR (aggregated), W1C
  reg_t get_ipr()         override;
  void  set_ipr(reg_t)    override;
  // IER (aggregated)
  reg_t get_ier()         override;
  void  set_ier(reg_t)    override;
  // IMR (aggregated)
  reg_t get_imr()         override;
  void  set_imr(reg_t)    override;
  // INVR (aggregated)
  reg_t get_invr()        override;
  void  set_invr(reg_t)   override;
  // ISAR (aggregated)
  reg_t get_isar()        override;
  void  set_isar(reg_t)   override;
  // relative indexed access to interrupt control/status regs
  // IDX
  reg_t get_ridx()        override;
  void  set_ridx(reg_t)   override;
  // ICSR
  reg_t get_icsr()        override;
  void  set_icsr(reg_t)   override;
};

reg_t ext_ipic::get_mcicsr()
{
  uint32_t value;
  spike_vcs_TL::vcs_device_agent::instance().csr_load(IPIC_CSR_MCICSR_OFFS, 3, &value);
  return value;
}
void  ext_ipic::set_mcicsr(reg_t v)
{
  spike_vcs_TL::vcs_device_agent::instance().csr_store(IPIC_CSR_MCICSR_OFFS, 3, v);
}
reg_t ext_ipic::get_scicsr()
{
  uint32_t value;
  spike_vcs_TL::vcs_device_agent::instance().csr_load(IPIC_CSR_SCICSR_OFFS, 1, &value);
  return value;
}
void  ext_ipic::set_scicsr(reg_t v)
{
  spike_vcs_TL::vcs_device_agent::instance().csr_store(IPIC_CSR_SCICSR_OFFS, 1, v);
}
// MEOI/SEOI
void  ext_ipic::set_meoi(reg_t v)
{
  spike_vcs_TL::vcs_device_agent::instance().csr_store(IPIC_CSR_MEOI_OFFS, 3, v);
}
void  ext_ipic::set_seoi(reg_t v)
{
  spike_vcs_TL::vcs_device_agent::instance().csr_store(IPIC_CSR_SEOI_OFFS, 1, v);
}
// SOI
void  ext_ipic::set_soi(reg_t v)
{
  spike_vcs_TL::vcs_device_agent::instance().csr_store(IPIC_CSR_SOI_OFFS, 3, v);
}
// CISV
reg_t ext_ipic::get_cisv() // RO
{
  uint32_t value;
  spike_vcs_TL::vcs_device_agent::instance().csr_load(IPIC_CSR_CISV_OFFS, 3, &value);
  return value;
}
// ISR (aggregated)
reg_t ext_ipic::get_isvr() // RO
{
  uint32_t value;
  spike_vcs_TL::vcs_device_agent::instance().csr_load(IPIC_CSR_ISVR_OFFS, 3, &value);
  return value;
}
// IPR (aggregated)
reg_t ext_ipic::get_ipr()
{
  uint32_t value;
  spike_vcs_TL::vcs_device_agent::instance().csr_load(IPIC_CSR_IPR_OFFS, 3, &value);
  return value;
}
void  ext_ipic::set_ipr(reg_t v) // W1C
{
  spike_vcs_TL::vcs_device_agent::instance().csr_store(IPIC_CSR_IPR_OFFS, 3, v);
}
// IER (aggregated)
reg_t ext_ipic::get_ier()
{
  uint32_t value;
  spike_vcs_TL::vcs_device_agent::instance().csr_load(IPIC_CSR_IER_OFFS, 3, &value);
  return value;
}
void  ext_ipic::set_ier(reg_t v)
{
  spike_vcs_TL::vcs_device_agent::instance().csr_store(IPIC_CSR_IER_OFFS, 3, v);
}
// IMR (aggregated)
reg_t ext_ipic::get_imr()
{
  uint32_t value;
  spike_vcs_TL::vcs_device_agent::instance().csr_load(IPIC_CSR_IMR_OFFS, 3, &value);
  return value;
}
void  ext_ipic::set_imr(reg_t v)
{
  spike_vcs_TL::vcs_device_agent::instance().csr_store(IPIC_CSR_IMR_OFFS, 3, v);
}
// INVR (aggregated)
reg_t ext_ipic::get_invr()
{
  uint32_t value;
  spike_vcs_TL::vcs_device_agent::instance().csr_load(IPIC_CSR_INVR_OFFS, 3, &value);
  return value;
}
void  ext_ipic::set_invr(reg_t v)
{
  spike_vcs_TL::vcs_device_agent::instance().csr_store(IPIC_CSR_INVR_OFFS, 3, v);
}
// ISAR (aggregated)
reg_t ext_ipic::get_isar()
{
  uint32_t value;
  spike_vcs_TL::vcs_device_agent::instance().csr_load(IPIC_CSR_ISAR_OFFS, 3, &value);
  return value;
}
void  ext_ipic::set_isar(reg_t v)
{
  spike_vcs_TL::vcs_device_agent::instance().csr_store(IPIC_CSR_ISAR_OFFS, 3, v);
}
// rel indexed access to interrupt control/status regs
// IDX
reg_t ext_ipic::get_ridx()
{
  uint32_t value;
  spike_vcs_TL::vcs_device_agent::instance().csr_load(IPIC_CSR_OFFS_OFFS, 3, &value);
  return value;
}
void  ext_ipic::set_ridx(reg_t v)
{
  spike_vcs_TL::vcs_device_agent::instance().csr_store(IPIC_CSR_OFFS_OFFS, 3, v);
}
// ICSR
reg_t ext_ipic::get_icsr()
{
  uint32_t value;
  spike_vcs_TL::vcs_device_agent::instance().csr_load(IPIC_CSR_ICSR_OFFS, 3, &value);
  return value;
}
void  ext_ipic::set_icsr(reg_t v)
{
  spike_vcs_TL::vcs_device_agent::instance().csr_store(IPIC_CSR_ICSR_OFFS, 3, v);
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

  internal_ipic(sim_t *s, processor_t *p);
  ~internal_ipic() {}

  // check IPIC inerrupt line state
  bool is_irq_active();

  // regsiter access functions

  // MCICSR/SCICSR
  reg_t get_mcicsr()      override;
  void  set_mcicsr(reg_t) override;
  reg_t get_scicsr()      override;
  void  set_scicsr(reg_t) override;
  // MEOI/SEOI
  void  set_meoi(reg_t)   override;
  void  set_seoi(reg_t)   override;
  // SOI
  void  set_soi(reg_t)    override;
  // CISV, RO
  reg_t get_cisv()        override;
  // ISR (aggregated), RO
  reg_t get_isvr()        override;
  // IPR (aggregated), W1C
  reg_t get_ipr()         override;
  void  set_ipr(reg_t)    override;
  // IER (aggregated)
  reg_t get_ier()         override;
  void  set_ier(reg_t)    override;
  // IMR (aggregated)
  reg_t get_imr()         override;
  void  set_imr(reg_t)    override;
  // INVR (aggregated)
  reg_t get_invr()        override;
  void  set_invr(reg_t)   override;
  // ISAR (aggregated)
  reg_t get_isar()        override;
  void  set_isar(reg_t)   override;
  // relative indexed access to interrupt control/status regs
  // IDX
  reg_t get_ridx()        override;
  void  set_ridx(reg_t)   override;
  // ICSR
  reg_t get_icsr()        override;
  void  set_icsr(reg_t)   override;

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
};

internal_ipic::internal_ipic(sim_t *s, processor_t *p)
  : ipic_implementation(s, p)
{
  // init ext irq lines for vectors
  for (int i = 0; i < IPIC_IRQ_VECTORS; ++i)
    intmap[i] = IPIC_IRQ_LN_VOID;
}

// update state of ext irq lines
void internal_ipic::update_lines_state()
{
  uint32_t v = spike_vcs_TL::vcs_device_agent::instance().irq_state();
  uint32_t changes = ext_irq ^ v;

  // fprintf(stderr, "update lines: cur= %08X, new= %08X, changes= %08X, cur_ipr= %08X\n",
  //         (unsigned)ext_irq, (unsigned)v, (unsigned)changes, (unsigned)ipr);

  // for eache changed line check state
  for (unsigned i = 0; i < IPIC_IRQ_VECTORS; ++i) {
    if (intmap[i] >= IPIC_IRQ_LN_VOID)
      continue;
    uint32_t mask = 1 << i;
    uint32_t line_mask = (1 << intmap[i]);
    if (changes & line_mask) {
      // fprintf(stderr, "update lines: changed: vec= %u, line= %u\n",
      //         i, (unsigned)intmap[i]);
      uint32_t state = v & line_mask;
      if (invr & mask)
        state = !state;
      if (imr & mask) { // edge detection
        if (state)
          ipr |= mask;
      } else { // level detection
        if (state)
          ipr |= mask;
        else
          ipr &= ~mask;
      }
      // fprintf(stderr, "update lines: new ipr= %08X\n", (unsigned)ipr);
    }
  }
  // update current ext irq line's state
  ext_irq = v;
}
// check IPIC inerrupt line state
bool internal_ipic::is_irq_active()
{
  update_lines_state();

  // mask lower prio irqs
  reg_t cisv = get_cisv();
  uint32_t en_mask = ~0;
  if (cisv != IPIC_ISV_NONE)
    en_mask = (1 << cisv) - 1;
  return ((ipr & ier) & en_mask) != 0;
}
reg_t internal_ipic::get_mcicsr()
{
  uint32_t cicsr = 0;
  if (!in_service.empty()) {
    uint32_t idx = in_service.back();
    uint32_t mask = 1 << idx;
    if (ipr & mask)
      cicsr |= IPIC_ICS_IP;
    if (ier & mask)
      cicsr |= IPIC_ICS_IE;
  }
  return cicsr;
}
void  internal_ipic::set_mcicsr(reg_t v)
{
  uint32_t cicsr = 0;
  if (!in_service.empty()) {
    uint32_t idx = in_service.back();
    uint32_t mask = 1 << idx;
    // clear pending state
    if (v & IPIC_ICS_IP)
      ipr &= ~mask;
    // enable/disable interrupt
    if (v & IPIC_ICS_IE)
      ier |= mask;
    else
      ier &= ~mask;
  }
}
reg_t internal_ipic::get_scicsr()
{
  if (!in_service.empty()) {
    uint32_t mask = 1 << in_service.back();
    if (isar & mask)
      return get_mcicsr();
  }
  return 0;
}
void  internal_ipic::set_scicsr(reg_t v)
{
  if (!in_service.empty()) {
    uint32_t mask = 1 << in_service.back();
    if (isar & mask)
      set_mcicsr(v);
  }
}
// MEOI/SEOI
void  internal_ipic::set_meoi(reg_t )
{
  if (!in_service.empty()) {
    uint32_t mask = 1 << in_service.back();
    isvr &= ~mask;
    in_service.pop_back();
  }
}
void  internal_ipic::set_seoi(reg_t v)
{
  if (!in_service.empty()) {
    uint32_t mask = 1 << in_service.back();
    if (isar & mask)
      set_meoi(v);
  }
}
// SOI
void  internal_ipic::set_soi(reg_t v)
{
  // mask lower prio irqs
  reg_t cisv = get_cisv();
  uint32_t en_mask = ~0;
  if (cisv != IPIC_ISV_NONE)
    en_mask = (1 << cisv) - 1;
  uint32_t active_int = (ipr & ier) & en_mask;
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
reg_t internal_ipic::get_cisv() // RO
{
  if (!in_service.empty())
    return in_service.back();
  return IPIC_ISV_NONE;
}
// ISVR (aggregated)
reg_t internal_ipic::get_isvr() // RO
{
  return isvr;
}
// IPR (aggregated)
reg_t internal_ipic::get_ipr()
{
  return ipr;
}
void  internal_ipic::set_ipr(reg_t v) // W1C
{
}
// IER (aggregated)
reg_t internal_ipic::get_ier()
{
  return ier;
}
void  internal_ipic::set_ier(reg_t v)
{
  ier = v;
}
// IMR (aggregated)
reg_t internal_ipic::get_imr()
{
  return imr;
}
void  internal_ipic::set_imr(reg_t v)
{
  imr = v;
}
// INVR (aggregated)
reg_t internal_ipic::get_invr()
{
  return invr;
}
void  internal_ipic::set_invr(reg_t v)
{
  invr = v;
}
// ISAR (aggregated)
reg_t internal_ipic::get_isar()
{
  return isar;
}
void  internal_ipic::set_isar(reg_t v)
{
  isar = v;
}
// rel indexed access to interrupt control/status regs
// IDX
reg_t internal_ipic::get_ridx()
{
  return ridx;
}
void  internal_ipic::set_ridx(reg_t v)
{
  ridx = v % IPIC_IRQ_VECTORS;
}
// ICSR
reg_t internal_ipic::get_icsr()
{
  uint32_t mask = 1 << ridx;
  uint32_t status = intmap[ridx] << IPIC_ICS_LN_OFFS;
  if (ipr & mask)
    status |= IPIC_ICS_IP;
  if (ier & mask)
    status |= IPIC_ICS_IE;
  if (imr & mask)
    status |= IPIC_ICS_IM;
  if (invr & mask)
    status |= IPIC_ICS_INV;
  if (isar & mask)
    status |= IPIC_ICS_SA;
  if (isvr & mask)
    status |= IPIC_ICS_IS;

  return status;
}
void  internal_ipic::set_icsr(reg_t v)
{
  uint32_t mask = 1 << ridx;

  if (v & IPIC_ICS_IE)
    ier |= mask;
  else
    ier &= ~mask;
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
  unsigned line_num = (v >> IPIC_ICS_LN_OFFS) & ((1 << IPIC_ICS_LN_BITS) - 1);
  intmap[ridx] = line_num;

  // update pending state
  ipr &= ~mask;
  if (line_num < IPIC_IRQ_LN_VOID) {
    uint32_t line_mask = 1 << intmap[ridx];
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

  // fprintf(stderr, "set ICSR: ridx= %u, v= %08X, line= %u, ipr= %08X, ext_irq= %08X\n",
  //         ridx, (unsigned)v, intmap[ridx], ipr, ext_irq);
  // fprintf(stderr, "set ICSR: ier= %08X, imr= %08X, invr= %08X, isar= %08X, ipr= %08X\n",
  //         ier, imr, invr, isar, ipr);
}

//----------------------------------------------------------
// IPIC

ipic_t::ipic_t(sim_t *s, processor_t *p, emulation_mode _mode)
  : mode(_mode)
{
  if (mode == emulation_mode::internal) {
    // fprintf(stderr, "ipic: init internal\n");
    impl = new internal_ipic(s, p);
  } else {
    // fprintf(stderr, "ipic: init external\n");
    impl = new ext_ipic(s, p);
  }
}

ipic_t::~ipic_t()
{
  delete impl;
}

// check IPIC inerrupt line state
bool ipic_t::is_irq_active()
{
  bool res = impl->is_irq_active();
  // fprintf(stderr, "ipic: is_irq_active(): %d\n", res);
  return res;
}

reg_t ipic_t::get_mcicsr()
{
  return impl->get_mcicsr();
}
void  ipic_t::set_mcicsr(reg_t v)
{
  impl->set_mcicsr(v);
}
reg_t ipic_t::get_scicsr()
{
  return impl->get_scicsr();
}
void  ipic_t::set_scicsr(reg_t v)
{
  impl->set_scicsr(v);
}
// MEOI/SEOI
void  ipic_t::set_meoi(reg_t v)
{
  impl->set_meoi(v);
}
void  ipic_t::set_seoi(reg_t v)
{
  impl->set_seoi(v);
}
// SOI
void  ipic_t::set_soi(reg_t v)
{
  impl->set_soi(v);
}
// CISV
reg_t ipic_t::get_cisv() // RO
{
  return impl->get_cisv();
}
// aggregated access to fields of
// ISVR (aggregated)
reg_t ipic_t::get_isvr() // RO
{
  return impl->get_isvr();
}
// IPR (aggregated)
reg_t ipic_t::get_ipr()
{
  return impl->get_ipr();
}
void  ipic_t::set_ipr(reg_t v)
{
  impl->set_ipr(v);
}
// IER (aggregated)
reg_t ipic_t::get_ier()
{
  return impl->get_ier();
}
void  ipic_t::set_ier(reg_t v)
{
  impl->set_ier(v);
}
// IMR (aggregated)
reg_t ipic_t::get_imr()
{
  return impl->get_imr();
}
void  ipic_t::set_imr(reg_t v)
{
  impl->set_imr(v);
}
// INVR (aggregated)
reg_t ipic_t::get_invr()
{
  return impl->get_invr();
}
void  ipic_t::set_invr(reg_t v)
{
  impl->set_invr(v);
}
// ISAR (aggregated)
reg_t ipic_t::get_isar()
{
  return impl->get_isar();
}
void  ipic_t::set_isar(reg_t v)
{
  impl->set_isar(v);
}
// relative indexed access to interrupt control/status regs
// IDX
reg_t ipic_t::get_ridx()
{
  return impl->get_ridx();
}
void  ipic_t::set_ridx(reg_t v)
{
  impl->set_ridx(v);
}
// ICSR
reg_t ipic_t::get_icsr()
{
  return impl->get_icsr();
}
void  ipic_t::set_icsr(reg_t v)
{
  impl->set_icsr(v);
}

} // namespace riscv_isa_sim
