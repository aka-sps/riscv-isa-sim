// See LICENSE file for license details.

#ifndef DEBUG_ROM_DEFINES_H
#define DEBUG_ROM_DEFINES_H

// These are implementation-specific addresses in the Debug Module
//#define DEBUG_ROM_HALTED    0x1100
#define DEBUG_ROM_HALTED    0x1100
#define DEBUG_ROM_GOING     0x1104
#define DEBUG_ROM_RESUMING  0x1108
#define DEBUG_ROM_EXCEPTION 0x110C

// Region of memory where each hart has 1
// byte to read.
#define DEBUG_ROM_FLAGS 0x1400
#define DEBUG_ROM_FLAG_GO     0
#define DEBUG_ROM_FLAG_RESUME 1

// These needs to match the link.ld         
#define DEBUG_ROM_WHERETO 0x1300
#define DEBUG_ROM_ENTRY   0x1800
#define DEBUG_ROM_TVEC    0x1808
// Clobbered registers go here
#define DEBUG_ROM_SCRATCH 0x2000

#endif
