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
	/* global value for epsilon */
	double eps;
	/* fraction of epsilon for first step */
	double eps_share;
	/* max items in generation step */
	size_t mis;
	/* number of trees (0 == no tree) */
	size_t nt;
	/* number of rules to extract */
	size_t k;
	/* min alpha value */
	double minalpha;
	/* random seed */
	long int seed;
} args;

static void usage(const char *prg)
{
	fprintf(stderr, "Usage: %s TFILE EPS EPS_SHARE MINALPHA MIS NT K [SEED]\n", prg);
	exit(EXIT_FAILURE);
}

static void parse_arguments(int argc, char **argv)
{
	int i;

	printf("Called with: argc=%d\n", argc);
	for (i = 0; i < argc; i++)
		printf("%s ", argv[i]);
	printf("\n");

	if (argc < 8 || argc > 9)
		usage(argv[0]);
	args.tfname = strdup(argv[1]);

	if (sscanf(argv[2], "%lf", &args.eps) != 1 || args.eps < 0)
		usage(argv[0]);
	if (sscanf(argv[3], "%lf", &args.eps_share) != 1 || args.eps_share < 0 || args.eps_share >= 1)
		usage(argv[0]);
	if (sscanf(argv[4], "%lf", &args.minalpha) != 1)
		usage(argv[0]);
	if (sscanf(argv[5], "%lu", &args.mis) != 1 || args.mis < 2 || args.mis > 7)
		usage(argv[0]);
	if (sscanf(argv[6], "%lu", &args.nt) != 1)
		usage(argv[0]);
	if (sscanf(argv[7], "%lu", &args.k) != 1)
		usage(argv[0]);
	if (argc == 9) {
		if (sscanf(argv[8], "%ld", &args.seed) != 1)
			usage(argv[0]);
	} else
		args.seed = 42;
}

int main(int argc, char **argv)
{
	struct fptree fp;

	parse_arguments(argc, argv);

	fpt_read_from_file(args.tfname, &fp);
	printf("data-struct: nodes: %lu, edges: %lu, transactions: %lu\n",
			fp.n, fp.e, fp.t);

	dp2d(&fp, args.eps, args.eps_share, args.mis,
			args.nt, args.k, args.minalpha, args.seed);

	fpt_cleanup(&fp);
	free(args.tfname);

	return 0;
}
