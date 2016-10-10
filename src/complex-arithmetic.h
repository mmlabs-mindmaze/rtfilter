/*
    Copyright (C) 2016 Nicolas Bourdaud <nicolas.bourdaud@mindmaze.ch>

    This file is part of the rtfilter library

    The rtfilter library is free software: you can redistribute it and/or
    modify it under the terms of the version 3 of the GNU Lesser General
    Public License as published by the Free Software Foundation.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef COMPLEX_ARITHMETIC_H
#define COMPLEX_ARITHMETIC_H

typedef struct 
{
	float real;
	float imag;
} complex_float_t;

typedef struct 
{
	double real;
	double imag;
} complex_double_t;


static inline
complex_float_t cadd_f(complex_float_t a, complex_float_t b)
{
	complex_float_t r;

	r.real = a.real + b.real;
	r.imag = a.imag + b.imag;

	return r;
}

static inline
complex_float_t csub_f(complex_float_t a, complex_float_t b)
{
	complex_float_t r;

	r.real = a.real - b.real;
	r.imag = a.imag - b.imag;

	return r;
}

static inline
complex_float_t cmul_f(complex_float_t a, complex_float_t b)
{
	complex_float_t r;

	r.real = a.real*b.real - a.imag*b.imag;
	r.imag = a.real*b.imag + a.imag*b.real;

	return r;
}


static inline
complex_float_t cinv_f(complex_float_t a)
{
	complex_float_t r;
	float norm;

	norm = (a.real*a.real) + (a.imag*a.imag);

	r.real = a.real / norm;
	r.imag = -a.imag / norm;

	return r;
}

static inline
complex_float_t cdiv_f(complex_float_t a, complex_float_t b)
{
	return cmul_f(a, cinv_f(b));
}

static inline
complex_float_t cscale_f(complex_float_t a, float b)
{
	complex_float_t r;

	r.real = a.real*b;
	r.imag = a.imag*b;

	return r;
}

static inline
float creal_f(complex_float_t a)
{
	return a.real;
}

static inline
float cimag_f(complex_float_t a)
{
	return a.imag;
}


static inline
complex_double_t cadd_d(complex_double_t a, complex_double_t b)
{
	complex_double_t r;

	r.real = a.real + b.real;
	r.imag = a.imag + b.imag;

	return r;
}

static inline
complex_double_t csub_d(complex_double_t a, complex_double_t b)
{
	complex_double_t r;

	r.real = a.real - b.real;
	r.imag = a.imag - b.imag;

	return r;
}

static inline
complex_double_t cmul_d(complex_double_t a, complex_double_t b)
{
	complex_double_t r;

	r.real = a.real*b.real - a.imag*b.imag;
	r.imag = a.real*b.imag + a.imag*b.real;

	return r;
}

static inline
complex_double_t cinv_d(complex_double_t a)
{
	complex_double_t r;
	double norm;

	norm = (a.real*a.real) + (a.imag*a.imag);

	r.real = a.real / norm;
	r.imag = -a.imag / norm;

	return r;
}

static inline
complex_double_t cdiv_d(complex_double_t a, complex_double_t b)
{
	return cmul_d(a, cinv_d(b));
}

static inline
complex_double_t cscale_d(complex_double_t a, double b)
{
	complex_double_t r;

	r.real = a.real*b;
	r.imag = a.imag*b;

	return r;
}

static inline
double creal_d(complex_double_t a)
{
	return a.real;
}

static inline
double cimag_d(complex_double_t a)
{
	return a.imag;
}




#endif
