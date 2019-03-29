// See LICENSE for license details.

#include "sim.h"
#include "mmu.h"
#include "ini_file.h"
#include "dts.h"
#include "remote_bitbang.h"
#include <map>
#include <iostream>
#include <sstream>
#include <climits>
#include <cstdlib>
#include <cassert>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

volatile bool ctrlc_pressed = false;
static void handle_signal(int sig)
{
  if (ctrlc_pressed)
    exit(-1);
  ctrlc_pressed = true;
  signal(sig, &handle_signal);
}

sim_t::sim_t(const char* isa, size_t nprocs, bool halted, reg_t start_pc,
             std::vector<std::pair<reg_t, mem_t*>> mems,
             const std::vector<std::string>& args,
             std::vector<int> const hartids, unsigned progsize,
             unsigned max_bus_master_bits, bool require_authentication)
  : htif_t(args), mems(mems), procs(std::max(nprocs, size_t(1))),
    start_pc(start_pc), current_step(0), current_proc(0), debug(false),
    histogram_enabled(false), dtb_enabled(true), remote_bitbang(NULL),
    debug_module(this, progsize, max_bus_master_bits, require_authentication)
{
  signal(SIGINT, &handle_signal);

  for (auto& x : mems) {
    bus.add_device(x.first, x.second);
  }

  debug_module.add_device(&bus);

  debug_mmu = new mmu_t(this, NULL);

  if (hartids.size() == 0) {
    for (size_t i = 0; i < procs.size(); i++) {
      procs[i] = new processor_t(isa, this, i, halted);
    }
  }
  else {
    if (hartids.size() != procs.size()) {
      std::cerr << "Number of specified hartids doesn't match number of processors" << strerror(errno) << std::endl;
      exit(1);
    }
    for (size_t i = 0; i < procs.size(); i++) {
      procs[i] = new processor_t(isa, this, hartids[i], halted);
    }
  }

  clint.reset(new clint_t(procs));
  bus.add_device(CLINT_BASE, clint.get());
}

sim_t::~sim_t()
{
  for (size_t i = 0; i < procs.size(); i++)
    delete procs[i];
  delete debug_mmu;
}

void sim_thread_main(void* arg)
{
  ((sim_t*)arg)->main();
}

void sim_t::main()
{
  if (!debug && log)
    set_procs_debug(true);

  while (!done())
  {
    if (debug || ctrlc_pressed)
      interactive();
    else {
      step(1); //step(INTERLEAVE);
    }
    if (remote_bitbang) {
      remote_bitbang->tick();
    }
  }
}

int sim_t::run()
{
  host = context_t::current();
  target.init(sim_thread_main, this);
  return htif_t::run();
}

void sim_t::step(size_t n)
{
  for (size_t i = 0, steps = 0; i < n; i += steps)
  {
    processor_t *p = get_core("0");
    if ((p->get_state()->pc & 0x0FFFFFFFFULL) == get_sc_exit_addr()) {
      mmu_t* mmu; // = debug_mmu;
      reg_t xreg_addr = get_xreg_output_data();
      reg_t freg_addr = get_freg_output_data();
      reg_t val;

      fprintf(stderr, "ipc: 0x%016" PRIx64 "\n", p->get_state()->pc);

      val = get_reg2((char*)"a0");
      fprintf(stderr, "X10: 0x%016" PRIx64 "\n", val);

      val = get_reg2((char*)"a1");
      fprintf(stderr, "X11: 0x%016" PRIx64 "\n", val);

      val = get_reg2((char*)"a2");
      fprintf(stderr, "X12: 0x%016" PRIx64 "\n", val);

      fprintf(stderr, "xreg_addr: 0x%016" PRIx64 "\n", xreg_addr);
      fprintf(stderr, "freg_addr: 0x%016" PRIx64 "\n", freg_addr);
      

      if (xreg_addr || freg_addr) {
        FILE *pf = NULL;
        fprintf(stderr, "xreg_addr: %p\n", xreg_addr);
        if (xreg_addr & 0x80000000ULL) {
          xreg_addr |= ~0x0FFFFFFFFULL;
        }

        fprintf(stderr, "freg_addr: %p\n", freg_addr);
        if (freg_addr & 0x80000000ULL) {
          freg_addr |= ~0x0FFFFFFFFULL;
        }

        pf = fopen("regs_ref.c", "wt+");
        if (pf == NULL) {
          fprintf(stderr, "Error! Can't create file 'regs_ref.c'!\n");
        } else {
          mmu = p->get_mmu();
          if (xreg_addr) {
            fprintf(pf, "xreg_ref_data:\n");
            for (int i = 0; i < 32; i++) {
              val = mmu->load_uint64(xreg_addr);
              fprintf(pf, "reg_x%d_ref: .dword 0x%016" PRIx64 "\n", i, val);
              xreg_addr += 8;
            }
          }

          if (freg_addr) {
            fprintf(pf, "\nfreg_ref_data:\n");
            for (int i = 0; i < 32; i++) {
              val = mmu->load_uint64(freg_addr);
              fprintf(pf, "reg_f%d_ref: .dword 0x%016" PRIx64 "\n", i, val);
              freg_addr += 8;
            }
          }
          fclose(pf);
        }
      }
      stop();
      exit(0);
    }

    steps = std::min(n - i, INTERLEAVE - current_step);
    procs[current_proc]->step(steps);

    current_step += steps;
    if (current_step == INTERLEAVE)
    {
      current_step = 0;
      procs[current_proc]->get_mmu()->yield_load_reservation();
      if (++current_proc == procs.size()) {
        current_proc = 0;
        clint->increment(INTERLEAVE / INSNS_PER_RTC_TICK);
      }

      host->switch_to();
    }
  }
}

