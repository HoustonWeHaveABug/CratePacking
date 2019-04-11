CRATE_PACKING_C_FLAGS=-O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings

crate_packing: crate_packing.o
	gcc -o crate_packing crate_packing.o

crate_packing.o: crate_packing.c crate_packing.make
	gcc -c ${CRATE_PACKING_C_FLAGS} -o crate_packing.o crate_packing.c

clean:
	rm -f crate_packing crate_packing.o
