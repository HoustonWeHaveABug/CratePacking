#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

typedef enum {
	STATE_NONE,
	STATE_STAR,
	STATE_PRIME
}
state_t;

typedef struct {
	unsigned long m_max;
	unsigned long m;
	unsigned long *p;
}
mp_t;

typedef struct {
	unsigned long count;
	state_t state;
	double count_log;
	mp_t cost;
}
cell_t;

typedef struct {
	unsigned long row;
	unsigned long col;
}
location_t;

static unsigned long set_type_max(unsigned long, unsigned long, unsigned long);
static int set_cell(cell_t *, unsigned long, unsigned long);
static int set_costs(void);
static int compare_cells(const void *, const void *);
static int update_cost(cell_t *);
static int matrix_sum_min(void);
static int step1(void);
static int step2(void);
static int step3(location_t *);
static int find_zero(location_t *);
static int step4(location_t *);
static int step5(void);
static void set_location(location_t *, unsigned long, unsigned long);
static int mp_create(mp_t *, unsigned long);
static int mp_compare(const mp_t *, const mp_t *);
static int mp_convert_double(mp_t *, double);
static void hilo_add(unsigned long, unsigned long, unsigned long *, unsigned long *);
static void hilo_multiply(unsigned long, unsigned long, unsigned long *, unsigned long *);
static int mp_multiply(mp_t *, unsigned long);
static int mp_copy(mp_t *, mp_t *);
static void hilo_subtract(unsigned long, unsigned long, unsigned long *, unsigned long *);
static int mp_subtract(mp_t *, mp_t *);
static int mp_add(mp_t *, mp_t *);
static void mp_print(const char *, const mp_t *);
static void mp_free(mp_t *);

static int *rows, *cols;
static unsigned long long_digits, long_max, short_bits, short_max, dimensions_n, *crate, *box, cells_n;
static mp_t zero;
static cell_t *cells;
static location_t *locations;

