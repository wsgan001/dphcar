#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <gmp.h>
#include <mpfr.h>

#include "dp2d.h"
#include "fp.h"
#include "globals.h"
#include "histogram.h"
#include "rule.h"

#define MICROSECONDS 1000000L

/* print the noisy counts for each item */
#ifndef PRINT_ITEM_TABLE
#define PRINT_ITEM_TABLE 0
#endif
/* print the rule lattice generation step debug info */
#ifndef PRINT_RULE_LATTICE
#define PRINT_RULE_LATTICE 1
#endif
#if 0
/* print the rules generated at each step and their quality */
#ifndef PRINT_RULE_DOMAIN
#define PRINT_RULE_DOMAIN 0
#endif
/* print actions to the reservoir */
#ifndef PRINT_RS_TRACE
#define PRINT_RS_TRACE 0
#endif
/* print the returned rules */
#ifndef PRINT_FINAL_RULES
#define PRINT_FINAL_RULES 0
#endif
/* do rule expansion */
#ifndef RULE_EXPAND
#define RULE_EXPAND 0
#endif
#endif

static double quality(int x, int y, double c0)
{
	double q = x - y / c0;
	if (q < 0) return 0;
	return q;
}

struct item_count {
	int value;
	int real_count;
	double noisy_count;
};

struct reservoir {
	struct rule *r;
	double c;
	double v;
	int sxy;
	int sx;
};

#if 0
static void free_reservoir_array(struct reservoir *reservoir, int size)
{
	int i;

	for (i = 0 ; i < size; i++)
		free_rule(reservoir[i].r);
	free(reservoir);
}
#endif

static int ic_noisy_cmp(const void *a, const void *b)
{
	const struct item_count *ia = a, *ib = b;
	return double_cmp_r(&ia->noisy_count, &ib->noisy_count);
}

#if 0
static int reservoir_cmp(const void *a, const void *b)
{
	const struct reservoir *ra = a, *rb = b;
	return double_cmp(&ra->v, &rb->v);
}
#endif

static void build_items_table(const struct fptree *fp, struct item_count *ic,
		double eps, struct drand48_data *buffer)
{
	size_t i;

	for (i = 0; i < fp->n; i++) {
		ic[i].value = i + 1;
		ic[i].real_count = fpt_item_count(fp, i);
		ic[i].noisy_count = laplace_mechanism(ic[i].real_count,
				eps, 1, buffer);
		/* TODO: filter some noise */
		if (ic[i].noisy_count < 0)
			ic[i].noisy_count = 0;
	}

	qsort(ic, fp->n, sizeof(ic[0]), ic_noisy_cmp);
}

#if PRINT_RS_TRACE || PRINT_FINAL_RULES
static void print_reservoir(struct reservoir *reservoir, size_t rs)
{
	size_t i;

	for (i = 0; i < rs; i++) {
		printf("\t%lu\t%3.2lf\t% 11.5lf\t%7d\t%7d | ", i,
				reservoir[i].c, reservoir[i].v,
				reservoir[i].sxy, reservoir[i].sx);
		print_rule(reservoir[i].r);
		printf("\n");
	}
}
#endif

#if 0
static void process_rule(const struct fptree *fp,
		const int *AB, int ab_length, const int *A, int a_length,
		double eps, size_t *rs, struct reservoir *reservoir, size_t k,
		double u, double m)
{
	struct itemset *iA, *iAB;
	struct rule *r = NULL;
	int sup_a, sup_ab;
	double q, v, c;

	sup_a = fpt_itemset_count(fp, A, a_length);
	sup_ab = fpt_itemset_count(fp, AB, ab_length);
	iA = build_itemset(A, a_length);
	iAB = build_itemset(AB, ab_length);
	r = build_rule_A_AB(iA, iAB);
	q = quality(sup_a, sup_ab, m);
	v = log(log(1/u)) - eps * q / 2;
	c = (sup_ab + 0.0) / (sup_a + 0.0);

#if PRINT_RULE_DOMAIN || PRINT_RS_TRACE
	printf("\tRule: ");
	print_rule(r);
	printf(" %d/%d (%3.2lf) %5.4lf %5.4lf %5.4lf\n",
			sup_a, sup_ab, c, q, u, v);
#endif

	if (*rs < k) {
		reservoir[*rs].r = r;
		reservoir[*rs].v = v;
		reservoir[*rs].c = c;
		reservoir[*rs].sxy = sup_ab;
		reservoir[*rs].sx = sup_a;
		*rs = *rs + 1;

		if (*rs == k) {
			qsort(reservoir, k, sizeof(reservoir[0]), reservoir_cmp);
#if PRINT_RS_TRACE
			printf("Initial reservoir:\n");
			print_reservoir(reservoir, *rs);
#endif
		}
	} else if (v < reservoir[k-1].v) {
		free_rule(reservoir[k-1].r);
		reservoir[k-1].r = r;
		reservoir[k-1].v = v;
		reservoir[k-1].c = c;
		reservoir[k-1].sxy = sup_ab;
		reservoir[k-1].sx = sup_a;
		qsort(reservoir, k, sizeof(reservoir[0]), reservoir_cmp);
#if PRINT_RS_TRACE
		printf("Inserted into reservoir, now:\n");
		print_reservoir(reservoir, *rs);
#endif
	} else
		free_rule(r);

	free_itemset(iA);
	free_itemset(iAB);
}
#endif

