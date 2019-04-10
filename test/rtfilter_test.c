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

/* helpers to test different data types for rtfilter
 * Test the filter api itself throrougly , and test the higher-level
 * APIs (Eg. lowpass filter creation) with float only */

#define NANd __builtin_nanl("")
#define NANcf (NAN + I * NAN)
#define NANcd (NANd + I * NANd)

static int
finite_cf(complex float x)
{
	return isfinite(crealf(x)) && isfinite(cimagf(x));
}

static int
finite_cd(complex double x)
{
	return isfinite(creal(x)) && isfinite(cimag(x));
}


START_TEST(test_rtfilter_smoke_float)
{
	struct rtf_filter * f;
	double num[3] = {0., 1., 2.};
	double denum[4] = {3., 4., 5., 6.};

	f = rtf_create_filter(NCH, RTF_FLOAT,
	                      arraylen(num), num,
	                      arraylen(denum), denum,
	                      RTF_DOUBLE);
	ck_assert(f != NULL);

	smoke_test(f, NCH, NS);

	rtf_destroy_filter(f);
}
END_TEST


START_TEST(test_rtfilter_flat_float)
{
	struct rtf_filter * f;
	double num[3] = {0., 1., 2.};
	double denum[4] = {3., 4., 5., 6.};

	f = rtf_create_filter(NCH, RTF_FLOAT,
	                      arraylen(num), num,
	                      arraylen(denum), denum,
	                      RTF_DOUBLE);
	ck_assert(f != NULL);

	flat_test(f, NCH, NS);

	rtf_destroy_filter(f);
}
END_TEST


START_TEST(test_rtfilter_smoke_double)
{
	int i;
	double * in, * out;
	struct rtf_filter * f;
	double num[3] = {0., 1., 2.};
	double denum[4] = {3., 4., 5., 6.};

	f = rtf_create_filter(NCH, RTF_DOUBLE,
	                      arraylen(num), num,
	                      arraylen(denum), denum,
	                      RTF_DOUBLE);
	ck_assert(f != NULL);

	in = malloc(NS * NCH * sizeof(*in));
	out = malloc(NS * NCH * sizeof(*out));

	for (i = 0; i < NS * NCH; i++)
		in[i] = (double) rand();

	for (i = 0; i < NS * NCH; i++)
		out[i] = NANd;

	rtf_filter(f, in, out, NS);

	/* Smoke test:
	 * - no check on output values.
	 * - only ensure they are valid numbers */
	for (i = 0; i < NS * NCH; i++)
		ck_assert(isfinite(out[i]));

	rtf_destroy_filter(f);
	free(out);
	free(in);
}
END_TEST


START_TEST(test_rtfilter_flat_double)
{
	int i;
	double * in, * out;
	struct rtf_filter * f;
	double num[3] = {0., 1., 2.};
	double denum[4] = {3., 4., 5., 6.};

	f = rtf_create_filter(NCH, RTF_DOUBLE,
	                      arraylen(num), num,
	                      arraylen(denum), denum,
	                      RTF_DOUBLE);
	ck_assert(f != NULL);

	in = malloc(NS * NCH * sizeof(*in));
	out = malloc(NS * NCH * sizeof(*out));

	for (i = 0; i < NS * NCH; i++)
		in[i] = 0.;

	for (i = 0; i < NS * NCH; i++)
		out[i] = NANd;

	rtf_filter(f, in, out, NS);

	/* Smoke test:
	 * - no check on output values.
	 * - only ensure they are valid numbers */
	for (i = 0; i < NS * NCH; i++)
		ck_assert(out[i] == 0.);

	rtf_destroy_filter(f);
	free(out);
	free(in);
}
END_TEST


START_TEST(test_rtfilter_smoke_complex_float)
{
	int i;
	complex float * in, * out;
	struct rtf_filter * f;
	double num[3] = {0., 1., 2.};
	double denum[4] = {3., 4., 5., 6.};

	f = rtf_create_filter(NCH, RTF_CFLOAT,
	                      arraylen(num), num,
	                      arraylen(denum), denum,
	                      RTF_DOUBLE);
	ck_assert(f != NULL);

	in = malloc(NS * NCH * sizeof(*in));
	out = malloc(NS * NCH * sizeof(*out));

	for (i = 0; i < NS * NCH; i++)
		in[i] = rand() + I * rand();

	for (i = 0; i < NS * NCH; i++)
		out[i] = NANcf;

	rtf_filter(f, in, out, NS);

	/* Smoke test:
	 * - no check on output values.
	 * - only ensure they are valid numbers */
	for (i = 0; i < NS * NCH; i++) {
		ck_assert(finite_cf(out[i]));
	}

	rtf_destroy_filter(f);
	free(out);
	free(in);
}
END_TEST


