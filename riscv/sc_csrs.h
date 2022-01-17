#ifndef _SC_CSRS_H
#define _SC_CSRS_H

// For reg_t:
#include "decode.h"
// For std::shared_ptr
#include <memory>
// For access_type:
#include "memtracer.h"

class processor_t;
struct state_t;

#define CSR_MPUSELECT       (0xbc4)
#define CSR_MPUCONTROL      (0xbc5)
#define CSR_MPUADDRESS      (0xbc6)
#define CSR_MPUMASK         (0xbc7)

class mpu_csr_t: public csr_t {
 public:
  mpu_csr_t(processor_t* const proc, const reg_t addr);
  virtual reg_t read() const noexcept override final;
  virtual bool unlogged_write(const reg_t val) noexcept;
};

#define CSR_MEMCTRLGLOBAL   (0xbd4)

class l1ctrl_csr_t: public csr_t {
 public:
  l1ctrl_csr_t(processor_t* const proc, const reg_t addr);
  virtual reg_t read() const noexcept override final;
  virtual bool unlogged_write(const reg_t val) noexcept;
private:
  reg_t val;
};

class mtimer_csr_t: public csr_t {
 public:
  mtimer_csr_t(processor_t* const proc, const reg_t addr);
  virtual reg_t read() const noexcept override final;
  virtual bool unlogged_write(const reg_t val) noexcept;
private:
  reg_t val;
};

#endif