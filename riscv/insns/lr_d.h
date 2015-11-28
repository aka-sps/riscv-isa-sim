require_extension('A');
require_rv64;
p->get_state().load_reservation = RS1;
WRITE_RD(MMU.load<int64_t>(RS1));
