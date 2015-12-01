// See LICENSE for license details.

#ifndef _RISCV_MMU_H
#define _RISCV_MMU_H

#include "decode.h"
#include "trap.h"
#include "common.h"
#include "config.h"
#include "processor.h"
#include "memtracer.h"
#include <cstdlib>
#include <cstdint>
#include <vector>

// virtual memory configuration
#define PGSHIFT 12
const reg_t PGSIZE = 1 << PGSHIFT;

struct insn_fetch_t
{
  insn_func_t func;
  insn_t insn;
};

struct icache_entry_t {
  reg_t tag;
  reg_t pad;
  insn_fetch_t data;
};

/* #define HW_PAGEWALKER */

#ifndef HW_PAGEWALKER
// Implementation of TLB subsystem
namespace TLB {

  // FIXME: support 64bit mode
  typedef uint32_t virt_addr;
  typedef uint64_t phys_addr;
  typedef uint32_t page_attr;

  struct tlb_entry {
    virt_addr vaddr;
    page_attr pattr;
  };

  enum TLB_constants {
    I_SETS = 8,
    I_WAYS = 4,
    I_ENTRIES = I_WAYS * I_SETS,
    D_SETS = 8,
    D_WAYS = 4,
    D_ENTRIES = D_WAYS * D_SETS,
  };

  enum TLB_page_consts {
    SV32_PAGESZ_BITS = 12,
  };

  // TODO: add compatibility with Rocket PTE ???
  // move type field to bit#1
  enum TLB_page_bits {
    SV32_PAGE_PPN_BITS = 20,
    SV32_PAGE_PPN_OFFS = 9, // physical page number
    SV32_PAGE_PPN = ((1 << SV32_PAGE_PPN_BITS) - 1) << SV32_PAGE_PPN_OFFS,
    SV32_PAGE_MP_BITS = 1, // megapage
    SV32_PAGE_MP_OFFS = 8,
    SV32_PAGE_NC_BITS = 1, // non cachable
    SV32_PAGE_NC_OFFS = 7,
    SV32_PAGE_D_BITS = 1, // dirty (occured write)
    SV32_PAGE_D_OFFS = 6,
    SV32_PAGE_R_BITS = 1, // referenced (occured any access)
    SV32_PAGE_R_OFFS = 5,
    SV32_PAGE_TYPE_BITS = 4,
    SV32_PAGE_TYPE_OFFS = 1, // page type
    SV32_PAGE_TYPE = (((1 << SV32_PAGE_TYPE_BITS) - 1) << SV32_PAGE_TYPE_OFFS),
    SV32_PAGE_V_BITS = 1, // valid entry
    SV32_PAGE_V_OFFS = 0,
    SV32_PAGE_VALID = (((1 << SV32_PAGE_V_BITS) - 1) << SV32_PAGE_V_OFFS),
  };

  enum PTE_types {
    PTE_NEXT      = 0,
    PTE_G_NEXT    = 1,
    PTE_SR_URX    = 2,
    PTE_SRW_URWX  = 3,
    PTE_SR_UR     = 4,
    PTE_SRW_URW   = 5,
    PTE_SRX_URX   = 6,
    PTE_SRWX_URWX = 7,
    PTE_SR        = 8,
    PTE_SRW       = 9,
    PTE_SRX       = 10,
    PTE_SRWX      = 11,
    PTE_G_SR      = 12,
    PTE_G_SRW     = 13,
    PTE_G_SRX     = 14,
    PTE_G_SRWX    = 15,
    PTE_TYPE_MASK = 0xf,
  };

