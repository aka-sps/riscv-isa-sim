#include "disasm.h"
#include "sim.h"
#include <stdint.h>
#include <string>
#include <vector>
#include <mtimer.h>

extern reg_t mtimer_base;

void sim_t::dyn_print_mtimer_member(std::vector<std::string> &args, const char *name, 
                                    size_t size_of_member, size_t off_in_struct, FILE *out_regs,
                                    bool crlf)
{

#define SIZE_OF_64BIT_HEX 22

    char conv_buf[SIZE_OF_64BIT_HEX + sizeof("0x") + sizeof('\0')];
    
    // conver address of timer_ctl and push them to arg
    snprintf(conv_buf, sizeof(conv_buf), "0x%lx", mtimer_base + off_in_struct);
    args.push_back(conv_buf);

    if (size_of_member == 4) { // if timer reg was 32-bit size
        fprintf(out_regs, "%s %08" PRIx32 "%c", name, (uint32_t)get_mem(args), crlf ? '\n' : ' ');
    } else {
        fprintf(out_regs, "%s %016" PRIx64 "%c", name, get_mem(args), crlf ? '\n' : ' ');
    }
    args.pop_back();
}

void sim_t::dyn_print_info(FILE* out_regs)
{
    if (!out_regs)
        return;
    
    std::vector<std::string> out_reg_args;
    struct mtreg test_mr; // to determine struct member size

    // roll back into start of file
    fseek(out_regs, 0, SEEK_SET);

    // TODO: temporary hardcoded to CPU 0
    out_reg_args.push_back("0"); // cpu 0

    // view cpu regs
    processor_t* p = get_core(out_reg_args[0]);

    for (int r = 0; r < NXPR; ++r) {
        fprintf(out_regs, "%-4s: %016" PRIx64 "  ", xpr_name[r], p->get_state()->XPR[r]);

        if ((r + 1) % 2 == 0) {
            fprintf(out_regs, "\n");
        }
    }

    fprintf(out_regs, "________________________CPU0______________________\nPC : 0x%016" PRIx64 "\n", get_pc(out_reg_args));
    
    // print timer content
    dyn_print_mtimer_member(out_reg_args, "TIMCTRL", sizeof(test_mr.control), offsetof(struct mtreg, control), out_regs, false);
    dyn_print_mtimer_member(out_reg_args, "        TIMDIV ", sizeof(test_mr.divider), offsetof(struct mtreg, divider), out_regs, true);
    dyn_print_mtimer_member(out_reg_args, "TIMCOUM", sizeof(test_mr.timer), offsetof(struct mtreg, timer), out_regs, false);
    dyn_print_mtimer_member(out_reg_args, "TIMCOMP", sizeof(test_mr.compare), offsetof(struct mtreg, compare), out_regs, true);
    
    /* printf some CSR registers, use handmade text identation */
    fprintf(out_regs, "\nMCYCLE  %016" PRIx64, p->get_state()->minstret);
    fprintf(out_regs, " MCAUSE  %016" PRIx64 "\n", p->get_state()->mcause);
    fprintf(out_regs, "MEPC    %016" PRIx64, p->get_state()->mepc);
    fprintf(out_regs, " MIP     %016" PRIx64 "\n", p->get_state()->mip);
    fprintf(out_regs, "MSTATUS %016" PRIx64, p->get_state()->mstatus);
    fprintf(out_regs, " MISA    %016" PRIx64 "\n", p->get_state()->misa);
    fprintf(out_regs, "MIE     %016" PRIx64, p->get_state()->mie);
    fprintf(out_regs, " MTVEC   %016" PRIx64 "\n", p->get_state()->mtvec);
    fprintf(out_regs, "MCNTENA %08" PRIx32, p->get_state()->mcounteren);
    fprintf(out_regs, "         MTVAL   %016" PRIx64 "\n", p->get_state()->mtval);
    fprintf(out_regs, "MSCRTCH %016" PRIx64, p->get_state()->mscratch);
    fprintf(out_regs, " MINSTRT %016" PRIx64 "\n", p->get_state()->minstret);
    fprintf(out_regs, "MEDELEG %016" PRIx64, p->get_state()->medeleg);
    fprintf(out_regs, " MIDELEG %016" PRIx64 "\n", p->get_state()->mideleg);

    fflush(out_regs);
}
