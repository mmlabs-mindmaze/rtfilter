/*
 * @mindmaze_header@
 */
#include <check.h>
#include <math.h>
#include <rtfilter.h>
#include <stdlib.h>

#include "float-comparison.h"
#include "rtfilter.h"
#include "testcases.h"
#include "test-common.h"

#define NCH 4
#define NS 128
#define FS 2000


static
struct rtf_filter* create_lowpass(int nch, double fs, int order, double lowcut,
                                  int data_type)
{
	double fc;
	struct rtf_filter * f;

	ck_assert(nch > 0);
	ck_assert(lowcut > 0.);
	ck_assert(fs > lowcut);
	ck_assert(order > 0);

	fc = lowcut / fs;
	f = rtf_create_butterworth(nch, data_type, fc, order, 0);
	ck_assert(f != NULL);

	return f;
}

START_TEST(test_lowpass_coeffs)
{
	struct rtf_filter * f;

	f = create_lowpass(NCH, FS, 2, 50., RTF_FLOAT);
	ck_assert(f != NULL);

	struct rtf_coeffs * _coeffs = rtf_get_coeffs(f);
	struct real_coeffs * coeffs = &_coeffs->coeffs.real;

	/* compare generated coeffs against values computed by matlab */
	ck_assert(!_coeffs->is_complex);

	/* test generated num */
	ck_assert_int_eq(coeffs->num_len, 3);
	ck_assert(compare_floats(coeffs->num[0], 0.005542717408) == 0);
	ck_assert(compare_floats(coeffs->num[1], 0.011085434817) == 0);
	ck_assert(compare_floats(coeffs->num[2], 0.0055427174084) == 0);

	/* test generated denum */
	ck_assert(coeffs->denum[0] == 1.);  /* exact value */
	ck_assert_int_eq(coeffs->denum_len, 3);
	ck_assert(compare_floats(coeffs->denum[1], -1.778631806374) == 0);
	ck_assert(compare_floats(coeffs->denum[2], 0.8008026480675) == 0);

	rtf_destroy_filter(f);
}
END_TEST

START_TEST(test_lowpass_smoke)
{
	struct rtf_filter * f;

	f = create_lowpass(NCH, FS, 2, 50., RTF_FLOAT);
	ck_assert(f != NULL);

	smoke_test(f, NCH, NS);

	rtf_destroy_filter(f);
}
END_TEST

START_TEST(test_lowpass_flat)
{
	struct rtf_filter * f;

	f = create_lowpass(NCH, FS, 2, 50., RTF_FLOAT);
	ck_assert(f != NULL);

	flat_test(f, NCH, NS);

	rtf_destroy_filter(f);
}
END_TEST

TCase* create_lowpass_tcase(void)
{
	TCase * tc;

	tc = tcase_create("lowpass");
	tcase_add_test(tc, test_lowpass_coeffs);
	tcase_add_test(tc, test_lowpass_flat);
	tcase_add_test(tc, test_lowpass_smoke);
	return tc;
}
