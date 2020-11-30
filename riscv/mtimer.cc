#include <sys/time.h>

#include "devices.h"
#include "mtimer.h"
#include "processor.h"

static struct mtreg mr;
static uint32_t status;

mtimer_device_t::mtimer_device_t(std::vector<processor_t*>&  procs)
  : procs(procs)
{
  printf("INIT MTIM\n");
  mr.control = (1 << MTIM_CTL_EN) | (0 << MTIM_CTL_CLKSRC);
}

bool mtimer_device_t::load(reg_t addr, size_t len, uint8_t *bytes)
{
  printf("READ MTIM ACCESS\n");
  if (isbadaddr(addr, len) != 0)
    return false;
  memcpy(bytes, (unsigned char *)&mr + addr, len); /* FIXME: cast */

  return true;
}

bool mtimer_device_t::store(reg_t addr, size_t len, const uint8_t *bytes)
{
  size_t i;

  printf("WRITE MTIM ACCESS\n");
  if (isbadaddr(addr, len) != 0)
    return false;
  if (addr >= offsetof(struct mtreg, compare)) {
    status &= ~INT_PENDING;
/*    printf("DEASSERTING INTERRUPT\t%d\t%d\n", mr.timer == mr.compare, status & INT_PENDING); */
    for (i = 0; i < procs.size(); i++)
      procs[i]->state.mip &= ~MIP_MTIP;
  }
  memcpy((unsigned char *)&mr + addr, bytes, len); /* FIXME: cast */
/* zero out reserved values */
  mr.control &= 3;
  mr.divider &= 0x3FF;
/* reset timer on 'invasive' access? */
  if ((addr < offsetof(struct mtreg, compare)) && (addr + len > offsetof(struct mtreg, divider)))
    mr.timer = 0;

  return true;
}

void mtimer_device_t::increment(reg_t howmuch)
{

/*
 * TODO: now it only increments tim each instruction (or so).
 * Fix this behavior, drop howmuch?
 */
  static struct timeval now, prev;
  static unsigned div;

  size_t i;

  if ((mr.control & (1 << MTIM_CTL_EN)) == 0)
    return;
  if ((mr.control & (1 << MTIM_CTL_CLKSRC)) == 0) {
    div += howmuch;
  } else {
    (void) gettimeofday(&now, NULL);
    div += (now.tv_sec - prev.tv_sec) * 1000 * 1000 + (now.tv_usec - prev.tv_usec);
    prev = now;
  }
/* if howmuch is big, div can overrun mr.divider multiple times */
  while (div > mr.divider) {
    div -= mr.divider + 1;
    mr.timer++;
  }
  if ((mr.timer == mr.compare) || (status & INT_PENDING)) {
/*    printf("ASSERTING INTERRUPT\t%d\t%d\n", mr.timer == mr.compare, status & INT_PENDING); */
  
    status |= INT_PENDING;
    for (i = 0; i < procs.size(); i++)
      procs[i]->state.mip |= MIP_MTIP;
  } else {
    for (i = 0; i < procs.size(); i++)
      procs[i]->state.mip &= ~MIP_MTIP;
  }
  return;
}

int isbadaddr(reg_t addr, size_t len)
{
  if ((len % 4) != 0) {
    printf("BAD MTIM ACCESS LENGTH\n");
    return 1;
  }
  if ((addr & 3) != 0) {
    printf("BAD MTIM ACCESS ALIGNMENT\n");
    return 1;
  }
  if ((addr >= sizeof(mr)) || ((addr + len) > sizeof(mr))) {
    printf("ACCESS OUT OF BOUNDS");
    return 1;
  }
  return 0;
}
