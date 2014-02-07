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

#include "fp.h"
#include "globals.h"

/* Command line arguments */
struct {
	/* filename containing the transactions */
	char *tfname;
} args;

static void usage(char *prg)
{
	fprintf(stderr, "Usage: %s TFILE\n", prg);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

static void parse_arguments(int argc, char **argv)
{
	/* TODO: use optparse */
	if (argc != 2)
		usage(argv[0]);
	args.tfname = strdup(argv[1]);
}

static void final_cleanup(void)
{
	free(args.tfname);
}

int main(int argc, char **argv)
{
	struct fptree fp;

	atexit(final_cleanup);
	parse_arguments(argc, argv);
	
	fpt_read_from_file(args.tfname, &fp);

	fpt_cleanup(&fp);

	return 0;
}