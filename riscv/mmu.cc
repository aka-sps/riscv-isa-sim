// See LICENSE for license details.

#include "mmu.h"
#include "sim.h"
#include "processor.h"
#include <iostream>
#include <iomanip>

#define DBG_TLB_LVL 0 // 0 - disabled, 1 - walk, 2 - detailed

mmu_t::mmu_t(char* _mem, size_t _memsz)
 : mem(_mem), memsz(_memsz), proc(NULL)
{
#ifdef HW_PAGEWALKER
  flush_tlb();
#else
  flush_hw_tlb();
#endif // HW_PAGEWALKER
}

mmu_t::~mmu_t()
{
}

void mmu_t::flush_icache()
{
  for (size_t i = 0; i < ICACHE_ENTRIES; i++)
    icache[i].tag = -1;
}

void mmu_t::flush_tlb()
{
  memset(tlb_insn_tag, -1, sizeof(tlb_insn_tag));
  memset(tlb_load_tag, -1, sizeof(tlb_load_tag));
  memset(tlb_store_tag, -1, sizeof(tlb_store_tag));

  flush_icache();
}

reg_t mmu_t::translate(reg_t addr, access_type type)
{
  if (!proc)
    return addr;

  reg_t mode = get_field(proc->state.mstatus, MSTATUS_PRV);
  if (type != FETCH && get_field(proc->state.mstatus, MSTATUS_MPRV))
    mode = get_field(proc->state.mstatus, MSTATUS_PRV1);
  if (get_field(proc->state.mstatus, MSTATUS_VM) == VM_MBARE)
    mode = PRV_M;

  if (mode == PRV_M) {
    reg_t msb_mask = (reg_t(2) << (proc->xlen-1))-1; // zero-extend from xlen
    return addr & msb_mask;
  }
  return walk(addr, mode > PRV_U, type) | (addr & (PGSIZE-1));
}

const uint16_t* mmu_t::fetch_slow_path(reg_t addr)
{
  reg_t paddr = translate(addr, FETCH);
  if (paddr >= memsz)
    throw trap_instruction_access_fault(addr);
  return (const uint16_t*)(mem + paddr);
}

void mmu_t::load_slow_path(reg_t addr, reg_t len, uint8_t* bytes)
{
  reg_t paddr = translate(addr, LOAD);
  if (paddr < memsz) {
    memcpy(bytes, mem + paddr, len);
    if (tracer.interested_in_range(paddr, paddr + PGSIZE, LOAD))
      tracer.trace(paddr, len, LOAD);
    else
      refill_tlb(addr, paddr, LOAD);
  } else if (!proc || !proc->sim->mmio_load(paddr, len, bytes)) {
    throw trap_load_access_fault(addr);
  }
}

void mmu_t::store_slow_path(reg_t addr, reg_t len, const uint8_t* bytes)
{
  reg_t paddr = translate(addr, STORE);
  if (paddr < memsz) {
    memcpy(mem + paddr, bytes, len);
    if (tracer.interested_in_range(paddr, paddr + PGSIZE, STORE))
      tracer.trace(paddr, len, STORE);
    else
      refill_tlb(addr, paddr, STORE);
  } else if (!proc || !proc->sim->mmio_store(paddr, len, bytes)) {
    throw trap_store_access_fault(addr);
  }
}

void mmu_t::refill_tlb(reg_t vaddr, reg_t paddr, access_type type)
{
  reg_t idx = (vaddr >> PGSHIFT) % TLB_ENTRIES;
  reg_t expected_tag = vaddr >> PGSHIFT;

  if (tlb_load_tag[idx] != expected_tag) tlb_load_tag[idx] = -1;
  if (tlb_store_tag[idx] != expected_tag) tlb_store_tag[idx] = -1;
  if (tlb_insn_tag[idx] != expected_tag) tlb_insn_tag[idx] = -1;

  if (type == FETCH) tlb_insn_tag[idx] = expected_tag;
  else if (type == STORE) tlb_store_tag[idx] = expected_tag;
  else tlb_load_tag[idx] = expected_tag;

  tlb_data[idx] = mem + paddr - vaddr;
}

