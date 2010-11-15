#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "common-filters.c"
#include "cheby-table.h"

double num[20], den[20];

static
int test_chebyshev(struct coeffentry* entry)
{
	unsigned int i;
	double dnum, dden;

	if (!compute_cheby_iir(num, den,
	                       entry->npole,
	                       entry->highpass,
	                       entry->r,
	                       entry->fc))
		return -1;

	for (i=0; i<entry->npole+1; i++) {
		dnum = fabs(num[i] - entry->num[i]) / fabs(num[i]);
		dden = fabs(den[i] - entry->den[i]) / fabs(den[i]);
		if (dnum > 1e-5 || dden > 1e-5)
			return -1;
	}
	
	return 0;
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	unsigned int it;

	for (it=0; it<NUMCASE; it++)
		if (test_chebyshev(&(chebytable[it])))
			return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

