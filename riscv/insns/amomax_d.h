require_extension('A');
require_rv64;
sreg_t v = MMU.load<int64_t>(RS1);
MMU.store<uint64_t>(RS1, std::max(sreg_t(RS2),v));
WRITE_RD(v);
