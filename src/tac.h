#ifndef TAC_H_
#define TAC_H_ 1

#include <stdio.h>
#include <stdarg.h>
#include <limits.h>

#define TAC_MAX_OPERANDS 3

#define TAC_ID_UNKNOWN ULONG_MAX
#define TAC_ID_BOUNDARY (ULONG_MAX - 1)
#define TAC_ID_NON_LOCAL (ULONG_MAX - 2)
#define TAC_ID_MAX_OK (ULONG_MAX - 3)

typedef size_t tac_function_id_type;
typedef size_t tac_block_id_type;
typedef size_t tac_local_id_type;

enum tac_opcode {
    /**
     * mark beginning of function:
     *
     * .beginfun x:     -- <type> x(...) {
     */
    TAC_BEGINFUN,

    /**
     * declare parameter
     *
     * .defp x          -- , <type> x,
     */
    TAC_DEFP,

    /**
     * mark end of function:
     *
     * .endfun f        -- }
     */
    TAC_ENDFUN,

    /**
     * define scalar variable
     *
     * .defs x y        -- <type> x = y
     */
    TAC_DEFS,

    /**
     * begin vector variable definition
     *
     * .beginvec x:     -- <type> x[]
     */
    TAC_BEGINVEC,

    /**
     * define vector element
     *
     * .defv x          -- x
     */
    TAC_DEFV,

    /**
     * end vector variable definition
     *
     * .endvec x        -- [] - x;
     */
    TAC_ENDVEC,

    /**
     * move to scalar destination with scalar source
     *
     * move x, y        --  x = y
     */
    TAC_MOVE,

    /**
     * move to scalar destination with indexed source
     *
     * movi x, y, z     --  x = y[z]
     */
    TAC_MOVI,

    /**
     * move to vector destination
     *
     * movv x, y, z     --  x[y] = z
     */
    TAC_MOVV,

    /**
     * perform binary arithmetic operation
     *
     * <binop> x, y, z  --  x = y <binop> z
     */
    TAC_ADD,
    TAC_SUB,
    TAC_MUL,
    TAC_DIV,

    /**
     * perform binary comparison test
     *
     * <cmp> x, y, z    --  x = y <cmp> z
     */
    TAC_LT,
    TAC_GT,
    TAC_LE,
    TAC_GE,
    TAC_EQ,
    TAC_NE,

    /**
     * perform binary logical operation
     *
     * <binop> x, y, z  --  x = y <binop> z
     */
    TAC_AND,
    TAC_OR,

    /**
     * perform multiplication by power of two
     *
     *  shmul x, y, z     -- x = y << z (correcting negatives)
     */
    TAC_SHMUL,

    /**
     * perform division by power of two
     *
     *  shdiv x, y, z     -- x = y >> z (correcting negatives)
     */
    TAC_SHDIV,

    /**
     * perform unary logical operation
     *
     * <unop> x, y      -- x = <unop> y
     */
    TAC_NOT,

    /**
     * mark label entry point
     *
     * x:               -- x is a label pointing here
     */
    TAC_LABEL,

    /**
     * jump to destination if a value is zero
     *
     * ifz x, y         -- if y == 0 goto label x
     */
    TAC_IFZ,

    /**
     * jump to destination unconditionally
     *
     * jump x           -- goto label x
     */
    TAC_JUMP,

    /**
     * call a function
     *
     * call x           -- x(...);
     */
    TAC_CALL,

    /**
     * register an argument to the call of a function
     *
     * arg  , y         -- push y to argument list
     */
    TAC_ARG,

    /**
     * return from a function
     *
     * ret  , y         -- retorne y;
     */
    TAC_RET,

    /**
     * print to stdout
     *
     * print  , y       -- escreva y
     */
    TAC_PRINT,

    /**
     * read x           -- x = entrada;
     */
    TAC_READ
};

struct tac_instruction {
    enum tac_opcode opcode;
    struct symbol *dest;
    struct symbol *srcs[2];
};

struct tac_local_value {
    tac_function_id_type function_id;
    tac_block_id_type block_id;
    tac_local_id_type start_id;
    tac_local_id_type end_id;
    struct tac_node *start_node;
    struct tac_node *end_node;
    struct symbol *old_symbol;
    struct symbol *symbol_in_use;
    struct symbol *symbol_offered;
};

struct tac_local_value_set {
    size_t length;
    struct tac_local_value **ordered_values;
};

struct tac_node {
    struct tac_instruction instruction;
    tac_function_id_type function_id;
    tac_block_id_type block_id;
    tac_local_id_type local_id;
    struct tac_local_value *starting_local_value;
    int owns_starting_local_value;
    struct tac_local_value_set ending_local_values;
    struct tac_node *prev;
    struct tac_node *next;
};

struct tac {
    struct tac_node *first;
    struct tac_node *last;
    int locality_computed;
};

struct tac_render_params {
    int space_count;
    FILE *output;
};

struct tac_node *tac_create_node(struct tac_instruction instruction);

char const *tac_opcode_mnemonic(enum tac_opcode opcode);

char const *tac_opcode_raw_mnemonic(enum tac_opcode opcode);

void tac_instruction_print(
    struct tac_instruction instruction,
    struct tac_render_params params
);

void tac_instruction_raw_print(struct tac_instruction instruction);

void tac_print(struct tac tac, struct tac_render_params params);

void tac_raw_print(struct tac tac);

struct tac tac_empty(void);

struct tac tac_singleton(struct tac_instruction instruction);

void tac_prepend(struct tac *tac, struct tac_instruction instruction);

void tac_append(struct tac *tac, struct tac_instruction instruction);

int tac_pop(struct tac *tac, struct tac_instruction *output);

struct tac tac_vjoin(size_t count, va_list vargs);

struct tac tac_join(size_t count, ...);

void tac_free(struct tac tac);

int tac_is_block_boundary(enum tac_opcode opcode);

struct tac_local_value *tac_create_local_value(void);

void tac_draft_local_value_start(
    struct tac_node *target,
    struct tac_local_value *local_value
);

void tac_draft_local_value_end(
    struct tac_node *target,
    struct tac_local_value *local_value
);

void tac_confirm_local_value_start(struct tac_local_value *local_value);

void tac_confirm_local_value_end(struct tac_local_value *local_value);

void tac_mark_non_local_value(struct tac_local_value *local_value);

struct tac_node *tac_find_next_block_start(struct tac_node *from);

struct tac_node *tac_find_function_start(struct tac_node *from);

int tac_binsearch_local_value(
    struct tac_local_value_set set,
    tac_block_id_type block_id,
    tac_local_id_type start_id,
    size_t *index_out
);

int tac_insert_local_value(
    struct tac_local_value_set *set,
    struct tac_local_value *local_value,
    size_t *index_out
);

int tac_remove_local_value(
    struct tac_local_value_set *set,
    struct tac_local_value *local_value,
    size_t *index_out
);

void tac_debug_locality(struct tac tac);

void tac_compute_locality(struct tac *tac);

int tac_is_directive(enum tac_opcode opcode);

int tac_local_value_cmp_block_and_start(
    struct tac_local_value *left,
    tac_block_id_type right_block_id,
    tac_local_id_type right_start_id
);

int tac_local_value_cmp(
    struct tac_local_value *left,
    struct tac_local_value *right
);

#endif
