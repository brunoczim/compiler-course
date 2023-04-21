#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "alloc.h"
#include "parser.h"
#include "semantics.h"
#include "tacgen.h"
#include "tacopt.h"
#include "x86_64_asm.h"
#include "x86_64_opt.h"
#include "x86_64_pc_linux_gnu_gen.h"

enum operation {
    OPERATION_CHECK_SYNTAX = 0,
    OPERATION_CHECK_SEMANTICS = 1,
    OPERATION_EMIT_DEBUG_TAC = 2,
    OPERATION_EMIT_ASSEMBLY_TAC = 3,
    OPERATION_EMIT_ASSEMBLY = 4,
    OPERATION_EMIT_OBJECT = 5,
    OPERATION_EMIT_EXECUTABLE = 6
};

struct arguments {
    enum operation operation;
    int debug;
    tac_opt_flags_type tac_opt_flags;
    x86_64_opt_flags_type x86_64_opt_flags;
    char const *source;
};

static int fork_and_exec(char const *file, char *const argv[]);

static struct arguments parse_arguments(int argc, char const *argv[]);

static void show_usage(void);

int main(int argc, char const *argv[])
{
    int exit_code = 0;
    unsigned error_count = 0;
    struct semantic_error_params semantic_error_params;
    struct tac tac;
    struct tac_render_params tac_render_params;
    struct x86_64_render_params x86_64_render_params;
    struct x86_64_asm_unit x86_64_asm_unit;
    size_t path_length;
    FILE *assembly_file;
    char *assembly_path;
    char *cc_args[5] = { NULL };

    struct arguments arguments = parse_arguments(argc, argv);
    
    setvbuf(stdout, NULL, _IONBF, 0);

    initMe();
    g_ast.is_valid = 0;
    yyin = fopen(arguments.source, "r");

    if (yyin == NULL) {
        perror(arguments.source);
        exit_code = 2;
    }

    if (exit_code == 0 && arguments.operation >= OPERATION_CHECK_SYNTAX) {
        if (!parse()) {
            exit_code = 3;
        } 
    }

    if (exit_code == 0 && arguments.operation >= OPERATION_CHECK_SEMANTICS) {
        semantic_error_params.error_count = &error_count;
        semantic_error_params.output = stderr;
        semantic_check_program(&g_ast, semantic_error_params);
        fprintf(stderr, "exiting with %u semantic errors...\n", error_count);
        if (error_count > 0) {
            exit_code = 4;
        }
    }

    if (exit_code == 0 && arguments.operation >= OPERATION_EMIT_DEBUG_TAC) {
        tac = gen_tac_for_ast(g_ast);
        optimize_tac(&tac, arguments.tac_opt_flags);

        if (arguments.operation < OPERATION_EMIT_ASSEMBLY_TAC) {
            fputs("generated TAC:\n\n", stderr);
            tac_raw_print(tac);
        } else if (arguments.operation < OPERATION_EMIT_ASSEMBLY) {
            tac_render_params.output = stdout;
            tac_render_params.space_count = 4;
            tac_print(tac, tac_render_params);
        } else {
            x86_64_asm_unit = x86_64_pc_linux_gnu_gen(tac);
            x86_64_opt(&x86_64_asm_unit, arguments.x86_64_opt_flags);

            path_length = strlen(arguments.source);
            assembly_path = aborting_malloc(path_length + 2 + 1);
            strcpy(assembly_path, arguments.source);
            strcpy(assembly_path + path_length, ".s");

            assembly_file = fopen(assembly_path, "w");
            if (assembly_file == NULL) {
                perror(arguments.source);
                exit_code = 2;
            }

            if (exit_code == 0) {
                x86_64_render_params.output = assembly_file;
                x86_64_render_params.space_count = 4;
                x86_64_render_params.assembler = X86_64_GAS;
                x86_64_render(x86_64_asm_unit, x86_64_render_params);
                x86_64_asm_unit_free(x86_64_asm_unit);
                fclose(assembly_file);

                if (arguments.operation == OPERATION_EMIT_OBJECT) {
                    cc_args[0] = "cc";
                    cc_args[1] = assembly_path;
                    cc_args[2] = "-c";
                    if (arguments.debug) {
                        cc_args[3] = "-g";
                    }
                    if (fork_and_exec("cc", cc_args) < 0) {
                        perror("cc");
                        exit_code = 5;
                    }
                } else if (arguments.operation == OPERATION_EMIT_EXECUTABLE) {
                    cc_args[0] = "cc";
                    cc_args[1] = assembly_path;
                    if (arguments.debug) {
                        cc_args[2] = "-g";
                    }
                    if (fork_and_exec("cc", cc_args) < 0) {
                        perror("cc");
                        exit_code = 6;
                    }
                }
            }
        }
        tac_free(tac);
    }


    ast_free(g_ast);
    g_ast.is_valid = 0;
    g_ast.declaration_list.declarations = NULL;
    freeMe();
    if (yyin != NULL) {
        fclose(yyin);
    }

    return exit_code;
}

static struct arguments parse_arguments(int argc, char const *argv[])
{
    size_t i;
    struct arguments arguments;
    int operation_given_count = 0;

