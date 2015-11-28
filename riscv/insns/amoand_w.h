require_extension('A');
reg_t v = MMU.load<int32_t>(RS1);
MMU.store<uint32_t>(RS1, RS2 & v);
WRITE_RD(v);