#ifdef HW_PAGEWALKER
reg_t mmu_t::walk(reg_t addr, bool supervisor, access_type type)
{
  int levels, ptidxbits, ptesize;
  switch (get_field(proc->get_state()->mstatus, MSTATUS_VM))
  {
    case VM_SV32: levels = 2; ptidxbits = 10; ptesize = 4; break;
    case VM_SV39: levels = 3; ptidxbits = 9; ptesize = 8; break;
    case VM_SV48: levels = 4; ptidxbits = 9; ptesize = 8; break;
    default: abort();
  }

  // verify bits xlen-1:va_bits-1 are all equal
  int va_bits = PGSHIFT + levels * ptidxbits;
  reg_t mask = (reg_t(1) << (proc->xlen - (va_bits-1))) - 1;
  reg_t masked_msbs = (addr >> (va_bits-1)) & mask;
  if (masked_msbs != 0 && masked_msbs != mask)
    return -1;

  reg_t base = proc->get_state()->sptbr;
  int ptshift = (levels - 1) * ptidxbits;
  for (int i = 0; i < levels; i++, ptshift -= ptidxbits) {
    reg_t idx = (addr >> (PGSHIFT + ptshift)) & ((1 << ptidxbits) - 1);

    // check that physical address of PTE is legal
    reg_t pte_addr = base + idx * ptesize;
    if (pte_addr >= memsz)
      break;

    void* ppte = mem + pte_addr;
    reg_t pte = ptesize == 4 ? *(uint32_t*)ppte : *(uint64_t*)ppte;
    reg_t ppn = pte >> PTE_PPN_SHIFT;

    if (PTE_TABLE(pte)) { // next level of page table
      base = ppn << PGSHIFT;
    } else if (!PTE_CHECK_PERM(pte, supervisor, type == STORE, type == FETCH)) {
      break;
    } else {
      // set referenced and possibly dirty bits.
      *(uint32_t*)ppte |= PTE_R | ((type == STORE) * PTE_D);
      // for superpage mappings, make a fake leaf PTE for the TLB's benefit.
      reg_t vpn = addr >> PGSHIFT;
      return (ppn | (vpn & ((reg_t(1) << ptshift) - 1))) << PGSHIFT;
    }
  }

  return -1;
}
#else // HW_PAGEWALKER
TLB::tlb_entry *mmu_t::search_tlbi(TLB::virt_addr addr)
{
  TLB::virt_addr tlbe_vaddr = TLB::tlbe_vaddr_base(addr, false);
  TLB::virt_addr tlbe_mega_vaddr = TLB::tlbe_vaddr_base(addr, true);
  unsigned tlbe_idx = TLB::tlbe_tag(addr, false) & (TLB::I_SETS - 1);
  unsigned tlbe_mega_idx = TLB::tlbe_tag(addr, true) & (TLB::I_SETS - 1);

#if (DBG_TLB_LVL > 1)
  std::cerr << "<search_i " << std::hex << std::setfill('0') << std::setw(8) << addr
            << "=M" << tlbe_mega_idx << "/" << tlbe_idx << " ";
#endif // DBG_TLB_LVL
  // search megapage
  TLB::tlb_entry *entry = tlbi + tlbe_mega_idx;
  for (unsigned i = 0; i < TLB::I_WAYS; ++i, entry += TLB::I_SETS) {
    if ((entry->pattr & TLB::SV32_PAGE_VALID) && (entry->pattr & TLB::SV32_PAGE_MP) && entry->vaddr == tlbe_mega_vaddr) {
#if (DBG_TLB_LVL > 1)
      std::cerr << "found M way#" << i << ">";
#endif // DBG_TLB_LVL
      return entry;
    }
  }
  // search ordinar page
  entry = tlbi + tlbe_idx;
  for (unsigned i = 0; i < TLB::I_WAYS; ++i, entry += TLB::I_SETS) {
    if ((entry->pattr & TLB::SV32_PAGE_VALID) && !(entry->pattr & TLB::SV32_PAGE_MP) && entry->vaddr == tlbe_vaddr) {
#if (DBG_TLB_LVL > 1)
      std::cerr << "found way#" << i << ">";
#endif // DBG_TLB_LVL
      return entry;
    }
  }

#if (DBG_TLB_LVL > 1)
  std::cerr << "miss>";
#endif // DBG_TLB_LVL
  return 0;
}