#if 0
static void generate_and_add_all_rules(const struct fptree *fp,
		const int *items, size_t num_items, size_t st, double eps,
		size_t *rs, struct reservoir *reservoir,
		size_t k, struct drand48_data *randbuffer, double m)
{
	size_t i, j, l, max=1<<num_items, a_length, ab_length, max2;
	int *A, *AB;
	double u;

	A = calloc(num_items, sizeof(A[0]));
	AB = calloc(num_items, sizeof(AB[0]));

	/* generate rule's AB */
	for (i = st; i < max; i++) {
		ab_length = 0;
		for (j = 0; j < num_items; j++)
			if (i & (1 << j))
				AB[ab_length++] = items[j];
		if (ab_length < 2) continue;

		max2 = (1 << ab_length) - 1;
		for (j = 1; j < max2; j++) {
			a_length = 0;
			for (l = 0; l < ab_length; l++)
				if (j & (1 << l))
					A[a_length++] = AB[l];
			drand48_r(randbuffer, &u);
			process_rule(fp, AB, ab_length, A, a_length,
					eps, rs, reservoir, k, u, m);
		}
	}

	free(A);
	free(AB);
}

static void split_in_partitions(const struct fptree *fp,
		const struct item_count *ic, size_t bins, enum bin_mode bin_mode,
		int minth,
		size_t **parts, size_t *parlens,
		struct drand48_data *randbuffer)
{
	size_t i, j, k, c;
	size_t *items;

	/* compute how many items there are */
	for (c = 0; c < fp->n;)
		if (ic[c++].noisy_count < minth)
			break;
	printf("Items above threshold: %lu\n", c);

	/* determine count of items in each bin */
	if (bin_mode == EQUIWIDTH) {
		size_t *thresholds = calloc(bins, sizeof(thresholds[0]));
		size_t delta = (ic[0].noisy_count - minth)/bins;
		size_t t = ic[0].noisy_count;

		/* compute thresholds */
		for (i = 0; i < bins; i++) {
			t-=delta;
			thresholds[i] = t;
		}
		thresholds[bins-1] = minth;

		/* get counts */
		i = 0;
		for (j = 0, k = 0; j < c; j++,k++)
			if (ic[j].noisy_count < thresholds[i]) {
				parlens[i++] = k;
				k = 0;
			}

		free(thresholds);
	} else {
		for (i = 0; i < bins; i++)
			parlens[i] = c/bins;
		parlens[bins-1] += c%bins;
	}

	items = calloc(c, sizeof(*items));
	for (i = 0; i < c; i++)
		items[i] = i;

	/* RANDOM needs a shuffle of all items in ic */
	if (bin_mode == RANDOM) {
		size_t t;
		double rnd;

		for(i = c - 1; i; i--) {
			drand48_r(randbuffer, &rnd);
			j = i * rnd;
			t = items[j];
			items[j] = items[i];
			items[i] = t;
		}
	}

	/* distribute items */
	j = 0;
	for (i = 0; i < bins; i++) {
		parts[i] = calloc(parlens[i], sizeof(parts[i][0]));
		for (k = 0; k < parlens[i]; k++)
			parts[i][k] = items[j++];
	}

	free(items);
}
#endif

/**
 * Analyze the current items to see if we can select a good rule lattice.
 */
static void analyze_items(size_t *items, size_t lmax, const struct fptree *fp,
		const struct item_count *ic, double c0, double eps,
		struct drand48_data *randbuffer)
{
	int *AB = calloc(lmax, sizeof(AB[0]));
	int sup_ab, sup_a;
	double q, u, v;
	size_t i;

	for (i = 0; i < lmax; i++)
		AB[i] = ic[items[i]].value;

	sup_ab = fpt_itemset_count(fp, AB, lmax);

#if PRINT_RULE_LATTICE
	printf("Analyzing new set of items: ");
	for (i = 0; i < lmax; i++)
		printf("%3d ", AB[i]);
	printf(" | support: %d\n", sup_ab);
#endif

