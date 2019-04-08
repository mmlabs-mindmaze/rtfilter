#include <check.h>
#include <stdlib.h>

#include "rtfilter.h"
#include "test-common.h"


static inline
void create_random_signal(float * signal, int nch, int ns)
{
	int i;

	for (i = 0; i < ns * nch; i++)
		signal[i] = (float) rand();
}


void smoke_test(struct rtf_filter * f, int nch, int ns)
{
	int i;
	float * in, * out;

	ck_assert(f != NULL);

	in = malloc(ns * nch * sizeof(*in));
	out = malloc(ns * nch * sizeof(*out));

	create_random_signal(in, nch, ns);
	fill_signal(out, nch, ns, NAN);

	rtf_filter(f, in, out, ns);

	/* Smoke test:
	 * - no check on output values.
	 * - only ensure they are valid numbers */
	for (i = 0; i < ns * nch; i++)
		ck_assert(isfinite(out[i]));

	free(out);
	free(in);
}

void flat_test(struct rtf_filter * f, int nch, int ns)
{
	int i;
	float * in, * out;

	ck_assert(f != NULL);

	in = malloc(ns * nch * sizeof(*in));
	out = malloc(ns * nch * sizeof(*out));

	fill_signal(in, nch, ns, 0.f);
	fill_signal(out, nch, ns, NAN);

	rtf_filter(f, in, out, ns);

	/* should return 0 on flat input */
	for (i = 0; i < ns * nch; i++)
		ck_assert(out[i] == 0.f);

	free(out);
	free(in);
}
