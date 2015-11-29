require_privilege(PRV_S);
#ifdef HW_PAGEWALKER
MMU.flush_tlb();
#else
MMU.flush_hw_tlb();
#endif
