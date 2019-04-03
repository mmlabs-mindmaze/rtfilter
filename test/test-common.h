#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <math.h>

#define arraylen(v) ((int) (sizeof(v) / sizeof(*v)))

#ifndef NAN
#define NAN __builtin_nanf("")
#endif

static inline
double db(double x)
{
	return 10 * log(x);
}


static inline
double energy(float const * signal, int nch, int ns)
{
	double energy;
	int i, j;

	energy = 0.;
	for (i = 0; i < nch; i++) {
		for (j = 0; j < ns; j++) {
			energy = signal[j * nch + i] * signal[j * nch + i];
		}
	}

	return energy;
}


/**
 * create_signal() - create signal at given frequency
 * @signal: buffer to fill
 * @nch: number of channels
 * @ns: number of samples
 * @freq: requested frequency
 * @fs: sampling frequency
 */
static inline
void create_signal(float * signal, int nch, int ns, double freq, double fs)
{
	int i, j;

	for (i = 0; i < nch; i++) {
		for (j = 0; j < ns; j++) {
			signal[j * nch + i] = sinf(2.f * M_PI * freq * j / fs);
		}
	}
}

static inline
void fill_signal(float * signal, int nch, int ns, float value)
{
	int i;

	for (i = 0; i < ns * nch; i++)
		signal[i] = value;
}


void smoke_test(struct rtf_filter * f, int nch, int ns);

void flat_test(struct rtf_filter * f, int nch, int ns);

#endif /* TEST_COMMON_H */