void sim_t::set_debug(bool value)
{
  debug = value;
}

void sim_t::set_log(bool value)
{
  log = value;
}

int sim_t::set_config_ini(char *config_name)
{
  std::string param_str(config_name);
  ini_params.Ini_file_load_by_fname(param_str);
  return 0;
}

char * sim_t::get_config_ini_str(char *sec_name, char *prm_name)
{
  std::string sec_str(sec_name);
  std::string prm_str(prm_name);
  return ini_params.get_param(sec_str, prm_str);
}

void sim_t::set_histogram(bool value)
{
  histogram_enabled = value;
  for (size_t i = 0; i < procs.size(); i++) {
    procs[i]->set_histogram(histogram_enabled);
  }
}

void sim_t::set_procs_debug(bool value)
{
  for (size_t i=0; i< procs.size(); i++)
    procs[i]->set_debug(value);
}

bool sim_t::mmio_load(reg_t addr, size_t len, uint8_t* bytes)
{
  if (addr + len < addr)
    return false;
  return bus.load(addr, len, bytes);
}

bool sim_t::mmio_store(reg_t addr, size_t len, const uint8_t* bytes)
{
  if (addr + len < addr)
    return false;
  return bus.store(addr, len, bytes);
}

extern char *dtc_path ;

void sim_t::make_dtb()
{
  const int reset_vec_size = 8;

  start_pc = start_pc == reg_t(-1) ? get_entry_point() : start_pc;

  uint32_t reset_vec[reset_vec_size] = {
    0x297,                                      // auipc  t0,0x0
    0x28593 + (reset_vec_size * 4 << 20),       // addi   a1, t0, &dtb
    0xf1402573,                                 // csrr   a0, mhartid
    get_core(0)->get_xlen() == 32 ?
      0x0182a283u :                             // lw     t0,24(t0)
      0x0182b283u,                              // ld     t0,24(t0)
    0x28067,                                    // jr     t0
    0,
    (uint32_t) (start_pc & 0xffffffff),
    (uint32_t) (start_pc >> 32)
  };

  std::vector<char> rom((char*)reset_vec, (char*)reset_vec + sizeof(reset_vec));

  dts = make_dts(INSNS_PER_RTC_TICK, CPU_HZ, procs, mems);
  dtc_path = get_config_ini_str("common", "dtc_path");
  fprintf(stderr, "dtc_path=%s\n", dtc_path);
  std::string dtb = dts_compile(dts);

  rom.insert(rom.end(), dtb.begin(), dtb.end());
  const int align = 0x1000;
  rom.resize((rom.size() + align - 1) / align * align);

  boot_rom.reset(new rom_device_t(rom));
  bus.add_device(DEFAULT_RSTVEC, boot_rom.get());
}

char* sim_t::addr_to_mem(reg_t addr) {
  auto desc = bus.find_device(addr);
  if (auto mem = dynamic_cast<mem_t*>(desc.second))
    if (addr - desc.first < mem->size())
      return mem->contents() + (addr - desc.first);
  return NULL;
}

// htif

void sim_t::reset()
{
  if (dtb_enabled)
    make_dtb();
}

void sim_t::idle()
{
  target.switch_to();
}

void sim_t::read_chunk(addr_t taddr, size_t len, void* dst)
{
  assert(len == 8);
  auto data = debug_mmu->load_uint64(taddr);
  memcpy(dst, &data, sizeof data);
}

void sim_t::write_chunk(addr_t taddr, size_t len, const void* src)
{
  assert(len == 8);
  uint64_t data;
  memcpy(&data, src, sizeof data);
  debug_mmu->store_uint64(taddr, data);
}

void sim_t::proc_reset(unsigned id)
{
  debug_module.proc_reset(id);
}
