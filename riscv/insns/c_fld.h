require_extension('C');
require_extension('D');
require_fp;
WRITE_RVC_FRS2S(MMU.load<int64_t>(RVC_RS1S + insn.rvc_ld_imm()));
