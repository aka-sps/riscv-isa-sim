int csr = validate_csr(insn.csr(), true);
reg_t old = p->get_csr(csr);
if ((reg_t)insn.rs1() != 0) p->set_csr(csr, old | insn.rs1());
WRITE_RD(sext_xlen(old));
