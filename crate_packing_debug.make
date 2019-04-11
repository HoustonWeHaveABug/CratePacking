CRATE_PACKING_C_FLAGS=-g -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings

crate_packing_debug: crate_packing_debug.o
	gcc -g -o crate_packing_debug crate_packing_debug.o

crate_packing_debug.o: crate_packing.c crate_packing_debug.make
	gcc -c ${CRATE_PACKING_C_FLAGS} -o crate_packing_debug.o crate_packing.c

clean:
	rm -f crate_packing_debug crate_packing_debug.o
