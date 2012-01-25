/*
    Copyright (C) 2011-2012 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

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
#ifndef PROBESIMD_H
#define PROBESIMD_H

#if HAVE_CPUID
#include <cpuid.h>

static inline 
int cputest_sse(void)
{
	unsigned int eax, ebx, ecx, edx;
	if (__get_cpuid(1, &eax, &ebx, &ecx, &edx) && (edx & bit_SSE))
		return 1;
	return 0;
}

static inline 
int cputest_sse2(void)
{
	unsigned int eax, ebx, ecx, edx;
	if (__get_cpuid(1, &eax, &ebx, &ecx, &edx) && (edx & bit_SSE2))
		return 1;
	return 0;
}

static inline 
int cputest_sse3(void)
{
	unsigned int eax, ebx, ecx, edx;
	if (__get_cpuid(1, &eax, &ebx, &ecx, &edx) && (ecx & bit_SSE3))
		return 1;
	return 0;
}

#else /* HAVE_CPUID */

static inline 
int cputest_sse(void)
{
#ifdef __SSE__
	return 1;
#else
	return 0;
#endif
}

static inline 
int cputest_sse2(void)
{
#ifdef __SSE2__
	return 1;
#else
	return 0;
#endif
}

static inline 
int cputest_sse3(void)
{
#ifdef __SSE3__
	return 1;
#else
	return 0;
#endif
}

#endif /* HAVE_CPUID */


#endif /* PROBESIMD_H */