  enum PTE_type_mask {
    PTE_TYPE_MASK_SR = (1 << PTE_SR_URX) | (1 << PTE_SRW_URWX)
    | (1 << PTE_SR_UR) | (1 << PTE_SRW_URW) | (1 << PTE_SRX_URX)
    | (1 << PTE_SRWX_URWX) | (1 << PTE_SR) | (1 << PTE_SRW)
    | (1 << PTE_SRX) | (1 << PTE_SRWX) | (1 << PTE_G_SR)
    | (1 << PTE_G_SRW) | (1 << PTE_G_SRX) | (1 << PTE_G_SRWX),
    PTE_TYPE_MASK_SW = (1 << PTE_SRW_URWX)
    | (1 << PTE_SRW_URW) | (1 << PTE_SRWX_URWX) | (1 << PTE_SRW)
    | (1 << PTE_SRWX) | (1 << PTE_G_SRW) | (1 << PTE_G_SRWX),
    PTE_TYPE_MASK_SX = (1 << PTE_SRX_URX) | (1 << PTE_SRWX_URWX)
    | (1 << PTE_SRX) | (1 << PTE_SRWX)
    | (1 << PTE_G_SRX) | (1 << PTE_G_SRWX),
    PTE_TYPE_MASK_UR = (1 << PTE_SR_URX) | (1 << PTE_SRW_URWX)
    | (1 << PTE_SR_UR) | (1 << PTE_SRW_URW) | (1 << PTE_SRX_URX)
    | (1 << PTE_SRWX_URWX),
    PTE_TYPE_MASK_UW = (1 << PTE_SRW_URWX)
    | (1 << PTE_SRW_URW) | (1 << PTE_SRWX_URWX),
    PTE_TYPE_MASK_UX = (1 << PTE_SR_URX) | (1 << PTE_SRW_URWX)
    | (1 << PTE_SRX_URX) | (1 << PTE_SRWX_URWX),
  };

  // check TLB entry permissions
  inline bool check_tlbe_perm(page_attr tlbe, bool supervisor, access_type type) {
    unsigned tlb_type = (tlbe & SV32_PAGE_TYPE) >> SV32_PAGE_TYPE_OFFS;
    if (type == LOAD) {
      if (supervisor)
        return ((PTE_TYPE_MASK_SR >> tlb_type) & 1);
      else
        return ((PTE_TYPE_MASK_UR >> tlb_type) & 1);
    } else if (type == STORE) {
      if (supervisor)
        return ((PTE_TYPE_MASK_SW >> tlb_type) & 1);
      else
        return ((PTE_TYPE_MASK_UW >> tlb_type) & 1);
    } else { // FETCH
      if (supervisor)
        return ((PTE_TYPE_MASK_SX >> tlb_type) & 1);
      else
        return ((PTE_TYPE_MASK_UX >> tlb_type) & 1);
    }
  }

  // extract physical page number
  inline unsigned tlbe_ppn(page_attr tlbe) {
    return (tlbe & SV32_PAGE_PPN) >> SV32_PAGE_PPN_OFFS;
  }
  // extract physical address
  inline phys_addr tlbe_phys_addr(page_attr tlbe) {
    return tlbe_ppn(tlbe) << SV32_PAGESZ_BITS;
  }

  // TODO: use CSR_MBADADDR instead of TLB::CSR_I_VADDR and TLB::CSR_D_VADDR
  enum TLB_regs {
    // instruction TLB setup CSRs
    CSR_I_PATTR       = 0x790, // ITLB page attr (trigger, write after itlb_vaddr)
    CSR_I_VADDR       = 0x791, // ITLB vaddr (pagevaddr, write first)
    // data TLB setup CSRs
    CSR_D_PATTR       = 0x792, // DTLB page attr (trigger, write after dtlb_vaddr)
    CSR_D_VADDR       = 0x793, // DTLB vaddr (write first)
    // get instruction/data TLB entry data
    CSR_I_ENTRY_SCAN  = 0x794, // get ITLB entry data, tlb_idx = way*sets_num+idx
    CSR_D_ENTRY_SCAN  = 0x795, // get DTLB entry data, tlb_idx = way*sets_num+idx
    CSR_I_ENTRY_PATTR = 0x796, // ITLB entry: page attr
    CSR_I_ENTRY_VADDR = 0x797, // ITLB entry: vaddr
    CSR_D_ENTRY_PATTR = 0x798, // DTLB entry: page attr
    CSR_D_ENTRY_VADDR = 0x799, // DTLB entry: vaddr
  };
}
#endif // !HW_PAGEWALKER

// this class implements a processor's port into the virtual memory system.
// an MMU and instruction cache are maintained for simulator performance.
class mmu_t
{
public:
  mmu_t(char* _mem, size_t _memsz);
  ~mmu_t();

