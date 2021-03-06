/**
 * Global functions and utilities.
 */

#ifndef _GLOBALS_H
#define _GLOBALS_H

#define die(s, ...) \
	do {\
		fprintf(stderr, "[%s: %s %d] "s"\n", __FILE__, \
				__func__, __LINE__, ##__VA_ARGS__); \
		exit(EXIT_FAILURE); \
	} while (0)

#define max(a, b) \
	({\
		__typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; \
	})
#define min(a, b) \
	({\
		__typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a < _b ? _a : _b; \
	})

struct drand48_data;

/* qsort functions for integer comparisons */
int int_cmp(const void *a, const void *b);
int int_cmp_r(const void *a, const void *b);
int double_cmp(const void *a, const void *b);
int double_cmp_r(const void *a, const void *b);

void init_rng(long int seed, struct drand48_data *buffer);

/* Laplace mechanism */
double laplace_mechanism(double x, double eps, double sens,
		struct drand48_data *buffer);

/* version of bsearch which returns the rightmost insertion index
 * (the first index for which the element is at least equal to the key)
 */
int bsearch_i(const void *key, const void *base, size_t nmemb, size_t size,
		int (*compar)(const void *, const void *));

/**
 * Divides a by b and returns 0 instead of nan values.
 */
double div_or_zero(double a, double b);

#endif
