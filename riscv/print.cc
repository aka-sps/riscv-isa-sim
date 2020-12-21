#include "devices.h"

print_device_t::print_device_t()
{  
}

bool print_device_t::load(reg_t addr, size_t len, uint8_t *bytes)
{
  return true;
}

bool print_device_t::store(reg_t addr, size_t len, const uint8_t *bytes)
{
  putchar(bytes[0]);
  return true;
}
