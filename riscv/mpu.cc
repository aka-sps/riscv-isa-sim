#include "mpu.h"

#include "simif.h"
#include "processor.h"

mpu_t::mpu_t(simif_t* sim, processor_t* proc, uint8_t entries)
 : sim(sim), proc(proc), entries(entries)
{
  _control = new uint32_t[entries]{ (MPU_VALID | MPU_MMR | MPU_MMW | MPU_MMX | MTYPE_MMIO_NC_SO) };
  _address = new reg_t[entries];
  _mask = new reg_t[entries];
}

mpu_t::~mpu_t()
{
  delete [] _control;
  delete [] _address;
  delete [] _mask;
}

void mpu_t::select(uint32_t sel)
{
  _select = sel & 0xf;
}

void mpu_t::control(uint32_t ctrl)
{
  if (_control[_select] & MPU_LOCK)
        return;
  if ((ctrl & MPU_MTYPE) == MTYPE_MMIO_NC_SO)
    ctrl &= ~(MPU_MMX | MPU_SMX | MPU_UMX); //per scr5 eas rv64 6.1.7

  _control[_select] = ctrl & MPU_CONTROL_MASK;
}

void mpu_t::address(reg_t addr)
{
  if (_control[_select] & MPU_LOCK)
    return;
  if (proc->get_xlen() == 32) {
    _address[_select] = addr & MPU_ADDR_MASK_32;
  } else if (proc->get_xlen() == 64) {
    _address[_select] = addr & MPU_ADDR_MASK_64;
  }
}

void mpu_t::mask(reg_t mask)
{
  if (_control[_select] & MPU_LOCK)
    return;
  if (proc->get_xlen() == 32) {
    _mask[_select] = mask & MPU_ADDR_MASK_32;
  } else if (proc->get_xlen() == 64) {
    _mask[_select] = mask & MPU_ADDR_MASK_64;
  }
}

uint32_t mpu_t::select()
{
  return _select;
}

uint32_t mpu_t::control()
{
  return _control[_select];
}

reg_t mpu_t::address()
{
  return _address[_select];
}

reg_t mpu_t::mask()
{
  return _mask[_select];
}

#define MPU_DBG_PRINTOUT 0

#if MPU_DBG_PRINTOUT
  static char* instr_type[] = {"L", "S", "F",};
#endif
reg_t mpu_t::mpu_ok(reg_t addr, reg_t len, access_type type, reg_t mode)
{
  reg_t atc;
  int i;
  bool gp, ga = true;

  for (i = 0; i < entries; i++) {
    gp = false;
    if (!(_control[i] & MPU_VALID))
      continue;
    reg_t phys_address = _address[i]<<2;
    reg_t phys_address_mask = _mask[i]<<2;
    //if (i)
    #if MPU_DBG_PRINTOUT
      printf("          MPU ENTRY %u: PHYS ADDR, MASK, CTRL: %#x %#x %#x\n", i, phys_address, phys_address_mask, s->mpu_control[i]);
    #endif
    for (atc = addr; atc < addr + len; atc++) {
      if ((atc & phys_address_mask) == (phys_address & phys_address_mask)) {
        switch (mode) {
        case PRV_M:
          if (((type == LOAD) && (_control[i] & MPU_MMR)) ||
              ((type == STORE) && (_control[i] & MPU_MMW)) ||
              ((type == FETCH) && (_control[i] & MPU_MMX))) {
            gp = true;
          } else {
            #if MPU_DBG_PRINTOUT
              printf("          ! MPU_OK(%#x, %u, %s, MACHINE) => %u at record %u\n", addr, len, instr_type[type], 0, i);
            #endif
            return false;
          }
          break;
        case PRV_S:
          if (((type == LOAD) && (_control[i] & MPU_SMR)) ||
              ((type == STORE) && (_control[i] & MPU_SMW)) ||
              ((type == FETCH) && (_control[i] & MPU_SMX))) {
            gp = true;
          } else {
            #if MPU_DBG_PRINTOUT
              printf("          ! MPU_OK(%#x, %u, %s, SUPERVISOR) => %u at record %u\n", addr, len, instr_type[type], 0, i);
            #endif
            return false;
          }
          break;
        case PRV_U:
          if (((type == LOAD) && (_control[i] & MPU_UMR)) ||
              ((type == STORE) && (_control[i] & MPU_UMW)) ||
              ((type == FETCH) && (_control[i] & MPU_UMX))) {
            gp = true;
          } else {
            #if MPU_DBG_PRINTOUT
              printf("          ! MPU_OK(%#x, %u, %s, USER) => %u at record %u\n", addr, len, instr_type[type], 0, i);
            #endif
            return false;
          }
          break;
        }
        if (gp == false)
          ga = false;
      }
    }
  }
  #if MPU_DBG_PRINTOUT
    printf("          MPU_OK(%#x, %u, %s, %s) => %u\n", addr, len, instr_type[type], (mode == PRV_U ? "USER" : (mode == PRV_M ? "MACHINE" : "SUPERVISOR") ), ga);
  #endif
  return ga;
}

bool mpu_t::mpu_mmio(reg_t addr, reg_t len)
{
  return false;
  if(!proc)
    return false;
  bool is_mmio_region = false;
  for (int i = 0; i < entries; i++) {
    if (!(_control[i] & MPU_VALID))
      continue;
    reg_t phys_address = _address[i]<<2;
    reg_t phys_address_mask = _mask[i]<<2;
    if ((addr & phys_address_mask) == (phys_address & phys_address_mask)) {
      if ((_control[i] & MPU_MTYPE) == MTYPE_MMIO_NC_SO) {
        printf("%u mtype region\n", i);
        is_mmio_region = true;
      }
    }
  }
  return is_mmio_region;
}