TLB::tlb_entry *mmu_t::search_tlbd(TLB::virt_addr addr)
{
  TLB::virt_addr tlbe_vaddr = TLB::tlbe_vaddr_base(addr, false);
  TLB::virt_addr tlbe_mega_vaddr = TLB::tlbe_vaddr_base(addr, true);
  unsigned tlbe_idx = TLB::tlbe_tag(addr, false) & (TLB::D_SETS - 1);
  unsigned tlbe_mega_idx = TLB::tlbe_tag(addr, true) & (TLB::D_SETS - 1);

#if (DBG_TLB_LVL > 1)
  std::cerr << "<search_d " << std::hex << std::setfill('0') << std::setw(8) << addr
            << "=M" << tlbe_mega_idx << "/" << tlbe_idx << " ";
#endif // DBG_TLB_LVL

  // search megapage
  TLB::tlb_entry *entry = tlbd + tlbe_mega_idx;
  for (unsigned i = 0; i < TLB::D_WAYS; ++i, entry += TLB::D_SETS) {
    if ((entry->pattr & TLB::SV32_PAGE_VALID) && (entry->pattr & TLB::SV32_PAGE_MP) && entry->vaddr == tlbe_mega_vaddr) {
#if (DBG_TLB_LVL > 1)
      std::cerr << "found M way#" << i << ">";
#endif // DBG_TLB_LVL
      return entry;
    }
  }
  // search ordinar page
  entry = tlbd + tlbe_idx;
  for (unsigned i = 0; i < TLB::D_WAYS; ++i, entry += TLB::D_SETS) {
    if ((entry->pattr & TLB::SV32_PAGE_VALID) && !(entry->pattr & TLB::SV32_PAGE_MP) && entry->vaddr == tlbe_vaddr) {
#if (DBG_TLB_LVL > 1)
      std::cerr << "found way#" << i << ">";
#endif // DBG_TLB_LVL
      return entry;
    }
  }

#if (DBG_TLB_LVL > 1)
  std::cerr << "miss>";
#endif // DBG_TLB_LVL
  return 0;
}

void mmu_t::dbg_print_tlb(TLB::tlb_entry *tlb, unsigned tlb_sets, unsigned tlb_ways)
{
  TLB::tlb_entry *e = tlb;

  for (unsigned i = 0; i < tlb_sets; ++i, ++e) {
    std::cerr << std::dec << std::setfill('0') << std::setw(3) << i << ":";
    for (unsigned way = 0; way < tlb_ways; ++way) {
      std::cerr << " " << std::hex
                << std::setfill('0') << std::setw(8) << e[way * tlb_sets].vaddr
                << "," << std::setfill('0') << std::setw(8)
                << TLB::tlbe_phys_addr(e[way * tlb_sets].pattr);
    }
    std::cerr << std::endl;
    // decode type and attr bits
    std::cerr << "    ";
    for (unsigned way = 0; way < tlb_ways; ++way) {
      TLB::page_attr attr = e[way * tlb_sets].pattr;
      // decode type
      const char *type_str;
      switch (TLB::tlbe_type(attr)) {
      default:
        type_str = "???"; break;
      case TLB::PTE_NEXT:
        type_str = "NEXT"; break;
      case TLB::PTE_G_NEXT:
        type_str = "G_NEXT"; break;
      case TLB::PTE_SR_URX:
        type_str = "SR_URX"; break;
      case TLB::PTE_SRW_URWX:
        type_str = "SRW_URWX"; break;
      case TLB::PTE_SR_UR:
        type_str = "SR_UR"; break;
      case TLB::PTE_SRW_URW:
        type_str = "SRW_URW"; break;
      case TLB::PTE_SRX_URX:
        type_str = "SRX_URX"; break;
      case TLB::PTE_SRWX_URWX:
        type_str = "SRWX_URWX"; break;
      case TLB::PTE_SR:
        type_str = "SR"; break;
      case TLB::PTE_SRW:
        type_str = "SRW"; break;
      case TLB::PTE_SRX:
        type_str = "SRX"; break;
      case TLB::PTE_SRWX:
        type_str = "SRWX"; break;
      case TLB::PTE_G_SR:
        type_str = "G_SR"; break;
      case TLB::PTE_G_SRW:
        type_str = "G_SRW"; break;
      case TLB::PTE_G_SRX:
        type_str = "G_SRX"; break;
      case TLB::PTE_G_SRWX:
        type_str = "G_SRWX"; break;
      }
      std::cerr << std::setfill(' ') << std::setw(9) << type_str << "  ";
      // decode attr bits
      std::cerr << "  "
                << (attr & TLB::SV32_PAGE_MP ? "M" : "m")
                << (attr & TLB::SV32_PAGE_NC ? "C" : "c")
                << (attr & TLB::SV32_PAGE_D ? "D" : "d")
                << (attr & TLB::SV32_PAGE_R ? "R" : "r")
                << (attr & TLB::SV32_PAGE_VALID ? "V" : "v");
    }
    std::cerr << std::endl;
  }
}

