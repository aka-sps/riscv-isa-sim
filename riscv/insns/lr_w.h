require_extension('A');
p->get_state().load_reservation = RS1;
WRITE_RD(MMU.load<int32_t>(RS1));
