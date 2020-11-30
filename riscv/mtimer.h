/* ctl bit-shifts */
#define MTIM_CTL_EN 0 
#define MTIM_CTL_CLKSRC 1

/* status flags */
#define INT_PENDING 1

int isbadaddr(reg_t, size_t);

/* no need for packed */
struct mtreg {
  uint32_t control;
  uint32_t divider;
  uint64_t timer;
/* TODO: implement per-hart private registers */
  uint64_t compare;
};
