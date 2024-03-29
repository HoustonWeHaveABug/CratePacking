CRATE_PACKING_DEBUG_C_FLAGS=-c -g -std=c89 -Wpedantic -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings -Wswitch-default -Wswitch-enum -Wbad-function-cast -Wstrict-overflow=5 -Wundef -Wlogical-op -Wfloat-equal -Wold-style-definition

crate_packing_debug: crate_packing_debug.o
	gcc -g -o crate_packing_debug crate_packing_debug.o -lm

crate_packing_debug.o: crate_packing.c crate_packing_debug.make
	gcc ${CRATE_PACKING_DEBUG_C_FLAGS} -o crate_packing_debug.o crate_packing.c

clean:
	rm -f crate_packing_debug crate_packing_debug.o
