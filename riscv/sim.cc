// See LICENSE for license details.
#include "sim.hxx"
#include "htif.hxx"

#include <map>
#include <iostream>
#include <climits>
#include <cstdlib>
#include <cassert>
#include <csignal>

namespace riscv_isa_sim {
volatile bool ctrlc_pressed = false;
static void handle_signal(int sig)
{
  if (ctrlc_pressed)
    exit(-1);
  ctrlc_pressed = true;
  signal(sig, &handle_signal);
}

sim_t::sim_t(const char* isa, size_t nprocs, size_t mem_mb,
             const std::vector<std::string>& args)
  : htif(new htif_isasim_t(this, args)), procs(std::max(nprocs, size_t(1))),
    rtc(0), current_step(0), current_proc(0), debug(false)
{
  signal(SIGINT, &handle_signal);
  // allocate target machine's memory, shrinking it as necessary
  // until the allocation succeeds
  size_t memsz0 = (size_t)mem_mb << 20;
  size_t quantum = 1L << 20;
  if (memsz0 == 0)
    memsz0 = 1L << (sizeof(size_t) == 8 ? 32 : 30);

  memsz = memsz0;
  while ((mem = (char*)calloc(1, memsz)) == NULL)
    memsz = memsz*10/11/quantum*quantum;

  if (memsz != memsz0)
    fprintf(stderr, "warning: only got %lu bytes of target mem (wanted %lu)\n",
            (unsigned long)memsz, (unsigned long)memsz0);

  debug_mmu = new mmu_t(mem, memsz);

  for (size_t i = 0; i < procs.size(); i++)
    procs[i] = new processor_t(isa, this, i);
}

sim_t::~sim_t()
{
  for (size_t i = 0; i < procs.size(); i++)
    delete procs[i];
  delete debug_mmu;
  free(mem);
}

void sim_t::send_ipi(reg_t who)
{
  if (who < procs.size())
    procs[who]->deliver_ipi();
}

reg_t sim_t::get_scr(int which)
{
  switch (which)
  {
    case 0: return procs.size();
    case 1: return memsz >> 20;
    default: return -1;
  }
}

int sim_t::run()
{
  if (!debug && log)
    set_procs_debug(true);
  while (htif->tick())
  {
    if (debug || ctrlc_pressed)
      interactive();
    else
      step(INTERLEAVE);
  }
  return htif->exit_code();
}

void sim_t::step(size_t n)
{
  for (size_t i = 0, steps = 0; i < n; i += steps)
  {
    steps = std::min(n - i, INTERLEAVE - current_step);
    procs[current_proc]->step(steps);

    current_step += steps;
    if (current_step == INTERLEAVE)
    {
      current_step = 0;
      procs[current_proc]->yield_load_reservation();
      if (++current_proc == procs.size()) {
        current_proc = 0;
        rtc += INTERLEAVE / INSNS_PER_RTC_TICK;
      }

      htif->tick();
    }
  }
}

bool sim_t::running()
{
  for (size_t i = 0; i < procs.size(); i++)
    if (procs[i]->running())
      return true;
  return false;
}

void sim_t::stop()
{
  procs[0]->state.tohost = 1;
  while (htif->tick())
    ;
}

void sim_t::set_debug(bool value)
{
  debug = value;
}

void sim_t::set_log(bool value)
{
  log = value;
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
  return false;
}

bool sim_t::mmio_store(reg_t addr, size_t len, const uint8_t* bytes)
{
  return false;
}
}  // namespace riscv_isa_sim
