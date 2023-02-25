#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct {
	unsigned val;
	int used;
}
box_edge_t;

typedef struct {
	box_edge_t *box_edge;
	unsigned count;
	unsigned product;
}
cell_t;

typedef struct {
	unsigned m_max;
	unsigned m;
	unsigned *p;
}
mp_t;

static int compare_uints(const void *, const void *);
static int set_box_edge(box_edge_t *);
static void set_cell(cell_t *, box_edge_t *, unsigned);
static int compare_cells(const void *, const void *);
static int mp_create(mp_t *, unsigned);
static void hilo_add(unsigned, unsigned, unsigned *, unsigned *);
static void hilo_mul(unsigned, unsigned, unsigned *, unsigned *);
static int mp_multiply(mp_t *, unsigned);
static int mp_compare(const mp_t *, const mp_t *);
static void mp_print(const mp_t *);
static void mp_free(mp_t *);
static void crate_packing(unsigned);

static unsigned dimensions_n, *crate, cells_n, short_bits, short_max, long_size, cost;
static box_edge_t *box_edges;
static cell_t *cells, **tried_cells;
static mp_t mp_product_max, mp_product;

int main(void) {
	unsigned i;
	if (scanf("%u", &dimensions_n) != 1 || dimensions_n < 2U) {
		fputs("Invalid number of dimensions\n", stderr);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	crate = malloc(sizeof(unsigned)*(size_t)dimensions_n);
	if (!crate) {
		fputs("Could not allocate memory for crate\n", stderr);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	for (i = 0U; i < dimensions_n; ++i) {
		if (scanf("%u", crate+i) != 1 || crate[i] < 1U) {
			fputs("Invalid crate edge\n", stderr);
			fflush(stderr);
			free(crate);
			return EXIT_FAILURE;
		}
	}
	qsort(crate, (size_t)dimensions_n, sizeof(unsigned), compare_uints);
	box_edges = malloc(sizeof(box_edge_t)*(size_t)dimensions_n);
	if (!box_edges) {
		fputs("Could not allocate memory for box_edges\n", stderr);
		fflush(stderr);
		free(crate);
		return EXIT_FAILURE;
	}
	for (i = 0U; i < dimensions_n; ++i) {
		if (!set_box_edge(box_edges+i)) {
			free(box_edges);
			free(crate);
			return EXIT_FAILURE;
		}
	}
	cells_n = dimensions_n*dimensions_n;
	cells = malloc(sizeof(cell_t)*(size_t)cells_n);
	if (!cells) {
		fputs("Could not allocate memory for cells\n", stderr);
		fflush(stderr);
		free(box_edges);
		free(crate);
		return EXIT_FAILURE;
	}
	for (i = 0U; i < dimensions_n; ++i) {
		unsigned j;
		for (j = 0U; j < dimensions_n; ++j) {
			set_cell(cells+i*dimensions_n+j, box_edges+j, crate[i]);
		}
		qsort(cells+i*dimensions_n, (size_t)dimensions_n, sizeof(cell_t), compare_cells);
	}
	tried_cells = malloc(sizeof(cell_t *)*(size_t)dimensions_n);
	if (!tried_cells) {
		fputs("Could not allocate memory for tried_cells\n", stderr);
		fflush(stderr);
		free(cells);
		free(box_edges);
		free(crate);
		return EXIT_FAILURE;
	}
	short_bits = (unsigned)sizeof(unsigned)*4U;
	short_max = (1U << short_bits)-1U;
	long_size = (unsigned)sizeof(unsigned)*2U;
	if (!mp_create(&mp_product_max, 0U)) {
		free(tried_cells);
		free(cells);
		free(box_edges);
		free(crate);
	}
	if (!mp_create(&mp_product, 1U)) {
		mp_free(&mp_product_max);
		free(tried_cells);
		free(cells);
		free(box_edges);
		free(crate);
	}
	cost = 0U;
	crate_packing(0U);
	printf("Cost %u\n", cost);
	fflush(stdout);
	mp_free(&mp_product);
	mp_free(&mp_product_max);
	free(tried_cells);
	free(cells);
	free(box_edges);
	free(crate);
	return EXIT_SUCCESS;
}

static int compare_uints(const void *a, const void *b) {
	const unsigned *uint_a = (const unsigned *)a, *uint_b = (const unsigned *)b;
	if (*uint_a < *uint_b) {
		return 1;
	}
	if (*uint_a > *uint_b) {
		return -1;
	}
	return 0;
}

static int set_box_edge(box_edge_t *box_edge) {
	if (scanf("%u", &box_edge->val) != 1 || box_edge->val < 1U) {
		fputs("Invalid box edge\n", stderr);
		fflush(stderr);
		return 0;
	}
	box_edge->used = 0;
	return 1;
}

static void set_cell(cell_t *cell, box_edge_t *box_edge, unsigned crate_edge) {
	cell->box_edge = box_edge;
	cell->count = crate_edge/box_edge->val;
	cell->product = box_edge->val*cell->count;
}

static int compare_cells(const void *a, const void *b) {
	const cell_t *cell_a = (const cell_t *)a, *cell_b = (const cell_t *)b;
	if (cell_a->product < cell_b->product) {
		return 1;
	}
	if (cell_a->product > cell_b->product) {
		return -1;
	}
	if (cell_a->box_edge < cell_b->box_edge) {
		return -1;
	}
	return 1;
}

static int mp_create(mp_t *mp, unsigned val) {
	mp->p = malloc(sizeof(unsigned));
	if (!mp->p) {
		fputs("Could not allocate memory for mp->p\n", stderr);
		fflush(stderr);
		return 0;
	}
	mp->m_max = 1U;
	mp->m = 1U;
	mp->p[0U] = val;
	return 1;
}

static int mp_copy(mp_t *mp_a, mp_t *mp_b) {
	unsigned i;
	if (mp_a->m > mp_b->m_max) {
		unsigned *p = realloc(mp_b->p, sizeof(unsigned)*(size_t)mp_a->m);
		if (!p) {
			fputs("Could not reallocate memory for mp_b->p\n", stderr);
			fflush(stderr);
			return 0;
		}
		mp_b->m_max = mp_a->m;
		mp_b->p = p;
	}
	mp_b->m = mp_a->m;
	for (i = 0U; i < mp_a->m; ++i) {
		mp_b->p[i] = mp_a->p[i];
	}
	return 1;
}

static void hilo_add(unsigned a, unsigned b, unsigned *lo, unsigned *hi) {
	*lo = a+b;
	*hi = a > *lo ? 1U:0U;
}

static void hilo_mul(unsigned a, unsigned b, unsigned *lo, unsigned *hi) {
	unsigned a1 = a & short_max, a2 = a >> short_bits, b1 = b & short_max, b2 = b >> short_bits, m12 = a1*b2, m21 = a2*b1, carry1, carry2;
	hilo_add(a1*b1, (m12 & short_max) << short_bits, lo, &carry1);
	hilo_add(*lo, (m21 & short_max) << short_bits, lo, &carry2);
	*hi = carry1+carry2+(m12 >> short_bits)+(m21 >> short_bits)+a2*b2;
}

static int mp_multiply(mp_t *mp, unsigned val) {
	unsigned carry_bak, i;
	if (mp->m == mp->m_max) {
		unsigned *p = realloc(mp->p, sizeof(unsigned)*(size_t)(mp->m_max+1U));
		if (!p) {
			fputs("Could not reallocate memory for mp->p\n", stderr);
			fflush(stderr);
			return 0;
		}
		++mp->m_max;
		mp->p = p;
	}
	carry_bak = 0U;
	for (i = 0U; i < mp->m; ++i) {
		unsigned carry1, carry2;
		hilo_mul(mp->p[i], val, mp->p+i, &carry1);
		hilo_add(mp->p[i], carry_bak, mp->p+i, &carry2);
		carry_bak = carry1+carry2;
	}
	if (carry_bak) {
		mp->p[mp->m++] = carry_bak;
	}
	return 1;
}

static int mp_compare(const mp_t *mp_a, const mp_t *mp_b) {
	unsigned i;
	if (mp_a->m < mp_b->m) {
		return -1;
	}
	if (mp_a->m > mp_b->m) {
		return 1;
	}
	for (i = mp_a->m; i > 0U && mp_a->p[i-1U] == mp_b->p[i-1U]; --i);
	if (i > 0U) {
		if (mp_a->p[i-1U] < mp_b->p[i-1U]) {
			return -1;
		}
		if (mp_a->p[i-1U] > mp_b->p[i-1U]) {
			return 1;
		}
	}
	return 0;
}

static void mp_print(const mp_t *mp) {
	unsigned i;
	printf("%x", mp->p[mp->m-1U]);
	for (i = mp->m-1U; i > 0U; --i) {
		printf("%0*x", long_size, mp->p[i-1U]);
	}
	puts("");
}

static void mp_free(mp_t *mp) {
	free(mp->p);
}

static void crate_packing(unsigned dimension) {
	unsigned i;
	mp_t mp_tmp;
	++cost;
	if (dimension < dimensions_n) {
		if (!mp_create(&mp_tmp, 0U)) {
			return;
		}
		if (!mp_copy(&mp_product, &mp_tmp)) {
			mp_free(&mp_tmp);
			return;
		}
		for (i = dimension; i < dimensions_n; ++i) {
			unsigned j;
			for (j = i*dimensions_n; j < i*dimensions_n+dimensions_n && cells[j].box_edge->used; ++j);
			if (!mp_multiply(&mp_tmp, cells[j].product)) {
				mp_free(&mp_tmp);
				return;
			}
			if (mp_compare(&mp_tmp, &mp_product_max) > 0) {
				break;
			}
		}
		if (i == dimensions_n) {
			mp_free(&mp_tmp);
			return;
		}
		mp_copy(&mp_product, &mp_tmp);
		for (i = dimension*dimensions_n; i < dimension*dimensions_n+dimensions_n; ++i) {
			if (!cells[i].box_edge->used && (dimension == 0U || crate[dimension] != crate[dimension-1U] || cells[i].count != tried_cells[dimension-1U]->count || cells[i].box_edge > tried_cells[dimension-1U]->box_edge) && (i == dimension*dimensions_n || cells[i-1U].box_edge->used || cells[i].count != cells[i-1U].count)) {
				cells[i].box_edge->used = 1;
				if (!mp_multiply(&mp_product, cells[i].product)) {
					mp_copy(&mp_tmp, &mp_product);
					mp_free(&mp_tmp);
					return;
				}
				tried_cells[dimension] = cells+i;
				crate_packing(dimension+1U);
				mp_copy(&mp_tmp, &mp_product);
				cells[i].box_edge->used = 0;
			}
		}
	}
	else {
		if (!mp_copy(&mp_product, &mp_product_max) || !mp_create(&mp_tmp, 1U)) {
			return;
		}
		for (i = 0U; i < dimensions_n && mp_multiply(&mp_tmp, tried_cells[i]->count); ++i);
		if (i < dimensions_n) {
			mp_free(&mp_tmp);
			return;
		}
		mp_print(&mp_tmp);
		fflush(stdout);
	}
	mp_free(&mp_tmp);
}
