require_extension('A');
int32_t v = MMU.load<int32_t>(RS1);
MMU.store<uint32_t>(RS1, std::max(int32_t(RS2),v));
WRITE_RD(v);
