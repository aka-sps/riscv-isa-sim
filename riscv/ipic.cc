// See LICENSE for license details.

#include "ipic.hxx"
#include "processor.hxx"
#include "sim.hxx"
#include <vector>

namespace riscv_isa_sim {

class ipic_implementation
{
public:

  ipic_implementation(sim_t *s, processor_t *p) : sim(s), proc(p) {}
  virtual ~ipic_implementation() {}

  // update state of ext irq lines
  virtual void update_lines_state(reg_t v) {}
  // check IPIC inerrupt line state
  virtual bool is_irq_active() { return false; }

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
  // ISR (aggregated)
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
};

reg_t ext_ipic::get_mcicsr()
{
  return 0;
}
void  ext_ipic::set_mcicsr(reg_t v)
{
}
reg_t ext_ipic::get_scicsr()
{
  return 0;
}
void  ext_ipic::set_scicsr(reg_t v)
{
}
// MEOI/SEOI
void  ext_ipic::set_meoi(reg_t v)
{
}
void  ext_ipic::set_seoi(reg_t v)
{
}
// SOI
void  ext_ipic::set_soi(reg_t v)
{
}
// CISV
reg_t ext_ipic::get_cisv() // RO
{
  return 0;
}
// ISR (aggregated)
reg_t ext_ipic::get_isvr() // RO
{
  return 0;
}
// IPR (aggregated)
reg_t ext_ipic::get_ipr()
{
  return 0;
}
void  ext_ipic::set_ipr(reg_t v) // W1C
{
}
// IER (aggregated)
reg_t ext_ipic::get_ier()
{
  return 0;
}
void  ext_ipic::set_ier(reg_t v)
{
}
// IMR (aggregated)
reg_t ext_ipic::get_imr()
{
  return 0;
}
void  ext_ipic::set_imr(reg_t v)
{
}
// INVR (aggregated)
reg_t ext_ipic::get_invr()
{
  return 0;
}
void  ext_ipic::set_invr(reg_t v)
{
}
// ISAR (aggregated)
reg_t ext_ipic::get_isar()
{
  return 0;
}
void  ext_ipic::set_isar(reg_t v)
{
}
// rel indexed access to interrupt control/status regs
// IDX
reg_t ext_ipic::get_ridx()
{
  return 0;
}
void  ext_ipic::set_ridx(reg_t v)
{
}
// ICSR
reg_t ext_ipic::get_icsr()
{
  return 0;
}
void  ext_ipic::set_icsr(reg_t v)
{
}
//----------------------------------------------------------
// internal IPIC

class internal_ipic : public ipic_implementation
{
public:

  internal_ipic(sim_t *s, processor_t *p)
    : ipic_implementation(s, p) {}
  ~internal_ipic() {}

  // update state of ext irq lines
  void update_lines_state(reg_t v);
  // check IPIC inerrupt line state
  bool is_irq_active();

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
  void  set_ipr(reg_t); // WC
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
  // rel indexed access to interrupt control/status regs
  // IDX
  reg_t get_ridx();
  void  set_ridx(reg_t);
  // ICSR
  reg_t get_icsr();
  void  set_icsr(reg_t);

private:
  uint32_t isvr;
  uint32_t ipr;
  uint32_t ier;
  uint32_t imr;
  uint32_t invr;
  uint32_t isar;
  uint32_t intmap[IPIC_IRQ_LINES];

  uint32_t ridx;
  uint32_t ext_irq; // current state of irq lines (for detection of edges)
  std::vector<unsigned> in_service; // back = current in-service interrupt
};

// update state of ext irq lines
void internal_ipic::update_lines_state(reg_t v)
{
  uint32_t changes = ext_irq ^ v;

  // for eache changed line check state
  for (unsigned i = 0; i < IPIC_IRQ_LINES; ++i) {
    uint32_t mask = 1 << i;
    uint32_t line_mask = (1 << intmap[i]);
    if (changes & line_mask) {
      int state = v & line_mask;
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
    }
  }
  // update current ext irq line's state
  ext_irq = v;
}
// check IPIC inerrupt line state
bool internal_ipic::is_irq_active()
{
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
    for (; int_num < IPIC_IRQ_LINES; ++int_num) {
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
  ridx = v % IPIC_IRQ_LINES;
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
  intmap[ridx] = (v >> IPIC_ICS_LN_OFFS) & ((1 << IPIC_ICS_LN_BITS) - 1);

  // update pending state,
  ipr &= ~mask;
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

//----------------------------------------------------------
// IPIC

ipic_t::ipic_t(sim_t *s, processor_t *p, emulation_mode mode)
{
  if (mode == emulation_mode::internal)
    impl = new internal_ipic(s, p);
  else
    impl = new ext_ipic(s, p);
}

ipic_t::~ipic_t()
{
  delete impl;
}

// update state of ext irq lines
void ipic_t::update_lines_state(reg_t v)
{
  impl->update_lines_state(v);
}
// check IPIC inerrupt line state
bool ipic_t::is_irq_active()
{
  return impl->is_irq_active();
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
