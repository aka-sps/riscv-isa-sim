require_extension('C');
require_extension('D');
require_fp;
MMU.store<uint64_t>(RVC_SP + insn.rvc_sdsp_imm(), RVC_FRS2);