int main(void) {
	unsigned long i;
	long_digits = sizeof(double);
	long_max = set_type_max(long_digits/2UL, sizeof(unsigned long), ULONG_MAX);
	short_bits = long_digits*2UL;
	short_max = set_type_max(long_digits/4UL, sizeof(unsigned short), USHRT_MAX);
	if (scanf("%lu", &dimensions_n) != 1 || dimensions_n < 1UL) {
		fputs("Invalid number of dimensions\n", stderr);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	crate = malloc(sizeof(unsigned long)*(size_t)dimensions_n);
	if (!crate) {
		fputs("Could not allocate memory for crate\n", stderr);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	for (i = 0UL; i < dimensions_n; ++i) {
		if (scanf("%lu", crate+i) != 1 || crate[i] < 1UL || crate[i] > long_max) {
			fputs("Invalid crate edge\n", stderr);
			fflush(stderr);
			free(crate);
			return EXIT_FAILURE;
		}
	}
	box = malloc(sizeof(unsigned long)*(size_t)dimensions_n);
	if (!box) {
		fputs("Could not allocate memory for box\n", stderr);
		fflush(stderr);
		free(crate);
		return EXIT_FAILURE;
	}
	for (i = 0UL; i < dimensions_n; ++i) {
		if (scanf("%lu", box+i) != 1 || box[i] < 1UL || box[i] > long_max) {
			fputs("Invalid box edge\n", stderr);
			fflush(stderr);
			free(box);
			free(crate);
			return EXIT_FAILURE;
		}
	}
	cells_n = dimensions_n*dimensions_n;
	cells = malloc(sizeof(cell_t)*(size_t)cells_n);
	if (!cells) {
		fputs("Could not allocate memory for cells\n", stderr);
		fflush(stderr);
		free(box);
		free(crate);
		return EXIT_FAILURE;
	}
	for (i = 0UL; i < dimensions_n; ++i) {
		unsigned long j;
		for (j = 0UL; j < dimensions_n; ++j) {
			if (!set_cell(cells+i*dimensions_n+j, i, j)) {
				free(cells);
				free(box);
				free(crate);
			}
		}
	}
	if (!set_costs()) {
		free(cells);
		free(box);
		free(crate);
		return EXIT_FAILURE;
	}
	if (!matrix_sum_min()) {
		mp_free(&zero);
		free(cells);
		free(box);
		free(crate);
		return EXIT_FAILURE;
	}
	mp_free(&zero);
	free(cells);
	free(box);
	free(crate);
	return EXIT_SUCCESS;
}

static unsigned long set_type_max(unsigned long type_size, unsigned long default_size, unsigned long default_max) {
	if (type_size < default_size) {
		unsigned long type_max = 0xffUL, i;
		for (i = 0UL; i < type_size; ++i) {
			type_max = (type_max << 8) | 0xffUL;
		}
		return type_max;
	}
	return default_max;
}

static int set_cell(cell_t *cell, unsigned long crate_edge, unsigned long box_edge) {
	cell->count = crate[crate_edge]/box[box_edge];
	cell->state = STATE_NONE;
	cell->count_log = cell->count > 0UL ? log((double)cell->count)+1:0;
	return mp_create(&cell->cost, (unsigned long)cell->count_log);
}

static int set_costs(void) {
	unsigned long i;
	mp_t cost_bak;
	cell_t **sorted_cells = malloc(sizeof(cell_t *)*(size_t)cells_n);
	if (!sorted_cells) {
		fputs("set_costs: could not allocate memory for sorted_cells\n", stderr);
		fflush(stderr);
		return 0;
	}
	for (i = 0UL; i < cells_n; ++i) {
		sorted_cells[i] = cells+i;
	}
	qsort(sorted_cells, (size_t)cells_n, sizeof(cell_t *), compare_cells);
	do {
		for (i = 1UL; i < cells_n; ++i) {
			if (sorted_cells[i]->count != sorted_cells[i-1UL]->count && !mp_compare(&sorted_cells[i]->cost, &sorted_cells[i-1UL]->cost)) {
				break;
			}
		}
		if (i == cells_n) {
			break;
		}
		for (i = 0UL; i < cells_n; ++i) {
			if (!update_cost(cells+i)) {
				free(sorted_cells);
				return 0;
			}
		}
	}
	while (1);
	do {
		double int_val;
		for (i = 0UL; i < cells_n; ++i) {
			if (modf(cells[i].count_log, &int_val) > 0) {
				break;
			}
		}
		if (i == cells_n) {
			break;
		}
		for (i = 0UL; i < cells_n; ++i) {
			if (!update_cost(cells+i)) {
				free(sorted_cells);
				return 0;
			}
		}
	}
	while (1);
	if (!mp_create(&cost_bak, 0UL)) {
		free(sorted_cells);
		return 0;
	}
	for (i = 1UL; i < cells_n; ++i) {
		if (!mp_copy(&sorted_cells[i]->cost, &cost_bak) || !mp_copy(&sorted_cells[0]->cost, &sorted_cells[i]->cost) || !mp_subtract(&sorted_cells[i]->cost, &cost_bak)) {
			mp_free(&cost_bak);
			free(sorted_cells);
			return 0;
		}
	}
	mp_free(&cost_bak);
	if (!mp_create(&zero, 0UL)) {
		free(sorted_cells);
		return 0;
	}
	mp_copy(&zero, &sorted_cells[0UL]->cost);
	free(sorted_cells);
	return 1;
}

static int compare_cells(const void *a, const void *b) {
	const cell_t *cell_a = *(cell_t * const *)a, *cell_b = *(cell_t * const *)b;
	if (cell_a->count < cell_b->count) {
		return 1;
	}
	if (cell_a->count > cell_b->count) {
		return -1;
	}
	return 0;
}

static int update_cost(cell_t *cell) {
	cell->count_log *= 2;
	return mp_convert_double(&cell->cost, cell->count_log);
}

static int matrix_sum_min(void) {
	int step;
	unsigned long i;
	mp_t boxes_n;
	location_t start;
	rows = calloc((size_t)dimensions_n, sizeof(int));
	if (!rows) {
		fputs("matrix_sum_min: could not allocate memory for rows\n", stderr);
		fflush(stderr);
		return 0;
	}
	cols = calloc((size_t)dimensions_n, sizeof(int));
	if (!cols) {
		fputs("matrix_sum_min: could not allocate memory for cols\n", stderr);
		fflush(stderr);
		free(rows);
		return 0;
	}
	locations = malloc(sizeof(location_t)*(size_t)cells_n);
	if (!locations) {
		fputs("matrix_sum_min: could not allocate memory for locations\n", stderr);
		fflush(stderr);
		free(cols);
		free(rows);
		return 0;
	}
	step = 1;
	set_location(&start, dimensions_n, dimensions_n);
	while (step && step != 6) {
		switch (step) {
			case 1:
				step = step1();
				break;
			case 2:
				step = step2();
				break;
			case 3:
				step = step3(&start);
				break;
			case 4:
				step = step4(&start);
				break;
			case 5:
				step = step5();
				break;
			default:
				break;
		}
	}
	if (!step) {
		free(locations);
		free(cols);
		free(rows);
		return 0;
	}
	printf("Assignment");
	for (i = 0UL; i < dimensions_n; ++i) {
		unsigned long j;
		for (j = 0UL; j < dimensions_n; ++j) {
			if (cells[i*dimensions_n+j].state == STATE_STAR) {
				printf(" %lu", j);
			}
		}
	}
	puts("");
	fflush(stdout);
	if (!mp_create(&boxes_n, 1UL)) {
		free(locations);
		free(cols);
		free(rows);
		return 0;
	}
	for (i = 0UL; i < dimensions_n; ++i) {
		unsigned long j;
		for (j = 0UL; j < dimensions_n; ++j) {
			if (cells[i*dimensions_n+j].state == STATE_STAR && !mp_multiply(&boxes_n, cells[i*dimensions_n+j].count)) {
				mp_free(&boxes_n);
				free(locations);
				free(cols);
				free(rows);
				return 0;
			}
		}
	}
	mp_print("Boxes", &boxes_n);
	fflush(stdout);
	mp_free(&boxes_n);
	free(locations);
	free(cols);
	free(rows);
	return 1;
}

static int step1(void) {
	unsigned long i;
	for (i = 0UL; i < dimensions_n; ++i) {
		unsigned long j;
		mp_t cost_min;
		if (!mp_create(&cost_min, 0UL)) {
			return 0;
		}
		if (!mp_copy(&cells[i*dimensions_n].cost, &cost_min)) {
			mp_free(&cost_min);
			return 0;
		}
		for (j = 1UL; j < dimensions_n; ++j) {
			if (mp_compare(&cells[i*dimensions_n+j].cost, &cost_min) < 0 && !mp_copy(&cells[i*dimensions_n+j].cost, &cost_min)) {
				mp_free(&cost_min);
				return 0;
			}
		}
		for (j = 0UL; j < dimensions_n; ++j) {
			if (!mp_subtract(&cells[i*dimensions_n+j].cost, &cost_min)) {
				mp_free(&cost_min);
				return 0;
			}
		}
		mp_free(&cost_min);
	}
	for (i = 0UL; i < dimensions_n; ++i) {
		unsigned long j;
		for (j = 0UL; j < dimensions_n; ++j) {
			if (!cols[j] && !mp_compare(&cells[i*dimensions_n+j].cost, &zero)) {
				cells[i*dimensions_n+j].state = STATE_STAR;
				cols[j] = 1;
				break;
			}
		}
	}
	for (i = 0UL; i < dimensions_n; ++i) {
		cols[i] = 0;
	}
	return 2;
}

static int step2(void) {
	unsigned long covered = 0UL, i;
	for (i = 0UL; i < dimensions_n; ++i) {
		unsigned long j;
		for (j = 0UL; j < dimensions_n; ++j) {
			if (cells[i*dimensions_n+j].state == STATE_STAR) {
				cols[j] = 1;
				++covered;
				break;
			}
		}
	}
	if (covered == dimensions_n) {
		return 6;
	}
	return 3;
}

static int step3(location_t *start) {
	while (1) {
		unsigned long col;
		if (!find_zero(start)) {
			return 5;
		}
		cells[start->row*dimensions_n+start->col].state = STATE_PRIME;
		for (col = 0UL; col < dimensions_n && cells[start->row*dimensions_n+col].state != STATE_STAR; ++col);
		if (col == dimensions_n) {
			return 4;
		}
		rows[start->row] = 1;
		cols[col] = 0;
	}
}

static int find_zero(location_t *start) {
	unsigned long i;
	for (i = 0UL; i < dimensions_n; ++i) {
		unsigned long j;
		for (j = 0UL; j < dimensions_n; ++j) {
			if (!rows[i] && !cols[j] && !mp_compare(&cells[i*dimensions_n+j].cost, &zero)) {
				set_location(start, i, j);
				return 1;
			}
		}
	}
	return 0;
}

static int step4(location_t *start) {
	unsigned long location_idx = 0UL, i;
	set_location(locations, start->row, start->col);
	while (1) {
		unsigned long row, col;
		for (row = 0UL; row < dimensions_n && cells[row*dimensions_n+locations[location_idx].col].state != STATE_STAR; ++row);
		if (row == dimensions_n) {
			break;
		}
		++location_idx;
		set_location(locations+location_idx, row, locations[location_idx-1UL].col);
		for (col = 0UL; col < dimensions_n && cells[locations[location_idx].row*dimensions_n+col].state != STATE_PRIME; ++col);
		++location_idx;
		set_location(locations+location_idx, locations[location_idx-1UL].row, col);
	}
	for (i = 0UL; i <= location_idx; ++i) {
		if (cells[locations[i].row*dimensions_n+locations[i].col].state == STATE_STAR) {
			cells[locations[i].row*dimensions_n+locations[i].col].state = STATE_NONE;
		}
		else if (cells[locations[i].row*dimensions_n+locations[i].col].state == STATE_PRIME) {
			cells[locations[i].row*dimensions_n+locations[i].col].state = STATE_STAR;
		}
	}
	for (i = 0UL; i < cells_n; ++i) {
		if (cells[i].state == STATE_PRIME) {
			cells[i].state = STATE_NONE;
		}
	}
	for (i = 0UL; i < dimensions_n; ++i) {
		rows[i] = 0;
	}
	for (i = 0UL; i < dimensions_n; ++i) {
		cols[i] = 0;
	}
	return 2;
}

static int step5(void) {
	unsigned long i;
	mp_t cost_min;
	cell_t *cell_min;
	if (!mp_create(&cost_min, 0UL)) {
		return 0;
	}
	cell_min = NULL;
	for (i = 0UL; i < dimensions_n; ++i) {
		unsigned long j;
		for (j = 0UL; j < dimensions_n; ++j) {
			if (!rows[i] && !cols[j] && (!cell_min || mp_compare(&cells[i*dimensions_n+j].cost, &cost_min) < 0)) {
				if (!mp_copy(&cells[i*dimensions_n+j].cost, &cost_min)) {
					mp_free(&cost_min);
					return 0;
				}
				cell_min = cells+i*dimensions_n+j;
			}
		}
	}
	for (i = 0UL; i < dimensions_n; ++i) {
		unsigned long j;
		for (j = 0UL; j < dimensions_n; ++j) {
			if (rows[i] && !mp_add(&cells[i*dimensions_n+j].cost, &cost_min)) {
				mp_free(&cost_min);
				return 0;
			}
			if (!cols[j] && !mp_subtract(&cells[i*dimensions_n+j].cost, &cost_min)) {
				mp_free(&cost_min);
				return 0;
			}
		}
	}
	mp_free(&cost_min);
	return 3;
}

static void set_location(location_t *location, unsigned long row, unsigned long col) {
	location->row = row;
	location->col = col;
}

static int mp_create(mp_t *mp, unsigned long val) {
	mp->p = malloc(sizeof(unsigned long));
	if (!mp->p) {
		fputs("mp_create: could not allocate memory for mp->p\n", stderr);
		fflush(stderr);
		return 0;
	}
	mp->m_max = 1UL;
	mp->m = 1UL;
	mp->p[0UL] = val;
	return 1;
}

static int mp_compare(const mp_t *mp_a, const mp_t *mp_b) {
	unsigned long i;
	if (mp_a->m < mp_b->m) {
		return -1;
	}
	if (mp_a->m > mp_b->m) {
		return 1;
	}
	for (i = mp_a->m; i > 0UL && mp_a->p[i-1UL] == mp_b->p[i-1UL]; --i);
	if (i > 0UL) {
		if (mp_a->p[i-1UL] < mp_b->p[i-1UL]) {
			return -1;
		}
		return 1;
	}
	return 0;
}

static int mp_convert_double(mp_t *mp, double val) {
	unsigned long i = 0;
	do {
		double q, int_q, int_r;
		if (i == mp->m_max) {
			unsigned long *p = realloc(mp->p, sizeof(unsigned long)*(size_t)(mp->m_max+1UL));
			if (!p) {
				fputs("mp_convert_double: could not reallocate memory for mp->p\n", stderr);
				fflush(stderr);
				return 0;
			}
			++mp->m_max;
			mp->p = p;
		}
		if (i == mp->m) {
			++mp->m;
		}
		q = val/(double)long_max;
		modf(q, &int_q);
		int_r = val-int_q*(double)long_max;
		mp->p[i++] = (unsigned long)int_r;
		val = int_q;
	}
	while (val > 0);
	return 1;
}

static void hilo_add(unsigned long a, unsigned long b, unsigned long *lo, unsigned long *hi) {
	unsigned long delta = long_max-b;
	if (a > delta) {
		*lo = a-delta-1UL;
		*hi = 1UL;
	}
	else {
		*lo = a+b;
		*hi = 0UL;
	}
}

static void hilo_multiply(unsigned long a, unsigned long b, unsigned long *lo, unsigned long *hi) {
	unsigned long a1 = a & short_max, a2 = a >> short_bits, b1 = b & short_max, b2 = b >> short_bits, m12 = a1*b2, m21 = a2*b1, carry1, carry2;
	hilo_add(a1*b1, (m12 & short_max) << short_bits, lo, &carry1);
	hilo_add(*lo, (m21 & short_max) << short_bits, lo, &carry2);
	*hi = carry1+carry2+(m12 >> short_bits)+(m21 >> short_bits)+a2*b2;
}

static int mp_multiply(mp_t *mp, unsigned long val) {
	unsigned long carry_bak, i;
	if (mp->m == mp->m_max) {
		unsigned long *p = realloc(mp->p, sizeof(unsigned long)*(size_t)(mp->m_max+1UL));
		if (!p) {
			fputs("mp_multiply: could not reallocate memory for mp->p\n", stderr);
			fflush(stderr);
			return 0;
		}
		++mp->m_max;
		mp->p = p;
	}
	carry_bak = 0UL;
	for (i = 0UL; i < mp->m; ++i) {
		unsigned long carry1, carry2;
		hilo_multiply(mp->p[i], val, mp->p+i, &carry1);
		hilo_add(mp->p[i], carry_bak, mp->p+i, &carry2);
		carry_bak = carry1+carry2;
	}
	if (carry_bak) {
		mp->p[mp->m++] = carry_bak;
	}
	for (; mp->m > 1UL && mp->p[mp->m-1UL] == 0UL; --mp->m);
	return 1;
}

static int mp_copy(mp_t *mp_a, mp_t *mp_b) {
	unsigned long i;
	if (mp_a->m > mp_b->m_max) {
		unsigned long *p = realloc(mp_b->p, sizeof(unsigned long)*(size_t)mp_a->m);
		if (!p) {
			fputs("mp_copy: could not reallocate memory for mp_b->p\n", stderr);
			fflush(stderr);
			return 0;
		}
		mp_b->m_max = mp_a->m;
		mp_b->p = p;
	}
	mp_b->m = mp_a->m;
	for (i = 0UL; i < mp_a->m; ++i) {
		mp_b->p[i] = mp_a->p[i];
	}
	return 1;
}

static void hilo_subtract(unsigned long a, unsigned long b, unsigned long *lo, unsigned long *hi) {
	if (a < b) {
		unsigned long delta = long_max-b;
		*lo = a+delta+1UL;
		*hi = 1UL;
	}
	else {
		*lo = a-b;
		*hi = 0UL;
	}
}

static int mp_subtract(mp_t *mp_a, mp_t *mp_b) {
	unsigned long carry_bak, i;
	if (mp_compare(mp_a, mp_b) < 0) {
		fputs("mp_subtract: result would be negative\n", stderr);
		fflush(stderr);
		return 0;
	}
	carry_bak = 0UL;
	for (i = 0UL; i < mp_b->m; ++i) {
		unsigned long carry1, carry2;
		hilo_subtract(mp_a->p[i], mp_b->p[i], mp_a->p+i, &carry1);
		hilo_subtract(mp_a->p[i], carry_bak, mp_a->p+i, &carry2);
		carry_bak = carry1+carry2;
	}
	for (; i < mp_a->m && carry_bak; ++i) {
		hilo_subtract(mp_a->p[i], carry_bak, mp_a->p+i, &carry_bak);
	}
	for (; mp_a->m > 1UL && mp_a->p[mp_a->m-1UL] == 0UL; --mp_a->m);
	return 1;
}

static int mp_add(mp_t *mp_a, mp_t *mp_b) {
	unsigned long carry_bak, i;
	if (mp_a->m_max <= mp_b->m) {
		unsigned long *p = realloc(mp_a->p, sizeof(unsigned long)*(size_t)(mp_b->m+1UL));
		if (!p) {
			fputs("mp_add: could not reallocate memory for mp_a->p\n", stderr);
			fflush(stderr);
			return 0;
		}
		mp_a->m_max = mp_b->m+1UL;
		mp_a->p = p;
	}
	if (mp_a->m < mp_b->m) {
		for (i = mp_a->m; i < mp_b->m; ++i) {
			mp_a->p[i] = 0UL;
		}
		mp_a->m = mp_b->m;
	}
	if (mp_a->m == mp_a->m_max) {
		unsigned long *p = realloc(mp_a->p, sizeof(unsigned long)*(size_t)(mp_a->m_max+1UL));
		if (!p) {
			fputs("mp_add: could not reallocate memory for mp_a->p\n", stderr);
			fflush(stderr);
			return 0;
		}
		++mp_a->m_max;
		mp_a->p = p;
	}
	carry_bak = 0UL;
	for (i = 0UL; i < mp_b->m; ++i) {
		unsigned long carry1, carry2;
		hilo_add(mp_a->p[i], mp_b->p[i], mp_a->p+i, &carry1);
		hilo_add(mp_a->p[i], carry_bak, mp_a->p+i, &carry2);
		carry_bak = carry1+carry2;
	}
	for (; i < mp_a->m && carry_bak; ++i) {
		hilo_add(mp_a->p[i], carry_bak, mp_a->p+i, &carry_bak);
	}
	if (carry_bak) {
		mp_a->p[mp_a->m++] = carry_bak;
	}
	return 1;
}

static void mp_print(const char *name, const mp_t *mp) {
	unsigned long i;
	printf("%s 0x%lx", name, mp->p[mp->m-1UL]);
	for (i = mp->m-1UL; i > 0UL; --i) {
		printf("%0*lx", (int)long_digits, mp->p[i-1UL]);
	}
	puts("");
}

static void mp_free(mp_t *mp) {
	free(mp->p);
}
