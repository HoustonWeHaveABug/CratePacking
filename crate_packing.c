#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define DIMENSIONS_MIN 2
#define MP_SIZE 2

typedef struct node_s node_t;

struct node_s {
	long edge;
	node_t **last;
	node_t **next;
};

int compare_crate_edges(const void *, const void *);
int compare_box_edges(const void *, const void *);
void link_node(node_t *, node_t *, node_t *);
long *mp_create(long);
void crate_packing(long, long);
int mp_copy(long *, long **, long, long, long);
int mp_val_multiply(long *, long **, long, long);
int mp_compare(long *, long, long *);
int mp_max_set(long *, long, long, long **);

int p_len;
long dimensions, *crate, dimension, p_max, *q_max, q_stack_size, *q_stack, *v_max, v_stack_size, *v_stack;
node_t *box, *header;

int main(void) {
	long *index, i;
	node_t **links;
	if (scanf("%ld", &dimensions) != 1 || dimensions < DIMENSIONS_MIN) {
		fprintf(stderr, "Invalid number of dimension\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	crate = malloc(sizeof(long)*(size_t)dimensions);
	if (!crate) {
		fprintf(stderr, "Could not allocate memory for crate\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	for (i = 0; i < dimensions; i++) {
		if (scanf("%ld", crate+i) != 1 || crate[i] < 1) {
			fprintf(stderr, "Invalid crate edge\n");
			fflush(stderr);
			free(crate);
			return EXIT_FAILURE;
		}
	}
	qsort(crate, (size_t)dimensions, sizeof(long), compare_crate_edges);
	box = malloc(sizeof(node_t)*(size_t)(dimensions+1));
	if (!box) {
		fprintf(stderr, "Could not allocate memory for box\n");
		fflush(stderr);
		free(crate);
		return EXIT_FAILURE;
	}
	for (i = 0; i < dimensions; i++) {
		if (scanf("%ld", &box[i].edge) != 1 || box[i].edge < 1) {
			fprintf(stderr, "Invalid box edge\n");
			fflush(stderr);
			free(box);
			free(crate);
			return EXIT_FAILURE;
		}
	}
	box[i].edge = 0;
	links = malloc(sizeof(node_t *)*(size_t)(dimensions*dimensions*2+dimensions*2));
	if (!links) {
		fprintf(stderr, "Could not allocate memory for links\n");
		fflush(stderr);
		free(box);
		free(crate);
		return EXIT_FAILURE;
	}
	for (i = 0; i <= dimensions; i++) {
		box[i].last = links+i*dimensions*2;
		box[i].next = box[i].last+dimensions;
	}
	index = malloc(sizeof(long)*(size_t)(dimensions+1));
	if (!index) {
		fprintf(stderr, "Could not allocate memory for index\n");
		fflush(stderr);
		free(links);
		free(box);
		free(crate);
		return EXIT_FAILURE;
	}
	index[dimensions] = dimensions;
	for (dimension = 0; dimension < dimensions; dimension++) {
		for (i = 0; i < dimensions; i++) {
			index[i] = i;
		}
		qsort(index, (size_t)dimensions, sizeof(long), compare_box_edges);
		link_node(box+index[0], box+index[dimensions], box+index[1]);
		for (i = 1; i < dimensions; i++) {
			link_node(box+index[i], box+index[i-1], box+index[i+1]);
		}
		link_node(box+index[i], box+index[i-1], box+index[0]);
	}
	header = box+dimensions;
	p_len = 0;
	p_max = 1;
	while (p_max*10 <= SHRT_MAX) {
		p_len++;
		p_max *= 10;
	}
	q_max = mp_create(0);
	if (!q_max) {
		free(index);
		free(links);
		free(box);
		free(crate);
		return EXIT_FAILURE;
	}
	q_stack_size = MP_SIZE;
	q_stack = mp_create(1);
	if (!q_stack) {
		free(q_max);
		free(index);
		free(links);
		free(box);
		free(crate);
		return EXIT_FAILURE;
	}
	v_max = mp_create(0);
	if (!v_max) {
		free(q_stack);
		free(q_max);
		free(index);
		free(links);
		free(box);
		free(crate);
		return EXIT_FAILURE;
	}
	v_stack_size = MP_SIZE;
	v_stack = mp_create(1);
	if (!v_stack) {
		free(v_max);
		free(q_stack);
		free(q_max);
		free(index);
		free(links);
		free(box);
		free(crate);
		return EXIT_FAILURE;
	}
	dimension = 0;
	crate_packing(0, 0);
	free(v_stack);
	free(v_max);
	free(q_stack);
	free(q_max);
	free(index);
	free(links);
	free(box);
	free(crate);
	return EXIT_SUCCESS;
}

int compare_crate_edges(const void *a, const void *b) {
	const long *edge_a = (const long *)a, *edge_b = (const long *)b;
	if (*edge_a < *edge_b) {
		return -1;
	}
	if (*edge_a > *edge_b) {
		return 1;
	}
	return 0;
}

int compare_box_edges(const void *a, const void *b) {
	long div_a, div_b, mod_a, mod_b;
	const long *index_a = (const long *)a, *index_b = (const long *)b;
	div_a = crate[dimension]/box[*index_a].edge;
	div_b = crate[dimension]/box[*index_b].edge;
	if (div_a == 0) {
		if (div_b == 0) {
			return 0;
		}
		return 1;
	}
	if (div_b == 0) {
		return -1;
	}
	mod_a = crate[dimension]%box[*index_a].edge;
	mod_b = crate[dimension]%box[*index_b].edge;
	if (mod_a < mod_b) {
		return -1;
	}
	if (mod_a > mod_b) {
		return 1;
	}
	if (div_a < div_b) {
		return 1;
	}
	if (div_a > div_b) {
		return -1;
	}
	return 0;
}

void link_node(node_t *node, node_t *last, node_t *next) {
	node->last[dimension] = last;
	node->next[dimension] = next;
}

long *mp_create(long val) {
	long *mp = malloc(sizeof(long)*MP_SIZE);
	if (!mp) {
		fprintf(stderr, "Could not allocate memory for mp\n");
		fflush(stderr);
		return NULL;
	}
	mp[0] = MP_SIZE;
	mp[1] = val;
	return mp;
}

void crate_packing(long q_last, long v_last) {
	long q_size = q_stack[q_last], v_size = v_stack[v_last], i;
	if (dimension < dimensions) {
		long q_next = q_last+q_size, v_next = v_last+v_size;
		node_t *node;
		if (!mp_copy(&v_stack_size, &v_stack, v_last, v_next, v_size)) {
			return;
		}
		for (i = dimension; i < dimensions; i++) {
			if (crate[i]/header->next[i]->edge == 0) {
				return;
			}
			if (!mp_val_multiply(&v_stack_size, &v_stack, v_next, crate[i]-crate[i]%header->next[i]->edge)) {
				return;
			}
			if (mp_compare(v_stack, v_next, v_max) > 0) {
				break;
			}
		}
		if (i == dimensions) {
			return;
		}
		for (node = header->next[dimension]; node != header && crate[dimension]/node->edge > 0; node = node->next[dimension]) {
			if (node->edge != node->last[dimension]->edge) {
				mp_copy(&q_stack_size, &q_stack, q_last, q_next, q_size);
				mp_val_multiply(&q_stack_size, &q_stack, q_next, crate[dimension]/node->edge);
				mp_copy(&v_stack_size, &v_stack, v_last, v_next, v_size);
				mp_val_multiply(&v_stack_size, &v_stack, v_next, crate[dimension]-crate[dimension]%node->edge);
				for (i = dimension; i < dimensions; i++) {
					node->next[i]->last[i] = node->last[i];
					node->last[i]->next[i] = node->next[i];
				}
				dimension++;
				crate_packing(q_next, v_next);
				dimension--;
				for (i = dimension; i < dimensions; i++) {
					node->last[i]->next[i] = node;
					node->next[i]->last[i] = node;
				}
			}
		}
	}
	else {
		if (!mp_max_set(q_stack, q_last, q_size, &q_max)) {
			return;
		}
		for (i = q_size-1; i > 0; i--) {
			printf("%0*ld", p_len, q_max[i]);
		}
		puts("");
		fflush(stdout);
		if (!mp_max_set(v_stack, v_last, v_size, &v_max)) {
			return;
		}
	}
}

int mp_copy(long *mp_stack_size, long **mp_stack, long mp_idx_a, long mp_idx_b, long mp_size) {
	long i;
	if (*mp_stack_size < mp_idx_b+mp_size) {
		long *mp_stack_tmp = realloc(*mp_stack, sizeof(long)*(size_t)(mp_idx_b+mp_size));
		if (!mp_stack_tmp) {
			fprintf(stderr, "Could not reallocate_memory for *mp_stack\n");
			fflush(stderr);
			return 0;
		}
		*mp_stack_size = mp_idx_b+mp_size;
		*mp_stack = mp_stack_tmp;
	}
	for (i = 0; i < mp_size; i++) {
		(*mp_stack)[mp_idx_b+i] = (*mp_stack)[mp_idx_a+i];
	}
	return 1;
}

int mp_val_multiply(long *mp_stack_size, long **mp_stack, long mp_idx, long val) {
	long val_tmp = val, offset = 0, mp_size = (*mp_stack)[mp_idx], product_size, *product, carry, i;
	while (val_tmp > 0) {
		val_tmp /= p_max;
		offset++;
	}
	product_size = mp_size-1+offset;
	product = calloc((size_t)product_size, sizeof(long));
	if (!product) {
		fprintf(stderr, "Could not allocate_memory for product\n");
		fflush(stderr);
		return 0;
	}
	if (*mp_stack_size < mp_idx+1+product_size) {
		long *mp_stack_tmp = realloc(*mp_stack, sizeof(long)*(size_t)(mp_idx+1+product_size));
		if (!mp_stack_tmp) {
			fprintf(stderr, "Could not reallocate_memory for *mp_stack\n");
			fflush(stderr);
			free(product);
			return 0;
		}
		*mp_stack_size = mp_idx+1+product_size;
		*mp_stack = mp_stack_tmp;
	}
	offset = 0;
	while (val > 0) {
		val_tmp = val%p_max;
		for (i = 1; i < mp_size; i++) {
			product[i-1+offset] += (*mp_stack)[mp_idx+i]*val_tmp;
		}
		val /= p_max;
		offset++;
	}
	carry = product[0]/p_max;
	product[0] %= p_max;
	for (i = 1; i < product_size; i++) {
		product[i] += carry;
		carry = product[i]/p_max;
		product[i] %= p_max;
	}
	for (i = product_size; i > 0 && product[i-1] == 0; i--) {
		product_size--;
	}
	(*mp_stack)[mp_idx] = product_size+1;
	for (i = 1; i <= product_size; i++) {
		(*mp_stack)[mp_idx+i] = product[i-1];
	}
	free(product);
	return 1;
}

int mp_compare(long *mp_stack, long mp_idx, long *mp_max) {
	long i;
	if (mp_stack[mp_idx] < mp_max[0]) {
		return -1;
	}
	if (mp_stack[mp_idx] > mp_max[0]) {
		return 1;
	}
	for (i = mp_max[0]-1; i > 0 && mp_stack[mp_idx+i] == mp_max[i]; i--);
	if (i == 0) {
		return 0;
	}
	if (mp_stack[mp_idx+i] < mp_max[i]) {
		return -1;
	}
	return 1;
}

int mp_max_set(long *mp_stack, long mp_idx, long mp_size, long **mp_max) {
	int i;
	if (**mp_max < mp_size) {
		long *mp_max_tmp = realloc(*mp_max, sizeof(long)*(size_t)mp_size);
		if (!mp_max_tmp) {
			fprintf(stderr, "Could not reallocate_memory for mp_max\n");
			fflush(stderr);
			return 0;
		}
		*mp_max = mp_max_tmp;
	}
	for (i = 0; i < mp_size; i++) {
		(*mp_max)[i] = mp_stack[mp_idx+i];
	}
	return 1;
}
