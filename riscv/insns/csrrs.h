int csr = validate_csr(insn.csr(), insn.rs1() != 0);
reg_t old = p->get_csr(csr);
if (insn.rs1() != 0) p->set_csr(csr, old | RS1);
WRITE_RD(sext_xlen(old));
