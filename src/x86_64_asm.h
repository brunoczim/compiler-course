#ifndef X86_64_ISA_H_
#define X86_64_ISA_H_ 1

#include <stddef.h>
#include <stdio.h>

#define X86_64_MAX_OPERANDS 2
#define X86_64_MAX_DIRECTIVE_OPERANDS 2

#define X86_64_OPERAND_0_SRC (1U << 0)
#define X86_64_OPERAND_0_DEST (1U << 1)
#define X86_64_OPERAND_1_SRC (1U << 2)
#define X86_64_OPERAND_1_DEST (1U << 3)
#define X86_64_OPERAND_RAX_SRC (1U << 4)
#define X86_64_OPERAND_RAX_DEST (1U << 5)
#define X86_64_OPERAND_RDX_SRC (1U << 6)
#define X86_64_OPERAND_RDX_DEST (1U << 7)
#define X86_64_OPERAND_RSP_SRC (1U << 8)
#define X86_64_OPERAND_RSP_DEST (1U << 9)
#define X86_64_OPERAND_RIP_SRC (1U << 10)
#define X86_64_OPERAND_RIP_DEST (1U << 11)
#define X86_64_OPERAND_EFLAGS_SRC (1U << 12)
#define X86_64_OPERAND_EFLAGS_DEST (1U << 13)

typedef unsigned x86_64_operand_flags_type;

enum x86_64_register_size {
    X86_64_BYTE = 1,
    X86_64_WORD = 2,
    X86_64_DWORD = 4,
    X86_64_QWORD = 8,
    X86_64_SSE = 16
};

enum x86_64_operand_tag {
    X86_64_OPERAND_DIRECT,
    X86_64_OPERAND_INDEXED,
    X86_64_OPERAND_SCALED,
    X86_64_OPERAND_DISPLACED,
    X86_64_OPERAND_DISPLACED_PLT,
    X86_64_OPERAND_IMMEDIATE,
    X86_64_OPERAND_ADDRESS,
    X86_64_OPERAND_PLT
};

enum x86_64_directive_name {
    X86_64_DATA,
    X86_64_RODATA,
    X86_64_TEXT,
    X86_64_GLOBL,
    X86_64_EXTERN,
    X86_64_TYPE,
    X86_64_ASCII,
    X86_64_DOUBLE,
    X86_64_QUAD,
    X86_64_ZERO,
    X86_64_ALIGN
};

enum x86_64_asm_stmt_tag {
    X86_64_INSTRUCTION,
    X86_64_LABEL,
    X86_64_DIRECTIVE
};

enum x86_64_opcode {
    X86_64_MOV,
    X86_64_MOVABS,
    X86_64_CMOVNS,
    X86_64_LEA,
    X86_64_NOT,
    X86_64_SHL,
    X86_64_SAR,
    X86_64_AND,
    X86_64_OR,
    X86_64_XOR,
    X86_64_ADD,
    X86_64_INC,
    X86_64_NEG,
    X86_64_SUB,
    X86_64_DEC,
    X86_64_IMUL,
    X86_64_IDIV,
    X86_64_CQO,
    X86_64_TEST,
    X86_64_CMP,
    X86_64_JMP,
    X86_64_JZ,
    X86_64_JNZ,
    X86_64_JC,
    X86_64_JNC,
    X86_64_JP,
    X86_64_JNP,
    X86_64_JL,
    X86_64_JG,
    X86_64_JLE,
    X86_64_JGE,
    X86_64_SETZ,
    X86_64_SETNZ,
    X86_64_SETP,
    X86_64_SETNP,
    X86_64_SETC,
    X86_64_SETNC,
    X86_64_SETG,
    X86_64_SETGE,
    X86_64_SETL,
    X86_64_SETLE,
    X86_64_PUSH,
    X86_64_POP,
    X86_64_CALL,
    X86_64_RET,
    X86_64_MOVQ,
    X86_64_MOVSD,
    X86_64_ADDSD,
    X86_64_SUBSD,
    X86_64_MULSD,
    X86_64_DIVSD,
    X86_64_UCOMISD
};

enum x86_64_register {
    X86_64_AH,
    X86_64_BH,
    X86_64_CH,
    X86_64_DH,
    X86_64_AL,
    X86_64_BL,
    X86_64_CL,
    X86_64_DL,
    X86_64_SIL,
    X86_64_DIL,
    X86_64_SPL,
    X86_64_BPL,
    X86_64_R8B,
    X86_64_R9B,
    X86_64_R10B,
    X86_64_R11B,
    X86_64_R12B,
    X86_64_R13B,
    X86_64_R14B,
    X86_64_R15B,
    X86_64_AX,
    X86_64_BX,
    X86_64_CX,
    X86_64_DX,
    X86_64_SI,
    X86_64_DI,
    X86_64_SP,
    X86_64_BP,
    X86_64_R8W,
    X86_64_R9W,
    X86_64_R10W,
    X86_64_R11W,
    X86_64_R12W,
    X86_64_R13W,
    X86_64_R14W,
    X86_64_R15W,
    X86_64_EAX,
    X86_64_EBX,
    X86_64_ECX,
    X86_64_EDX,
    X86_64_ESI,
    X86_64_EDI,
    X86_64_ESP,
    X86_64_EBP,
    X86_64_R8D,
    X86_64_R9D,
    X86_64_R10D,
    X86_64_R11D,
    X86_64_R12D,
    X86_64_R13D,
    X86_64_R14D,
    X86_64_R15D,
    X86_64_RAX,
    X86_64_RBX,
    X86_64_RCX,
    X86_64_RDX,
    X86_64_RSI,
    X86_64_RDI,
    X86_64_RSP,
    X86_64_RBP,
    X86_64_R8,
    X86_64_R9,
    X86_64_R10,
    X86_64_R11,
    X86_64_R12,
    X86_64_R13,
    X86_64_R14,
    X86_64_R15,
    X86_64_RIP,
    X86_64_XMM0,
    X86_64_XMM1,
    X86_64_XMM2,
    X86_64_XMM3,
    X86_64_XMM4,
    X86_64_XMM5,
    X86_64_XMM6,
    X86_64_XMM7,
    X86_64_XMM8,
    X86_64_XMM9,
    X86_64_XMM10,
    X86_64_XMM11,
    X86_64_XMM12,
    X86_64_XMM13,
    X86_64_XMM14,
    X86_64_XMM15
};

