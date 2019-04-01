/*
    Copyright (C) 2010-2011 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

    The program is free software: you can redistribute it and/or
    modify it under the terms of the version 3 of the GNU General
    Public License as published by the Free Software Foundation.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "cheby-table.h"
#include "common-filters.c"

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

