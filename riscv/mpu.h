/* privs */
#define MPU_VALID 1
#define MPU_MMR 2
#define MPU_MMW 4
#define MPU_MMX 8
#define MPU_UMR 0x10
#define MPU_UMW 0x20
#define MPU_UMX 0x40
#define MPU_SMR 0x80
#define MPU_SMW 0x100
#define MPU_SMX 0x200
#define MPU_LOCK 0x80000000
#define MPU_MTYPE (3 << 16)
#define MPU_CONTROL_MASK (0x3FF | MPU_MTYPE | MPU_LOCK)
#define MPU_ADDR_MASK_32 (~(0x3FF | (3 << 30)))
#define MPU_ADDR_MASK_64 (~(0x3FFULL | (0x3FFFFFFULL << 38)))

#define MTYPE_C_WO (0 << 16)
#define MTYPE_NC_SO (1 << 16)
#define MTYPE_NC_WO (2 << 16)
#define MTYPE_MMIO_NC_SO (3 << 16)