reg_t mmu_t::walk(reg_t addr, bool supervisor, access_type type)
{
#if (DBG_TLB_LVL > 0)
  std::cerr << "<walk_tlb>:" << std::hex << addr
            << " " << (supervisor ? "S" : "U")
            << (type == LOAD ? "R" : (type == STORE ? "W" : "X")) << " ";
#endif // DBG_TLB_LVL

  TLB::tlb_entry *entry = 0;

  if (type == FETCH) {
    entry = search_tlbi(static_cast<TLB::virt_addr>(addr));
  } else {
    entry = search_tlbd(static_cast<TLB::virt_addr>(addr));
  }

  bool perm = false;

  if (entry) {
    perm = TLB::check_tlbe_perm(entry->pattr, supervisor, type);
#if (DBG_TLB_LVL > 0)
    std::cerr << std::dec << "[" << (type == FETCH ? entry - tlbi : entry - tlbd) << "]: ";
#endif // DBG_TLB_LVL
  }

  TLB::phys_addr phyaddr = static_cast<TLB::phys_addr>(-1);

  if (perm) {
    // set referenced and dirty bits
    entry->pattr |= TLB::SV32_PAGE_R | (type == STORE ? TLB::SV32_PAGE_D : 0);
    phyaddr = TLB::tlbe_phys_addr(entry->pattr);
    // copy vaddr bits when megapege
    if (entry->pattr & TLB::SV32_PAGE_MP) {
      phyaddr |= TLB::tlbe_vaddr_offs(static_cast<TLB::virt_addr>(addr), true) & ~((1 << TLB::SV32_PAGESZ_BITS) - 1);
    }
  }

#if (DBG_TLB_LVL > 0)
  if (!entry)
    std::cerr << "*MISS*";
  else if (!perm)
    std::cerr << "*DENY*";
  else
    std::cerr << std::hex << std::setfill('0') << std::setw(8) << phyaddr;
  std::cerr << std::endl;
  if (type == FETCH)
    dbg_print_tlbi();
  else
    dbg_print_tlbd();
#endif // DBG_TLB_LVL

  return static_cast<reg_t>(phyaddr);
}

static void refill_tlb_entry(TLB::tlb_entry *entry, unsigned idx, TLB::page_attr pattr, TLB::virt_addr vaddr)
{
  // mask vaddr offset
  vaddr &= ~((1 << (TLB::tlbe_megapage(pattr) ? TLB::SV32_MPAGESZ_BITS : TLB::SV32_PAGESZ_BITS)) - 1);

#if (DBG_TLB_LVL > 0)
  std::cerr << "set_tlb[" << idx << "]:"
            << std::hex << " (" << entry->vaddr << ", " << entry->pattr << ")"
            << " <- (" << vaddr << ", " << pattr << ")"
            << std::endl;
#endif // DBG_TLB_LVL
  entry->pattr = pattr;
  entry->vaddr = vaddr;
}

