require_extension('A');
require_rv64;
if (RS1 == p->get_state().load_reservation)
{
  MMU.store<uint64_t>(RS1, RS2);
  WRITE_RD(0);
}
else
  WRITE_RD(1);
