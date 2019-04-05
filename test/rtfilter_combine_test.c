#include <check.h>
#include <complex.h>
#include <math.h>
#include <rtfilter.h>
#include <stdlib.h>
#include <string.h>

#include "rtfilter.h"
#include "testcases.h"
#include "test-common.h"

#define NCH 4
#define NS 128
#define NUM_FILTERS 2

START_TEST(test_rtfilter_combine_smoke)
{
	int i;
	struct rtf_filter * filters[NUM_FILTERS];
	struct rtf_filter * f;
	double num[3] = {0., 1., 2.};
	double denum[4] = {3., 4., 5., 6.};

	for (i = 0; i < arraylen(filters); i++) {
		filters[i] = rtf_create_filter(NCH, RTF_FLOAT,
		                               arraylen(num), num,
		                               arraylen(denum), denum,
		                               RTF_DOUBLE);
		ck_assert(filters[i] != NULL);
	}

	f = rtf_filter_combine(NUM_FILTERS, filters);
	ck_assert(f != NULL);

	smoke_test(f, NCH, NS);

	rtf_destroy_filter(f);
}
END_TEST


START_TEST(test_rtfilter_combine_flat)
{
	int i;
	struct rtf_filter * filters[NUM_FILTERS];
	struct rtf_filter * f;
	double num[3] = {0., 1., 2.};
	double denum[4] = {3., 4., 5., 6.};

	for (i = 0; i < arraylen(filters); i++) {
		filters[i] = rtf_create_filter(NCH, RTF_FLOAT,
		                               arraylen(num), num,
		                               arraylen(denum), denum,
		                               RTF_DOUBLE);
		ck_assert(filters[i] != NULL);
	}

	f = rtf_filter_combine(NUM_FILTERS, filters);
	ck_assert(f != NULL);

	flat_test(f, NCH, NS);

	rtf_destroy_filter(f);
}
END_TEST


TCase* create_rtfilter_combine_tcase(void)
{
	TCase * tc;

	tc = tcase_create("rtfilter-combine");
	tcase_add_test(tc, test_rtfilter_combine_flat);
	tcase_add_test(tc, test_rtfilter_combine_smoke);

	return tc;
}
