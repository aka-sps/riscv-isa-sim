require_extension('A');
uint32_t v = MMU.load<int32_t>(RS1);
MMU.store<uint32_t>(RS1, std::max(uint32_t(RS2),v));
WRITE_RD((int32_t)v);