struct x86_64_indexed {
    enum x86_64_register base;
    enum x86_64_register index;
    struct symbol *displacement;
    int scale;
};

struct x86_64_scaled {
    enum x86_64_register index;
    struct symbol *displacement;
    int scale;
};

struct x86_64_displaced {
    enum x86_64_register base;
    struct symbol *displacement;
};

struct x86_64_operand {
    enum x86_64_operand_tag tag;
    union {
        enum x86_64_register direct;
        struct x86_64_indexed indexed;
        struct x86_64_scaled scaled;
        struct x86_64_displaced displaced;
        struct symbol *immediate;
        struct symbol *address;
    } data;
};

struct x86_64_instruction {
    enum x86_64_opcode opcode;
    size_t operand_count;
    struct x86_64_operand operands[X86_64_MAX_OPERANDS];
};

struct x86_64_directive {
    enum x86_64_directive_name name;
    size_t operand_count;
    struct symbol *operands[X86_64_MAX_DIRECTIVE_OPERANDS];
};

struct x86_64_asm_stmt {
    enum x86_64_asm_stmt_tag tag;
    union {
        struct x86_64_instruction instruction;
        struct symbol *label;
        struct x86_64_directive directive;
    } data;
};

struct x86_64_asm_unit {
    size_t length;
    struct x86_64_asm_stmt *statements;
};

enum x86_64_syntax {
    X86_64_AT_T_SYNTAX
};

enum x86_64_assembler {
    X86_64_GAS
};

struct x86_64_render_params {
    int space_count;
    enum x86_64_assembler assembler;
    FILE *output;
};

struct x86_64_asm_unit x86_64_asm_unit_empty(void);

enum x86_64_syntax x86_64_assembler_syntax(enum x86_64_assembler assembler);

void x86_64_asm_unit_push(
    struct x86_64_asm_unit *unit,
    struct x86_64_asm_stmt statement
);

void x86_64_asm_unit_splice(
    struct x86_64_asm_unit *unit,
    size_t start,
    size_t end,
    struct x86_64_asm_stmt *replacement,
    size_t repl_length,
    struct x86_64_asm_stmt *old
);

void x86_64_asm_unit_free(struct x86_64_asm_unit unit);

struct x86_64_asm_unit x86_64_asm_unit_vjoin(size_t count, va_list vargs);

struct x86_64_asm_unit x86_64_asm_unit_join(size_t count, ...);

void x86_64_render_asm_stmt(
    struct x86_64_asm_stmt statement,
    struct x86_64_render_params params
);

void x86_64_render(
    struct x86_64_asm_unit unit,
    struct x86_64_render_params params
);

enum x86_64_register x86_64_make_register_b(enum x86_64_register reg);

enum x86_64_register x86_64_make_register_w(enum x86_64_register reg);

enum x86_64_register x86_64_make_register_dw(enum x86_64_register reg);

enum x86_64_register x86_64_make_register_qw(enum x86_64_register reg);

enum x86_64_register_size x86_64_register_size(enum x86_64_register reg);

int x86_64_symbol_data_size(struct symbol const *symbol);

enum x86_64_register_size x86_64_symbol_reg_size(struct symbol const *symbol);

enum x86_64_register x86_64_make_register_of_size(
    enum x86_64_register_size size,
    enum x86_64_register reg
);

int x86_64_instruction_data_size(struct x86_64_instruction instruction);

x86_64_operand_flags_type x86_64_operand_flags(enum x86_64_opcode opcode);

int x86_64_operand_cmp(struct x86_64_operand left, struct x86_64_operand right);

int x86_64_is_operand_memory(enum x86_64_operand_tag tag);

char const *x86_64_register_name(enum x86_64_register reg);

char const *x86_64_opcode_mnemonic(enum x86_64_opcode opcode);

int x86_64_reg_unsized_eq(
    enum x86_64_register reg_left,
    enum x86_64_register reg_right
);

int x86_64_operand_data_size(struct x86_64_operand operand);

int x86_64_operand_uses_reg(
    struct x86_64_operand operand,
    enum x86_64_register reg
);

#endif