void mmu_t::tlbi_setup_entry(TLB::page_attr pattr)
{
  unsigned tlbe_idx = TLB::tlbe_tag(tlbi_vaddr, TLB::tlbe_megapage(pattr)) & (TLB::I_SETS - 1);
  // unsigned tlbe_idx = (tlbi_vaddr >> TLB::SV32_PAGESZ_BITS) & (TLB::I_SETS - 1);

#if (DBG_TLB_LVL > 0)
  std::cerr << "I:";
#endif // DBG_TLB_LVL

  // search existing entry
  TLB::tlb_entry *entry = search_tlbi(tlbi_vaddr);
  if (entry) {
    // update existing entry
    refill_tlb_entry(entry, tlbe_idx, pattr, tlbi_vaddr);
    return;
  }
  // search empty entry
  entry = tlbi + tlbe_idx;
  for (unsigned i = 0; i < TLB::I_WAYS; ++i, entry += TLB::I_SETS) {
    if (!(entry->pattr & TLB::SV32_PAGE_VALID)) {
      // fill empty entry
      refill_tlb_entry(entry, tlbe_idx, pattr, tlbi_vaddr);
      return;
    }
  }
  // roll entries in a ways dir and refill last way
  entry = tlbi + tlbe_idx;
  for (unsigned i = 1; i < TLB::I_WAYS; ++i, entry += TLB::I_SETS) {
    *entry = *(entry + TLB::I_SETS);
  }
  // refill last entry
  entry = tlbi + (TLB::I_WAYS - 1) * TLB::I_SETS + tlbe_idx;
  refill_tlb_entry(entry, tlbe_idx, pattr, tlbi_vaddr);
}

void mmu_t::tlbd_setup_entry(TLB::page_attr pattr)
{
  unsigned tlbe_idx = TLB::tlbe_tag(tlbd_vaddr, TLB::tlbe_megapage(pattr)) & (TLB::D_SETS - 1);
  // unsigned tlbe_idx = (tlbd_vaddr >> TLB::SV32_PAGESZ_BITS) & (TLB::D_SETS - 1);

#if (DBG_TLB_LVL > 0)
  std::cerr << "D:";
#endif // DBG_TLB_LVL

  // search existing entry
  TLB::tlb_entry *entry = search_tlbd(tlbd_vaddr);
  if (entry) {
    // update existing entry
    refill_tlb_entry(entry, tlbe_idx, pattr, tlbd_vaddr);
    return;
  }
  // search empty entry
  entry = tlbd + tlbe_idx;
  for (unsigned i = 0; i < TLB::D_WAYS; ++i, entry += TLB::D_SETS) {
    if (!(entry->pattr & TLB::SV32_PAGE_VALID)) {
      // fill empty entry
      refill_tlb_entry(entry, tlbe_idx, pattr, tlbd_vaddr);
      return;
    }
  }
  // roll entries in a ways dir and refill last way
  entry = tlbd + tlbe_idx;
  for (unsigned i = 1; i < TLB::D_WAYS; ++i, entry += TLB::D_SETS) {
    *entry = *(entry + TLB::D_SETS);
  }
  // refill last entry
  entry = tlbd + (TLB::D_WAYS - 1) * TLB::D_SETS + tlbe_idx;
  refill_tlb_entry(entry, tlbe_idx, pattr, tlbd_vaddr);
}

void mmu_t::flush_hw_tlb(void)
{
  memset(tlbi, 0, sizeof(tlbi));
  memset(tlbd, 0, sizeof(tlbd));

  flush_tlb();
}
#endif // HW_PAGEWALKER

void mmu_t::register_memtracer(memtracer_t* t)
{
  flush_tlb();
  tracer.hook(t);
}
