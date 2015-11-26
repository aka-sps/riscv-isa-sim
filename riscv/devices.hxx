#ifndef _RISCV_DEVICES_H
#define _RISCV_DEVICES_H

#include "decode.hxx"
#include <map>
#include <vector>

namespace riscv_isa_sim {
class abstract_device_t {
 public:
  virtual bool load(reg_t addr, size_t len, uint8_t* bytes) = 0;
  virtual bool store(reg_t addr, size_t len, const uint8_t* bytes) = 0;
  virtual ~abstract_device_t() {}
};

class bus_t : public abstract_device_t {
 public:
  bool load(reg_t addr, size_t len, uint8_t* bytes)override;
  bool store(reg_t addr, size_t len, const uint8_t* bytes)override;
  void add_device(reg_t addr, abstract_device_t* dev);

 private:
  std::map<reg_t, abstract_device_t*> devices;
};

class rom_device_t : public abstract_device_t {
 public:
  rom_device_t(std::vector<char> data);
  bool load(reg_t addr, size_t len, uint8_t* bytes)override;
  bool store(reg_t addr, size_t len, const uint8_t* bytes)override;
 private:
  std::vector<char> data;
};
}  // namespace riscv_isa_sim

#endif
