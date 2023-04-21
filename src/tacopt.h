#ifndef TACOPT_H_
#define TACOPT_H_ 1

#include "tac.h"

#define TAC_OPT_POWER_OF_TWO (1U << 0)
#define TAC_OPT_REUSE_TMPS (1U << 1)

#define TAC_OPT_OFF 0

#define TAC_OPT_FULL (~0U)

typedef unsigned tac_opt_flags_type;

void optimize_tac(struct tac *tac, tac_opt_flags_type flags);

#endif
