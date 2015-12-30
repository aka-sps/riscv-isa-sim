// See LICENSE for license details.

#ifndef _HTIF_H
#define _HTIF_H

#include "fesvr/htif_pthread.hxx"

namespace riscv_isa_sim {
class sim_t;
struct packet;

// this class implements the host-target interface for program loading, etc.
// a simpler implementation would implement the high-level interface
// (read/write cr, read/write chunk) directly, but we implement the lower-
// level serialized interface to be more similar to real target machines.

class htif_isasim_t : public ::riscv_fesvr::htif_pthread_t
{
public:
  htif_isasim_t(sim_t* _sim, const std::vector<std::string>& args);
  bool tick();
  bool done();

private:
  sim_t* sim;
  bool reset;
  uint8_t seqno;

  void tick_once();
};
}  // namespace riscv_isa_sim

#endif