START_TEST(test_rtfilter_flat_complex_float)
{
	int i;
	complex float * in, * out;
	struct rtf_filter * f;
	double num[3] = {0., 1., 2.};
	double denum[4] = {3., 4., 5., 6.};

	f = rtf_create_filter(NCH, RTF_CFLOAT,
	                      arraylen(num), num,
	                      arraylen(denum), denum,
	                      RTF_DOUBLE);
	ck_assert(f != NULL);

	in = malloc(NS * NCH * sizeof(*in));
	out = malloc(NS * NCH * sizeof(*out));

	for (i = 0; i < NS * NCH; i++)
		in[i] = 0.;

	for (i = 0; i < NS * NCH; i++)
		out[i] = NANcf;

	rtf_filter(f, in, out, NS);

	/* Smoke test:
	 * - no check on output values.
	 * - only ensure they are valid numbers */
	for (i = 0; i < NS * NCH; i++)
		ck_assert(out[i] == 0.);

	rtf_destroy_filter(f);
	free(out);
	free(in);
}
END_TEST


START_TEST(test_rtfilter_smoke_complex_double)
{
	int i;
	complex double * in, * out;
	struct rtf_filter * f;
	double num[3] = {0., 1., 2.};
	double denum[4] = {3., 4., 5., 6.};

	f = rtf_create_filter(NCH, RTF_CDOUBLE,
	                      arraylen(num), num,
	                      arraylen(denum), denum,
	                      RTF_DOUBLE);
	ck_assert(f != NULL);

	in = malloc(NS * NCH * sizeof(*in));
	out = malloc(NS * NCH * sizeof(*out));

	for (i = 0; i < NS * NCH; i++)
		in[i] = rand() + I * rand();

	for (i = 0; i < NS * NCH; i++)
		out[i] = NANcd;

	rtf_filter(f, in, out, NS);

	/* Smoke test:
	 * - no check on output values.
	 * - only ensure they are valid numbers */
	for (i = 0; i < NS * NCH; i++) {
		ck_assert(finite_cd(out[i]));
	}

	rtf_destroy_filter(f);
	free(out);
	free(in);
}
END_TEST


START_TEST(test_rtfilter_flat_complex_double)
{
	int i;
	complex double * in, * out;
	struct rtf_filter * f;
	double num[3] = {0., 1., 2.};
	double denum[4] = {3., 4., 5., 6.};

	f = rtf_create_filter(NCH, RTF_CDOUBLE,
	                      arraylen(num), num,
	                      arraylen(denum), denum,
	                      RTF_DOUBLE);
	ck_assert(f != NULL);

	in = malloc(NS * NCH * sizeof(*in));
	out = malloc(NS * NCH * sizeof(*out));

	for (i = 0; i < NS * NCH; i++)
		in[i] = 0.;

	for (i = 0; i < NS * NCH; i++)
		out[i] = NANcd;

	rtf_filter(f, in, out, NS);

	/* Smoke test:
	 * - no check on output values.
	 * - only ensure they are valid numbers */
	for (i = 0; i < NS * NCH; i++)
		ck_assert(out[i] == 0.);

	rtf_destroy_filter(f);
	free(out);
	free(in);
}
END_TEST


START_TEST(test_complex_rtfilter_smoke_complex_float)
{
	int i;
	complex float * in, * out;
	struct rtf_filter * f;
	complex double num[3] = {0., 1., 2.};
	complex double denum[4] = {3., 4., 5., 6.};

	f = rtf_create_filter(NCH, RTF_CFLOAT,
	                      arraylen(num), num,
	                      arraylen(denum), denum,
	                      RTF_CDOUBLE);
	ck_assert(f != NULL);

	in = malloc(NS * NCH * sizeof(*in));
	out = malloc(NS * NCH * sizeof(*out));

	for (i = 0; i < NS * NCH; i++)
		in[i] = (complex float) rand();

	for (i = 0; i < NS * NCH; i++)
		out[i] = NANcf;

	rtf_filter(f, in, out, NS);

	/* Smoke test:
	 * - no check on output values.
	 * - only ensure they are valid numbers */
	for (i = 0; i < NS * NCH; i++) {
		ck_assert(finite_cf(out[i]));
	}

	rtf_destroy_filter(f);
	free(out);
	free(in);
}
END_TEST


