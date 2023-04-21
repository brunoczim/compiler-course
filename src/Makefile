LEX = lex

YACC = yacc

WATCH = make

ETAPA_DEPS = ast.o \
			 	panic.o \
				alloc.o \
				token_data.o \
				y.tab.o \
				lex.yy.o \
				symboltable.o \
				vector.o \
				semantics.o \
				types.o \
				const_eval.o \
				tac.o \
				tacgen.o \
				tacopt.o \
				x86_64_asm.o \
				x86_64_opt.o \
				x86_64_pc_linux_gnu_gen.o \
				main.o

etapa7: $(ETAPA_DEPS)
	$(CC) -o $@ $^

main.c: y.tab.c lex.yy.c

y.tab.c: parser.y
	$(YACC) -d $<

lex.yy.c: scanner.l y.tab.c
	$(LEX) $<

%.o: %.c
	$(CC) -o $@ -c $<

clean:
	$(RM) *.o etapa7 lex.yy.c y.tab.c y.tab.h out*.txt *.s a.out

compress: clean
	$(RM) etapa7.tgz
	tar cvzf etapa7.tgz *

watch:
	while inotifywait -e close_write *.h *.c *.y *.l Makefile; do $(WATCH); done
