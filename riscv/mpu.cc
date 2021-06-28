#include "mpu.h"

#include "simif.h"
#include "processor.h"

mpu_t::mpu_t(simif_t* sim, processor_t* proc, uint8_t entries)
 : sim(sim), proc(proc), entries(entries)
{
  if (entries) {
    _control = new uint32_t[entries]{ (MPU_VALID | MPU_MMR | MPU_MMW | MPU_MMX | MTYPE_NC_SO) };
    _address = new reg_t[entries];
    _mask = new reg_t[entries];
  }
}

mpu_t::~mpu_t()
{
  if (entries) {
    delete [] _control;
    delete [] _address;
    delete [] _mask;
  }
}

bool mpu_t::is_enabled()
{
  return entries != 0;
}

void mpu_t::select(uint32_t sel)
{
  _select = sel % entries;
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
  static char* instr_mode[] = {"USER", "SUPERVISOR", "ERROR", "MACHINE"};
#endif
reg_t mpu_t::mpu_ok(reg_t addr, reg_t len, access_type type, reg_t mode)
{
  reg_t atc;
  int i;
  bool gp, ga = true;

  const static uint32_t control_bit[][3] = {
    {MPU_UMR, MPU_UMW, MPU_UMX}, //PRV_U
    {MPU_SMR, MPU_SMW, MPU_SMX}, //PRV_S
    {0,0,0},
    {MPU_MMR, MPU_MMW, MPU_MMX}  //PRV_M
  };

  for (i = 0; i < entries; i++) {
    gp = false;
    if (!(_control[i] & MPU_VALID))
      continue;

    reg_t phys_address = _address[i]<<2;
    reg_t phys_address_mask = _mask[i]<<2;

    #if MPU_DBG_PRINTOUT
      printf("          MPU ENTRY %u: PHYS ADDR, MASK, CTRL: %#x %#x %#x\n", i, phys_address, phys_address_mask, _control[i]);
    #endif
    for (atc = addr; atc < addr + len; atc++) { //FIXME: either check both ends or just the address as per EAS
      if ((atc & phys_address_mask) == (phys_address & phys_address_mask)) {
        if (_control[i] & control_bit[mode][type]) {
          gp = true;
        } else {
          #if MPU_DBG_PRINTOUT
            printf("          ! MPU_OK(%#x, %u, %s, %s) => 0 at record %u\n", addr, len, instr_type[type], instr_mode[mode], i);
          #endif
          return false;
        }
        if (gp == false)
          ga = false;
      }
    }
  }
  #if MPU_DBG_PRINTOUT
    printf("          MPU_OK(%#x, %u, %s, %s) => %u\n", addr, len, instr_type[type], instr_mode[mode] , ga);
  #endif
  return ga;
}

bool mpu_t::mpu_mmio(reg_t addr, reg_t len)
{
  for (int i = 0; i < entries; i++) {
    if (!(_control[i] & MPU_VALID))
      continue;
    reg_t phys_address = _address[i]<<2;
    reg_t phys_address_mask = _mask[i]<<2;
    if ((addr & phys_address_mask) == (phys_address & phys_address_mask)) {
      if ((_control[i] & MPU_MTYPE) == MTYPE_MMIO_NC_SO) {
        printf("%u mtype region\n", i);
        return true;
      }
    }
  }
  return false;
}

reg_t mpu_t::get_mmio_base(reg_t addr)
{
  for (int i = 0; i < entries; i++) {
    if (!(_control[i] & MPU_VALID))
      continue;
    reg_t phys_address = _address[i]<<2;
    reg_t phys_address_mask = _mask[i]<<2;
    if ((addr & phys_address_mask) == (phys_address & phys_address_mask)) {
      if ((_control[i] & MPU_MTYPE) == MTYPE_MMIO_NC_SO) {
        return phys_address;
      }
    }
  }
  return 0;
}
