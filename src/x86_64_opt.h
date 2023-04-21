#ifndef X86_64_OPT_H_
#define X86_64_OPT_H_ 1

#include "x86_64_asm.h"

#define X86_64_OPT_DEDUP_MOVS (1U << 0)

#define X86_64_OPT_INC_DECS (1U << 1)

#define X86_64_OPT_OFF 0

#define X86_64_OPT_FULL (~0U)

typedef unsigned x86_64_opt_flags_type;

void x86_64_opt(struct x86_64_asm_unit *unit, x86_64_opt_flags_type flags);

#endif