START_TEST(test_complex_rtfilter_flat_complex_float)
{
	int i;
	complex float * in, * out;
	struct rtf_filter * f;
	complex double num[3] = {0., 1., 2.};
	complex double denum[4] = {3., 4., 5., 6.};

	f = rtf_create_filter(NCH, RTF_CFLOAT,
	                      arraylen(num), num,
	                      arraylen(denum), denum,
	                      RTF_CDOUBLE);
	ck_assert(f != NULL);

	in = malloc(NS * NCH * sizeof(*in));
	out = malloc(NS * NCH * sizeof(*out));

	for (i = 0; i < NS * NCH; i++)
		in[i] = 0.;

	for (i = 0; i < NS * NCH; i++)
		out[i] = NANcf;

	rtf_filter(f, in, out, NS);

	/* Smoke test:
	 * - no check on output values.
	 * - only ensure they are valid numbers */
	for (i = 0; i < NS * NCH; i++)
		ck_assert(out[i] == 0.);

	rtf_destroy_filter(f);
	free(out);
	free(in);
}
END_TEST


START_TEST(test_complex_rtfilter_smoke_complex_double)
{
	int i;
	complex double * in, * out;
	struct rtf_filter * f;
	complex double num[3] = {0., 1., 2.};
	complex double denum[4] = {3., 4., 5., 6.};

	f = rtf_create_filter(NCH, RTF_CDOUBLE,
	                      arraylen(num), num,
	                      arraylen(denum), denum,
	                      RTF_CDOUBLE);
	ck_assert(f != NULL);

	in = malloc(NS * NCH * sizeof(*in));
	out = malloc(NS * NCH * sizeof(*out));

	for (i = 0; i < NS * NCH; i++)
		in[i] = (complex float) rand();

	for (i = 0; i < NS * NCH; i++)
		out[i] = NANcd;

	rtf_filter(f, in, out, NS);

	/* Smoke test:
	 * - no check on output values.
	 * - only ensure they are valid numbers */
	for (i = 0; i < NS * NCH; i++) {
		ck_assert(finite_cd(out[i]));
	}

	rtf_destroy_filter(f);
	free(out);
	free(in);
}
END_TEST


START_TEST(test_complex_rtfilter_flat_complex_double)
{
	int i;
	complex double * in, * out;
	struct rtf_filter * f;
	complex double num[3] = {0., 1., 2.};
	complex double denum[4] = {3., 4., 5., 6.};

	f = rtf_create_filter(NCH, RTF_CDOUBLE,
	                      arraylen(num), num,
	                      arraylen(denum), denum,
	                      RTF_CDOUBLE);
	ck_assert(f != NULL);

	in = malloc(NS * NCH * sizeof(*in));
	out = malloc(NS * NCH * sizeof(*out));

	for (i = 0; i < NS * NCH; i++)
		in[i] = 0.;

	for (i = 0; i < NS * NCH; i++)
		out[i] = NANcd;

	rtf_filter(f, in, out, NS);

	/* Smoke test:
	 * - no check on output values.
	 * - only ensure they are valid numbers */
	for (i = 0; i < NS * NCH; i++)
		ck_assert(out[i] == 0.);

	rtf_destroy_filter(f);
	free(out);
	free(in);
}
END_TEST




TCase* create_rtfilter_tcase(void)
{
	TCase * tc = tcase_create("rtfilter");

	/* smoke tests on float data with real coeffs */
	tcase_add_test(tc, test_rtfilter_flat_float);
	tcase_add_test(tc, test_rtfilter_smoke_float);

	/* smoke tests on double data with real coeffs */
	tcase_add_test(tc, test_rtfilter_flat_double);
	tcase_add_test(tc, test_rtfilter_smoke_double);

	/* smoke tests on complex float data with real coeffs */
	tcase_add_test(tc, test_rtfilter_flat_complex_float);
	tcase_add_test(tc, test_rtfilter_smoke_complex_float);

	/* smoke tests on complex double data with real coeffs */
	tcase_add_test(tc, test_rtfilter_flat_complex_double);
	tcase_add_test(tc, test_rtfilter_smoke_complex_double);

	/* smoke tests on complex float data with complex coeffs */
	tcase_add_test(tc, test_complex_rtfilter_flat_complex_float);
	tcase_add_test(tc, test_complex_rtfilter_smoke_complex_float);

	/* smoke tests on complex double data with complex coeffs */
	tcase_add_test(tc, test_complex_rtfilter_flat_complex_double);
	tcase_add_test(tc, test_complex_rtfilter_smoke_complex_double);

	return tc;
}
