// See LICENSE for license details.
#include "sim.hxx"

#include "htif.hxx"
#include "devicetree.hxx"
#include "spike_client.hxx"
#include <map>
#include <iostream>
#include <climits>
#include <cstdlib>
#include <cassert>
#include <csignal>
#include <new>

namespace riscv_isa_sim {

class vsim_device : public abstract_device_t
{
public:
    vsim_device(uint32_t a_base_address)
        : base_address(a_base_address)
    {
    }
    virtual bool
        load(reg_t addr, size_t len, uint8_t* bytes)override
    {
        auto& vcs = spike_vcs_TL::vcs_device_agent::instance();
        vcs.load(addr + base_address, len, bytes);
        return true;
    }
    virtual bool
        store(reg_t addr, size_t len, const uint8_t* bytes) override
    {
        auto& vcs = spike_vcs_TL::vcs_device_agent::instance();
        vcs.store(addr + base_address, len, bytes);
        return true;
    }

    uint32_t base_address;
};


volatile bool ctrlc_pressed = false;
static void handle_signal(int sig)
{
    if (ctrlc_pressed)
        exit(-1);  ///< \bug Using of exit() in c++ prevents normal sequence of object destruction
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
    size_t const memsz0 = mem_mb ? static_cast<size_t>(mem_mb) << 20 : 1LU << (sizeof(size_t) == 8 ? 32 : 30);
    static size_t const quantum = 1UL << 20;

    this->memsz = memsz0;
    while (!(this->mem = new(std::nothrow) char[memsz]))
        this->memsz = this->memsz * 10 / 11 / quantum * quantum;

    if (this->memsz != memsz0)
        fprintf(stderr, "warning: only got %lu bytes of target mem (wanted %lu)\n",
        static_cast<unsigned long>(memsz), static_cast<unsigned long>(memsz0));

    this->debug_mmu = new mmu_t(mem, memsz);

    for (size_t i = 0; i < procs.size(); ++i)
        procs[i] = new processor_t(isa, this, i);

    make_device_tree();
}

sim_t::~sim_t()
{
    for (size_t i = 0; i < procs.size(); i++)
        delete procs[i];
    delete debug_mmu;
    delete mem;
}

reg_t sim_t::get_scr(int which)
{
    switch (which) {
        case 0: return procs.size();
        case 1: return memsz >> 20;
        default: return -1;
    }
}

int sim_t::run()
{
    if (!debug && log)
        set_procs_debug(true);
    using namespace spike_vcs_TL;
    try {
        this->bus.add_device(this->memsz, new vsim_device(this->memsz));
        this->bus.add_device(0xFEED0000u, new vsim_device(0xFEED0000u));
        auto& vcs = spike_vcs_TL::vcs_device_agent::instance();
        vcs.wait_while_reset_is_active();
        while (htif->tick()) {
            if (debug || ctrlc_pressed)
                interactive();
            else
                step(INTERLEAVE);
            vcs.end_of_clock();
        }
    } catch (vcs_device_agent::Reset_active const& _excp) {
    }
    return htif->exit_code();
}

void sim_t::step(size_t n)
{
    for (size_t i = 0, steps = 0; i < n; i += steps) {
        steps = std::min(n - i, INTERLEAVE - current_step);
        procs[current_proc]->step(steps);

        current_step += steps;
        if (current_step == INTERLEAVE) {
            current_step = 0;
            procs[current_proc]->yield_load_reservation();
            if (++current_proc == procs.size()) {
                current_proc = 0;
                rtc += INTERLEAVE / INSNS_PER_RTC_TICK;
            }

            htif->tick();
        }
        auto& vcs = spike_vcs_TL::vcs_device_agent::instance();
        vcs.end_of_clock();
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
    for (size_t i = 0; i < procs.size(); i++)
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

void sim_t::make_device_tree()
{
    char buf[32];
    size_t max_devtree_size = procs.size() * 4096; // sloppy upper bound
    size_t cpu_size = NCSR * procs[0]->max_xlen / 8;
    reg_t cpu_addr = memsz + max_devtree_size;

    device_tree dt;
    dt.begin_node("");
    dt.add_prop("#address-cells", 2);
    dt.add_prop("#size-cells", 2);
    dt.add_prop("model", "Spike");
    dt.begin_node("memory@0");
    dt.add_prop("device_type", "memory");
    dt.add_reg({0, memsz});
    dt.end_node();
    dt.begin_node("cpus");
    dt.add_prop("#address-cells", 2);
    dt.add_prop("#size-cells", 2);
    for (size_t i = 0; i < procs.size(); i++) {
        sprintf(buf, "cpu@%" PRIx64, cpu_addr);
        dt.begin_node(buf);
        dt.add_prop("device_type", "cpu");
        dt.add_prop("compatible", "riscv");
        dt.add_prop("isa", procs[i]->isa);
        dt.add_reg({cpu_addr});
        dt.end_node();

        bus.add_device(cpu_addr, procs[i]);
        cpu_addr += cpu_size;
    }
    dt.end_node();
    dt.end_node();

    devicetree.reset(new rom_device_t(dt.finalize()));
    bus.add_device(memsz, devicetree.get());
}
}  // namespace riscv_isa_sim
