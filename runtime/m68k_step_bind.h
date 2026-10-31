/* Native generated-operation bindings. Include only in generated modules whose
 * operation context parameter is named ctx. No global/TLS CPU state is used. */
#ifndef M68K_STEP_BIND_H
#define M68K_STEP_BIND_H
#include "m68k_step.h"
#define g_cpu ctx->state.regs
#define m68k_brief_index_value m68k_step_brief_index_value
#define m68k_read8(a) m68k_step_read8(ctx,(a))
#define m68k_read16(a) m68k_step_read16(ctx,(a))
#define m68k_read32(a) m68k_step_read32(ctx,(a))
#define m68k_write8(a,v) m68k_step_write8(ctx,(a),(v))
#define m68k_write16(a,v) m68k_step_write16(ctx,(a),(v))
#define m68k_write32(a,v) m68k_step_write32(ctx,(a),(v))
#endif
