CRATE_PACKING_PG_C_FLAGS=-c -pg -std=c89 -Wpedantic -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings -Wswitch-default -Wswitch-enum -Wbad-function-cast -Wstrict-overflow=5 -Wundef -Wlogical-op -Wfloat-equal -Wold-style-definition

crate_packing_pg: crate_packing_pg.o
	gcc -pg -o crate_packing_pg crate_packing_pg.o -lm

crate_packing_pg.o: crate_packing.c crate_packing_pg.make
	gcc ${CRATE_PACKING_PG_C_FLAGS} -o crate_packing_pg.o crate_packing.c

clean:
	rm -f crate_packing_pg crate_packing_pg.o