	/* try for each corner item */
	for (i = 0; i < lmax; i++) {
		sup_a = ic[items[i]].real_count;
		q = quality(sup_a, sup_ab, c0);
		drand48_r(randbuffer, &u);
		v = log(log(1/u)) - eps * q / 2;

#if PRINT_RULE_LATTICE
		printf("\t%3d -> {}: c=%7.6f q=%5.2f u=%5.2f v=%5.2f\n",
				AB[i], (sup_ab + 0.0)/sup_a, q, u, v);
#endif
	}

	free(AB);
}

/**
 * Constructs the next items vector, the next set of rules to be analyzed.
 */
static int update_items(size_t *items, size_t lmax, size_t n)
{
	size_t ix = lmax - 1, ok;

	do {
		do {
			/* try next */
			ok = 1;
			items[ix]++;

			/* if impossible, move to one below */
			if (items[ix] == n) {
				if (!ix) return 1; /* end of generation */
				ix--;
				ok = 0; /* this level was not finished */
				break; /* restart process for ix - 1 */
			}

			/* TODO: check to not generate forbidden set */
		} while (!ok);

		if (ok) {
			ix++; /* move to next */
			if (ix == lmax)
				return 0; /* all generated */
			items[ix] = items[ix - 1]; /* before first option */
		}
	} while (ix < lmax);

	return 0; /* notreached */
}

/**
 * Initialize the items vector, the first set of rules to be analyzed.
 */
static inline void init_items(size_t *items, size_t lmax)
{
	size_t i;

	for (i = 0; i < lmax; i++)
		items[i] = i;
}

#if 0
void dp2d(const struct fptree *fp,
		size_t shelves, size_t bins, enum bin_mode bin_mode,
		double eps, double eps_share, int minth, size_t mis, size_t k,
		double minalpha, long int seed)
#else
void dp2d(const struct fptree *fp, double eps, double eps_ratio1,
		double c0, size_t lmax, size_t k, long int seed)