  // template for functions that load an aligned value from memory
  #define load_func(type) \
      type##_t load_##type(reg_t addr) __attribute__((always_inline)) { \
        if (addr & (sizeof(type##_t)-1))                                \
          throw trap_load_address_misaligned(addr);                     \
      reg_t vpn = addr >> PGSHIFT;                                      \
      type##_t res;                                                     \
      if (likely(tlb_load_tag[vpn % TLB_ENTRIES] == vpn))  {            \
          res = *(type##_t*)(tlb_data[vpn % TLB_ENTRIES] + addr);       \
      } else {                                                          \
          load_slow_path(addr, sizeof(type##_t), (uint8_t*)&res);       \
      }                                                                 \
      if (proc && proc->debug)                                          \
          fprintf(stderr, "LOAD %08x %08x " #type " %08x\n", (unsigned)addr, \
                  (unsigned)(tlb_data[vpn % TLB_ENTRIES] + addr - mem), \
                  (unsigned)res); \
      return res; \
    }

  // load value from memory at aligned address; zero extend to register width
  load_func(uint8)
  load_func(uint16)
  load_func(uint32)
  load_func(uint64)

  // load value from memory at aligned address; sign extend to register width
  load_func(int8)
  load_func(int16)
  load_func(int32)
  load_func(int64)

  // template for functions that store an aligned value to memory
  #define store_func(type) \
    void store_##type(reg_t addr, type##_t val) { \
      if (addr & (sizeof(type##_t)-1)) \
        throw trap_store_address_misaligned(addr); \
      reg_t vpn = addr >> PGSHIFT; \
      if (likely(tlb_store_tag[vpn % TLB_ENTRIES] == vpn)) \
        *(type##_t*)(tlb_data[vpn % TLB_ENTRIES] + addr) = val; \
      else \
        store_slow_path(addr, sizeof(type##_t), (const uint8_t*)&val); \
      if (proc && proc->debug)                                          \
          fprintf(stderr, "STOR %08x %08x " #type " %08x\n", (unsigned)addr, \
                  (unsigned)(tlb_data[vpn % TLB_ENTRIES] + addr - mem), \
                  (unsigned)val); \
    }

  // store value to memory at aligned address
  store_func(uint8)
  store_func(uint16)
  store_func(uint32)
  store_func(uint64)

  static const reg_t ICACHE_ENTRIES = 1024;

  inline size_t icache_index(reg_t addr)
  {
    return (addr / PC_ALIGN) % ICACHE_ENTRIES;
  }

  inline icache_entry_t* refill_icache(reg_t addr, icache_entry_t* entry)
  {
    const uint16_t* iaddr = translate_insn_addr(addr);
    insn_bits_t insn = *iaddr;
    int length = insn_length(insn);

    if (likely(length == 4)) {
      if (likely(addr % PGSIZE < PGSIZE-2))
        insn |= (insn_bits_t)*(const int16_t*)(iaddr + 1) << 16;
      else
        insn |= (insn_bits_t)*(const int16_t*)translate_insn_addr(addr + 2) << 16;
    } else if (length == 2) {
      insn = (int16_t)insn;
    } else if (length == 6) {
      insn |= (insn_bits_t)*(const int16_t*)translate_insn_addr(addr + 4) << 32;
      insn |= (insn_bits_t)*(const uint16_t*)translate_insn_addr(addr + 2) << 16;
    } else {
      static_assert(sizeof(insn_bits_t) == 8, "insn_bits_t must be uint64_t");
      insn |= (insn_bits_t)*(const int16_t*)translate_insn_addr(addr + 6) << 48;
      insn |= (insn_bits_t)*(const uint16_t*)translate_insn_addr(addr + 4) << 32;
      insn |= (insn_bits_t)*(const uint16_t*)translate_insn_addr(addr + 2) << 16;
    }

    insn_fetch_t fetch = {proc->decode_insn(insn), insn};
    entry->tag = addr;
    entry->data = fetch;

    reg_t paddr = (const char*)iaddr - mem;
    if (tracer.interested_in_range(paddr, paddr + 1, FETCH)) {
      entry->tag = -1;
      tracer.trace(paddr, length, FETCH);
    }
    return entry;
  }

  inline icache_entry_t* access_icache(reg_t addr)
  {
    icache_entry_t* entry = &icache[icache_index(addr)];
    if (likely(entry->tag == addr))
      return entry;
    return refill_icache(addr, entry);
  }

  inline insn_fetch_t load_insn(reg_t addr)
  {
    return access_icache(addr)->data;
  }

  void set_processor(processor_t* p) { proc = p; flush_tlb(); }

  void flush_tlb();
  void flush_icache();

  void register_memtracer(memtracer_t*);

private:
  char* mem;
  size_t memsz;
  processor_t* proc;
  memtracer_list_t tracer;

  // implement an instruction cache for simulator performance
  icache_entry_t icache[ICACHE_ENTRIES];

  // implement a TLB for simulator performance
  static const reg_t TLB_ENTRIES = 256;
  char* tlb_data[TLB_ENTRIES];
  reg_t tlb_insn_tag[TLB_ENTRIES];
  reg_t tlb_load_tag[TLB_ENTRIES];
  reg_t tlb_store_tag[TLB_ENTRIES];

#ifndef HW_PAGEWALKER
  // instruction and data TLBs
  TLB::tlb_entry tlbi[TLB::I_ENTRIES];
  TLB::tlb_entry tlbd[TLB::D_ENTRIES];
  // latched info for selected i/d tlb entry
  TLB::tlb_entry tlbi_info;
  TLB::tlb_entry tlbd_info;
  // temp for setup tlbX entry
  TLB::virt_addr tlbi_vaddr;
  TLB::virt_addr tlbd_vaddr;

  TLB::tlb_entry *search_tlbi(TLB::virt_addr addr);
  TLB::tlb_entry *search_tlbd(TLB::virt_addr addr);
  /* void refill_tlbi_entry(TLB::tlb_entry *entry, reg_t pattr, reg_t vaddr); */
  /* void refill_tlbd_entry(TLB::tlb_entry *entry, reg_t pattr, reg_t vaddr); */
  void dbg_print_tlb(TLB::tlb_entry *tlb, unsigned tlb_sets, unsigned tlb_ways);
#endif // !HW_PAGEWALKER

  // finish translation on a TLB miss and upate the TLB
  void refill_tlb(reg_t vaddr, reg_t paddr, access_type type);

  // perform a page table walk for a given VA; set referenced/dirty bits
  reg_t walk(reg_t addr, bool supervisor, access_type type);

  // handle uncommon cases: TLB misses, page faults, MMIO
  const uint16_t* fetch_slow_path(reg_t addr);
  void load_slow_path(reg_t addr, reg_t len, uint8_t* bytes);
  void store_slow_path(reg_t addr, reg_t len, const uint8_t* bytes);
  reg_t translate(reg_t addr, access_type type);

  // ITLB lookup
  const uint16_t* translate_insn_addr(reg_t addr) __attribute__((always_inline)) {
    reg_t vpn = addr >> PGSHIFT;
    if (likely(tlb_insn_tag[vpn % TLB_ENTRIES] == vpn))
      return (uint16_t*)(tlb_data[vpn % TLB_ENTRIES] + addr);
    return fetch_slow_path(addr);
  }

  friend class processor_t;

 public:
#ifndef HW_PAGEWALKER
  // flush simulated TLB
  void flush_hw_tlb(void);
  // setup tlbX entry (use latched vaddr)
  void tlbi_setup_entry(TLB::virt_addr pattr);
  void tlbd_setup_entry(TLB::virt_addr pattr);
  // latch tlbX vaddr (for setup tlbX)
  void tlbi_set_vaddr(reg_t val) {
    tlbi_vaddr = static_cast<TLB::virt_addr>(val);
  }
  void tlbd_set_vaddr(reg_t val) {
    tlbd_vaddr = static_cast<TLB::virt_addr>(val);
  }
  // latch selected tlb entry
  void tlbi_scan(reg_t idx) {
    tlbi_info = tlbi[idx % TLB::I_ENTRIES];
  }
  void tlbd_scan(reg_t idx) {
    tlbd_info = tlbd[idx % TLB::D_ENTRIES];
  }
  // access to latched tlb entry
  reg_t tlbi_get_pattr(void) {
    return static_cast<reg_t>(tlbi_info.pattr);
  }
  reg_t tlbi_get_vaddr(void) {
    return static_cast<reg_t>(tlbi_info.vaddr);
  }
  reg_t tlbd_get_pattr(void) {
    return static_cast<reg_t>(tlbd_info.pattr);
  }
  reg_t tlbd_get_vaddr(void) {
    return static_cast<reg_t>(tlbd_info.vaddr);
  }
  // print content of TLBs to std::cerr
  void dbg_print_tlbi(void)
  {
      dbg_print_tlb(tlbi, TLB::I_SETS, TLB::I_WAYS);
  }
  void dbg_print_tlbd(void)
  {
      dbg_print_tlb(tlbd, TLB::D_SETS, TLB::D_WAYS);
  }
#endif // !HW_PAGEWALKER
};

#endif
