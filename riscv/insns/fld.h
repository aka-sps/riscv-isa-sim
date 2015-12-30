require_extension('D');
require_fp;
WRITE_FRD(MMU.load<int64_t>(RS1 + insn.i_imm()));
