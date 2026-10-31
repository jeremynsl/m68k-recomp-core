/* Shared portable MC68000 timing policy. */
#ifndef M68K_TIMING_H
#define M68K_TIMING_H

#include "m68k_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned m68k_68000_nominal_cycles(const M68KInstr *instr);
unsigned m68k_68000_operand_cycles(unsigned ea, int long_size);
unsigned m68k_68000_jump_cycles(unsigned ea, int call);

#ifdef __cplusplus
}
#endif

#endif
