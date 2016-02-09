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
  virtual reg_t get_cisw() = 0; // RO
  // aggregated access to fields of
  // ISR (aggregated)
  virtual reg_t get_isvr() = 0; // RO
  // IPR (aggregated)
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
  reg_t get_cisw(); // RO
  // aggregated access to fields of
  // ISR (aggregated)
  reg_t get_isvr(); // RO
  // IPR (aggregated)
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
reg_t ext_ipic::get_cisw() // RO
{
  return 0;
}
// aggregated access to fields of
// ISR (aggregated)
reg_t ext_ipic::get_isvr() // RO
{
  return 0;
}
// IPR (aggregated)
void  ext_ipic::set_ipr(reg_t v) // WC
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

  internal_ipic(sim_t *s, processor_t *p) : ipic_implementation(s, p) {}
  ~internal_ipic() {}

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
  reg_t get_cisw(); // RO
  // aggregated access to fields of
  // ISR (aggregated)
  reg_t get_isvr(); // RO
  // IPR (aggregated)
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
  uint32_t status[IPIC_IRQ_LINES];
  uint32_t cicsr;
  uint32_t ridx;
  std::vector<unsigned> in_service;
};

reg_t internal_ipic::get_mcicsr()
{
  return 0;
}
void  internal_ipic::set_mcicsr(reg_t v)
{
}
reg_t internal_ipic::get_scicsr()
{
  return 0;
}
void  internal_ipic::set_scicsr(reg_t v)
{
}
// MEOI/SEOI
void  internal_ipic::set_meoi(reg_t v)
{
}
void  internal_ipic::set_seoi(reg_t v)
{
}
// SOI
void  internal_ipic::set_soi(reg_t v)
{
}
// CISV
reg_t internal_ipic::get_cisw() // RO
{
  return 0;
}
// aggregated access to fields of
// ISR (aggregated)
reg_t internal_ipic::get_isvr() // RO
{
  return 0;
}
// IPR (aggregated)
void  internal_ipic::set_ipr(reg_t v) // WC
{
}
// IER (aggregated)
reg_t internal_ipic::get_ier()
{
  return 0;
}
void  internal_ipic::set_ier(reg_t v)
{
}
// IMR (aggregated)
reg_t internal_ipic::get_imr()
{
  return 0;
}
void  internal_ipic::set_imr(reg_t v)
{
}
// INVR (aggregated)
reg_t internal_ipic::get_invr()
{
  return 0;
}
void  internal_ipic::set_invr(reg_t v)
{
}
// ISAR (aggregated)
reg_t internal_ipic::get_isar()
{
  return 0;
}
void  internal_ipic::set_isar(reg_t v)
{
}
// rel indexed access to interrupt control/status regs
// IDX
reg_t internal_ipic::get_ridx()
{
  return ridx;
}
void  internal_ipic::set_ridx(reg_t v)
{
  ridx = v;
}
// ICSR
reg_t internal_ipic::get_icsr()
{
  return 0;
}
void  internal_ipic::set_icsr(reg_t v)
{
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
reg_t ipic_t::get_cisw() // RO
{
  return impl->get_cisw();
}
// aggregated access to fields of
// ISR (aggregated)
reg_t ipic_t::get_isvr() // RO
{
  return impl->get_isvr();
}
// IPR (aggregated)
void  ipic_t::set_ipr(reg_t v) // WC
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
