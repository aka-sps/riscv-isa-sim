require_extension('A');
require_rv64;
reg_t v = MMU.load<uint64_t>(RS1);
MMU.store<uint64_t>(RS1, std::min(RS2,v));
WRITE_RD(v);
