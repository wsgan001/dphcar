/**
 * Differentially-private high-confidence association rule extractor.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gmp.h>
#include <mpfr.h>

#include "dp2d.h"
#include "fp.h"
#include "globals.h"

/* Command line arguments */
static struct {
	/* filename containing the transactions */
	char *tfname;
#if 0 /* moving to graphs */
	/* binning mode */
	enum bin_mode bin_mode;
	/* number of bins */
	size_t bins;
	/* number of shelves */
	size_t shelves;
#endif
	/* global value for epsilon */
	double eps;
	/* fraction of epsilon for first step */
	double eps_share;
#if 0 /* moving to graphs */
	/* minimum threshold */
	size_t minth;
#endif
	/* max items in generation step */
	size_t mis;
	/* number of rules to extract */
	size_t k;
	/* min alpha value */
	double minalpha;
	/* random seed */
	long int seed;
} args;

static void usage(const char *prg)
{
#if 0 /* moving to graphs */
	fprintf(stderr, "Usage: %s TFILE BIN_MODE(n|r|w|d) NUM_BINS NUM_SHELVES EPS EPS_SHARE MINTH MINALHPA MIS K [SEED]\n", prg);
#else
	fprintf(stderr, "Usage: %s TFILE EPS EPS_SHARE MINALPHA MIS K [SEED]\n", prg);
#endif
	exit(EXIT_FAILURE);
}

static void parse_arguments(int argc, char **argv)
{
	int i;

	printf("Called with: argc=%d\n", argc);
	for (i = 0; i < argc; i++)
		printf("%s ", argv[i]);
	printf("\n");

#if 0 /* moving to graphs */
	if (argc < 11 || argc > 12)
#else
	if (argc < 7 || argc > 8)
#endif
		usage(argv[0]);
	args.tfname = strdup(argv[1]);
#if 0 /* moving to graphs */
	if (!strncmp(argv[2], "n", 1)) args.bin_mode = NONE;
	else if (!strncmp(argv[2], "r", 1)) args.bin_mode = RANDOM;
	else if (!strncmp(argv[2], "w", 1)) args.bin_mode = EQUIWIDTH;
	else if (!strncmp(argv[2], "d", 1)) args.bin_mode = EQUIDENSITY;
	else usage(argv[0]);
	if (sscanf(argv[3], "%lu", &args.bins) != 1)
		usage(argv[0]);
	if (args.bins < 1)
		die("Must have at least one bin!");
	if (args.bin_mode == NONE && args.bins > 1)
		die("NONE binning requires exactly 1 bin!");
	if (sscanf(argv[4], "%lu", &args.shelves) != 1)
		usage(argv[0]);
	if (args.shelves != 1 && args.bin_mode != RANDOM)
		die("Only RANDOM binning can have more than 1 shelf!");
#endif
#if 0 /* moving to graphs */
	if (sscanf(argv[5], "%lf", &args.eps) != 1 || args.eps < 0)
#else
	if (sscanf(argv[2], "%lf", &args.eps) != 1 || args.eps < 0)
#endif
		usage(argv[0]);
#if 0 /* moving to graphs */
	if (sscanf(argv[6], "%lf", &args.eps_share) != 1 || args.eps_share < 0 || args.eps_share >= 1)
#else
	if (sscanf(argv[3], "%lf", &args.eps_share) != 1 || args.eps_share < 0 || args.eps_share >= 1)
#endif
		usage(argv[0]);
#if 0 /* moving to graphs */
	if (sscanf(argv[7], "%lu", &args.minth) != 1)
		usage(argv[0]);
#endif
#if 0 /* moving to graphs */
	if (sscanf(argv[8], "%lf", &args.minalpha) != 1)
#else
	if (sscanf(argv[4], "%lf", &args.minalpha) != 1)
#endif
		usage(argv[0]);
#if 0 /* moving to graphs */
	if (sscanf(argv[9], "%lu", &args.mis) != 1 || args.mis < 2 || args.mis > 7)
#else
	if (sscanf(argv[5], "%lu", &args.mis) != 1 || args.mis < 2 || args.mis > 7)
#endif
		usage(argv[0]);
#if 0 /* moving to graphs */
	if (sscanf(argv[10], "%lu", &args.k) != 1)
#else
	if (sscanf(argv[6], "%lu", &args.k) != 1)
#endif
		usage(argv[0]);
#if 0 /* moving to graphs */
	if (argc == 12) {
		if (sscanf(argv[11], "%ld", &args.seed) != 1)
#else
	if (argc == 8) {
		if (sscanf(argv[7], "%ld", &args.seed) != 1)
#endif
			usage(argv[0]);
	} else
		args.seed = 42;
}

int main(int argc, char **argv)
{
	struct fptree fp;

	parse_arguments(argc, argv);

	fpt_read_from_file(args.tfname, &fp);
#if 0 /* moving to graphs */
	printf("fp-tree: items: %lu, transactions: %lu, nodes: %d, depth: %d\n",
			fp.n, fp.t, fpt_nodes(&fp), fpt_height(&fp));
#else
	printf("data-struct: nodes: %lu, edges: %lu, transactions: %lu\n",
			fp.n, fp.e, fp.t);
#endif

#if 0 /* moving to graphs */
	dp2d(&fp, args.shelves, args.bins, args.bin_mode,
			args.eps, args.eps_share,
			args.minth, args.mis, args.k, args.minalpha,
			args.seed);
#else
	dp2d(&fp, args.eps, args.eps_share, args.mis, args.k,
			args.minalpha, args.seed);
#endif

	fpt_cleanup(&fp);
	free(args.tfname);

	return 0;
}