    arguments.operation = OPERATION_EMIT_EXECUTABLE;
    arguments.source = NULL;
    arguments.debug = 0;
    arguments.tac_opt_flags = TAC_OPT_OFF;
    arguments.x86_64_opt_flags = X86_64_OPT_OFF;

    for (i = 1; i < argc; i++) {

        if (
            strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--check-syntax") == 0
        ) {
            operation_given_count++;
            arguments.operation = OPERATION_CHECK_SYNTAX;
        } else if (
            strcmp(argv[i], "-K") == 0
            || strcmp(argv[i], "--check-semantics") == 0
        ) {
            operation_given_count++;
            arguments.operation = OPERATION_CHECK_SEMANTICS;
        } else if (
            strcmp(argv[i], "-t") == 0
            || strcmp(argv[i], "--emit-debug-tac") == 0
        ) {
            operation_given_count++;
            arguments.operation = OPERATION_EMIT_DEBUG_TAC;
        } else if (
            strcmp(argv[i], "-T") == 0
            || strcmp(argv[i], "--emit-assembly-tac") == 0
        ) {
            operation_given_count++;
            arguments.operation = OPERATION_EMIT_ASSEMBLY_TAC;
        } else if (
            strcmp(argv[i], "-S") == 0
            || strcmp(argv[i], "--emit-assembly") == 0
        ) {
            operation_given_count++;
            arguments.operation = OPERATION_EMIT_ASSEMBLY;
        } else if (
            strcmp(argv[i], "-c") == 0
            || strcmp(argv[i], "--emit-obj-file") == 0
        ) {
            operation_given_count++;
            arguments.operation = OPERATION_EMIT_OBJECT;
        } else if (
            strcmp(argv[i], "-e") == 0
            || strcmp(argv[i], "--emit-executable") == 0
        ) {
            operation_given_count++;
            arguments.operation = OPERATION_EMIT_EXECUTABLE;
        } else if (
            strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0
        ) {
            show_usage();
        } else if (
            strcmp(argv[i], "-O") == 0 || strcmp(argv[i], "--optimize") == 0
        ) {
            arguments.x86_64_opt_flags = X86_64_OPT_FULL;
            arguments.tac_opt_flags = TAC_OPT_FULL;
        } else if (strcmp(argv[i], "-fdedup-movs") == 0) {
            arguments.x86_64_opt_flags |= X86_64_OPT_DEDUP_MOVS;
        } else if (strcmp(argv[i], "-finc-decs") == 0) {
            arguments.x86_64_opt_flags |= X86_64_OPT_INC_DECS;
            arguments.x86_64_opt_flags |= X86_64_OPT_DEDUP_MOVS;
        } else if (strcmp(argv[i], "-fpower-of-two") == 0) {
            arguments.tac_opt_flags |= TAC_OPT_POWER_OF_TWO;
        } else if (strcmp(argv[i], "-freuse-tmps") == 0) {
            arguments.tac_opt_flags |= TAC_OPT_REUSE_TMPS;
        } else if (
            strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--debug") == 0
        ) {
            arguments.debug = 1;
        } else {
            if (arguments.source != NULL) {
                fputs(
                    "cannot give argument for source code path more than once\n\n",
                    stderr
                );
                show_usage();
            }
            arguments.source = argv[i];
        }

        if (operation_given_count > 1) {
            fputs(
                "cannot give argument for operation more than once\n\n",
                stderr
            );
            show_usage();
        }
    }

    if (arguments.source == NULL) {
        fputs("argument for source code path required\n\n", stderr);
        show_usage();
    }

    return arguments;
}

static void show_usage(void)
{
    fputs("Usage: etapa7 [OPTIONS] <source code path>\n\n", stderr);
    fputs("Options:\n", stderr);
    fputs("    -k, --check-syntax           -- checks syntax\n", stderr);
    fputs("    -K, --check-semantics        -- checks semantics\n", stderr);
    fputs("    -t, --emit-debug-tac         -- emits debug TAC\n", stderr);
    fputs("    -T, --emit-assembly-tac      -- emits assembly TAC\n", stderr);
    fputs("    -S, --emit-assembly          -- emits assembly\n", stderr);
    fputs("    -c, --emit-obj-file          -- emits object file\n", stderr);
    fputs("    -e, --emit-executable        -- emits executable\n", stderr);
    fputs("    -O, --optimize               -- turns on all optimizations\n", stderr);
    fputs("    -fdedup-movs                 -- turns on dedup-movs optimization\n", stderr);
    fputs("    -finc-decs                   -- turns on inc-decs optimization\n", stderr);
    fputs("    -fpower-of-two               -- turns on power-of-two optimization\n", stderr);
    fputs("    -freuse-tmps                 -- turns on reuse-temps optimization\n", stderr);
    fputs("    -g, --debug                  -- generates assembly debug symbols\n", stderr);
    fputs("    -h, --help                   -- prints this message\n", stderr);
    exit(1);
}

static int fork_and_exec(char const *file, char *const argv[])
{
    pid_t pid = fork();
    int wstatus;
    if (pid < 0) {
        return -1;
    }
    if (pid == 0) {
        if (execvp(file, argv) < 0) {
            return -1;
        }
    }
    if (waitpid(pid, &wstatus, 0) < 0) {
        return -1;
    }
    if (!WIFEXITED(wstatus)) {
        return -1;
    }
    return 0;
}