#endif
{
#if 0
	size_t *ksh = calloc(shelves, sizeof(ksh[0]));
	size_t **partitions = NULL, *parlens = NULL;
	struct histogram *h = init_histogram();
	size_t i, j, ip, fm, rs, st, cis, sh;
	double maxc, minc;
#endif
	struct item_count *ic = calloc(fp->n, sizeof(ic[0]));
	double epsilon_step1 = eps * eps_ratio1;
	struct drand48_data randbuffer;
	size_t i, end;
	size_t *items;

	init_rng(seed, &randbuffer);
	items = calloc(lmax, sizeof(items[0]));

	printf("Running dp2D with eps=%lf, eps_step1=%lf, k=%lu, c0=%5.2lf, "
			"rmax=%lu\n", eps, epsilon_step1, k, c0, lmax);

	printf("Step 1: compute noisy counts for items with eps_1 = %lf\n",
			epsilon_step1);
	build_items_table(fp, ic, epsilon_step1, &randbuffer);

#if PRINT_ITEM_TABLE
	printf("\n");
	for (i = 0; i < fp->n; i++)
		printf("%d %d %lf\n", ic[i].value, ic[i].real_count, ic[i].noisy_count);
#endif

	eps = eps - epsilon_step1;
	eps /= k;
	printf("Step 2: mining %lu steps each with eps %lf\n", k, eps);

#if 0
	eps /= shelves;
	printf("\t-> per shelf:\t%lf\n", eps);

	for (sh = 0; sh < shelves; sh++)
		ksh[sh] = (k / shelves) + ((k % shelves) > sh);

	printf("\t-> rules per shelf:");
	for (sh = 0; sh < shelves; sh++)
		printf(" %lu", ksh[sh]);
	printf("\n");
#endif

	struct timeval starttime;
	gettimeofday(&starttime, NULL);

	for (i = 0; i < k; i++) {
		init_items(items, lmax);
		do {
			analyze_items(items, lmax, fp, ic, c0, eps,
					&randbuffer);

			/* TODO: last arg would be fp->n or a constant that is
			 * determined by the code, not a fixed argument */
			end = update_items(items, lmax, 20);
		} while (!end);
		break; /* TODO: remove */
	}
#if 0
	for (sh = 0; sh < shelves; sh++) {
		for (i = 0; i < bins && partitions; i++)
			free(partitions[i]);
		free(partitions);
		free(parlens);

		partitions = calloc(bins, sizeof(partitions[0]));
		parlens = calloc(bins, sizeof(parlens[0]));
		split_in_partitions(fp, ic, bins, bin_mode, minth,
				partitions, parlens, &randbuffer);
		printf("Partitions: %lu |", bins);
		for (i = 0; i < bins; i++)
			printf(" %lu", parlens[i]);
		printf("\n");

		for (ip = 0; ip < bins; ip++) {
			size_t k_now = ksh[sh];
			k_now = (k_now / bins) + ((k_now % bins) > ip);

			/* if no items just move away */
			if (!parlens[ip])
				continue;
			cis = mis;

			/* select mining domains */
			struct reservoir *reservoir = calloc(k_now, sizeof(reservoir[0]));
			rs = 0; /* empty reservoir */
			st = 0;

			/* initial items */
			for (fm = 0, j = 0; j < cis && fm < parlens[ip]; fm++)
				items[j++] = ic[partitions[ip][fm]].value;

			if (j < cis)
				cis = j;

			while (1) {
#if PRINT_RULE_DOMAIN || PRINT_RS_TRACE
				printf("Domain: %lu: ", fm);
				for (i = 0; i < cis; i++)
					printf("%d ", items[i]);
				printf("\n");
#endif

				generate_and_add_all_rules(fp, items, cis, st, eps/k_now,
						&rs, reservoir, k_now, &randbuffer, minalpha);
				st |= (1 << (cis - 1));

				if (fm == parlens[ip])
					break;
				for (i = 0; i < cis - 1; i++)
					items[i] = items[i+1];
				items[cis - 1] = ic[partitions[ip][fm++]].value;
			}

#if PRINT_FINAL_RULES
			print_reservoir(reservoir, rs);
#endif

			/* move rules from reservoir to histogram */
			minc = 1; maxc = 0;
#if RULE_EXPAND
			struct rule_table *rt = init_rule_table();
			for (i = 0; i < rs; i++) {
				struct rule *r = reservoir[i].r, *nr1, *nr2;
				int *items = calloc(r->B->length, sizeof(items[0]));
				struct itemset *A, *AB, *ABprime;
				int supA, supAB;
				size_t k1, nis;

				save_rule2(rt, r, reservoir[i].c);

				st = 1 << r->B->length; st--;
				AB = build_itemset_add_items(r->A, r->B->items, r->B->length);
				for (j = 1; j < st; j++) {
					nis = 0;
					for (k1 = 0; k1 < r->B->length; k1++)
						if ((1 << k1) & j)
							items[nis++] = r->B->items[k1];

					ABprime = build_itemset_del_items(AB, items, nis);
					nr1 = build_rule_A_AB(r->A, ABprime);
					supAB = fpt_itemset_count(fp, ABprime->items, ABprime->length);
					supA = fpt_itemset_count(fp, r->A->items, r->A->length);
					save_rule2(rt, nr1, supAB / (supA + 0.0));

					A = build_itemset_add_items(r->A, items, nis);
					nr2 = build_rule_A_AB(A, AB);
					supAB = fpt_itemset_count(fp, AB->items, AB->length);
					supA = fpt_itemset_count(fp, A->items, A->length);
					save_rule2(rt, nr2, supAB / (supA + 0.0));
				}
			}

			for (i = 0; i < rt->sz; i++) {
				if (rt->c[i] < minc)
					minc = rt->c[i];
				if (rt->c[i] > maxc)
					maxc = rt->c[i];
				histogram_register(h, rt->c[i]);
			}
#else
			for (i = 0; i < rs; i++) {
				if (reservoir[i].c < minc)
					minc = reservoir[i].c;
				if (reservoir[i].c > maxc)
					maxc = reservoir[i].c;
				histogram_register(h, reservoir[i].c);
			}
#endif

			printf("Rules saved: %lu, minconf: %3.2lf, maxconf: %3.2lf\n",
#if RULE_EXPAND
					rt->sz,
#else
					rs,
#endif
					minc, maxc);

			free_reservoir_array(reservoir, rs);
		}

		printf("Finished shelf %lu\n", sh);
	}
#endif

	struct timeval endtime;
	gettimeofday(&endtime, NULL);
	double t1 = starttime.tv_sec + (0.0 + starttime.tv_usec) / MICROSECONDS;
	double t2 = endtime.tv_sec + (0.0 + endtime.tv_usec) / MICROSECONDS;

	printf("Total time: %5.2lf\n", t2 - t1);
	printf("%ld %ld %ld %ld\n", starttime.tv_sec, starttime.tv_usec, endtime.tv_sec, endtime.tv_usec);

#if 0
	printf("Final histogram:\n");
	histogram_dump(stdout, h, 1, "\t");

	for (i = 0; i < bins; i++)
		free(partitions[i]);
	free(parlens);
	free(partitions);
	free_histogram(h);
	free(ksh);
#endif
	free(items);
	free(ic);
}
