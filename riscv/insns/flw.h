require_extension('F');
require_fp;
WRITE_FRD(MMU.load<uint32_t>(RS1 + insn.i_imm()));
