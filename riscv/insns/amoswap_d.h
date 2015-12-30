require_extension('A');
require_rv64;
reg_t v = MMU.load<uint64_t>(RS1);
MMU.store<uint64_t>(RS1, RS2);
WRITE_RD(v);
