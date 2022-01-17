#include "csrs.h"
#include "sc_csrs.h"
// For processor_t:
#include "processor.h"
#include "mmu.h"
// For get_field():
#include "decode.h"
// For trap_virtual_instruction and trap_illegal_instruction:
#include "trap.h"

// implement class mpu_csr_t
mpu_csr_t::mpu_csr_t(processor_t* const proc, const reg_t addr):
  csr_t(proc, addr) {
  unlogged_write(0);
}

reg_t mpu_csr_t::read() const noexcept {
  reg_t ret_val = 0;
  switch (address) {
  case CSR_MPUSELECT:
    ret_val = proc->get_mpu()->select();
    break;
  case CSR_MPUCONTROL:
    ret_val = proc->get_mpu()->control();
    break;
  case CSR_MPUADDRESS:
    ret_val = proc->get_mpu()->address();
    break;
  case CSR_MPUMASK:
    ret_val = proc->get_mpu()->mask();
    break;
  default:
    // TODO: throw exception
    break;
  }
  return ret_val;
}

bool mpu_csr_t::unlogged_write(const reg_t val) noexcept  {
  switch (address) {
  case CSR_MPUSELECT:
    proc->get_mpu()->select(val);
    break;
  case CSR_MPUCONTROL:
    proc->get_mpu()->control(val);
    break;
  case CSR_MPUADDRESS:
    proc->get_mpu()->address(val);
    break;
  case CSR_MPUMASK:
    proc->get_mpu()->mask(val);
    break;
  default:
    return false;
  }
  log_write();
  return true;
}

// implement class l1ctrl_csr_t
l1ctrl_csr_t::l1ctrl_csr_t(processor_t* const proc, const reg_t addr):
  csr_t(proc, addr) {
  unlogged_write(0);
}

reg_t l1ctrl_csr_t::read() const noexcept {
  return val;
}

bool l1ctrl_csr_t::unlogged_write(const reg_t val) noexcept  {
  this->val = val;
  log_write();
  return true;
}

// implement class mtimer_csr_t
mtimer_csr_t::mtimer_csr_t(processor_t* const proc, const reg_t addr):
  csr_t(proc, addr) {
  unlogged_write(0);
}

reg_t mtimer_csr_t::read() const noexcept {
  uint8_t ret_val=0;
  reg_t cfg_region = proc->get_mpu()->get_cfg_region();
  if (cfg_region){
    proc->get_sim()->mmio_load((proc->get_mpu()->get_cfg_region())+8UL, 8, &ret_val); //0x8 - address of MTIMER_MTIMELO, len 8 for loading from MTIMER_MTIMELO and MTIMER_MTIMEHI together
  }
  return (reg_t)ret_val;
}

bool mtimer_csr_t::unlogged_write(const reg_t val) noexcept  {
  this->val = val;
  log_write();
  return true;
}