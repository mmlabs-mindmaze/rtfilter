/*
 *  Copyright (C) 2011 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>
 *
 *  This file is part of the rtfilter library
 *
 *  The rtfilter library is free software: you can redistribute it and/or
 *  modify it under the terms of the version 3 of the GNU Lesser General
 *  Public License as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FILTER_FUNCS_H
#define FILTER_FUNCS_H

#include "filter-internal.h"

#define DECLARE_FILTFUNC(fnname)\
	LOCAL_FN void fnname(struct rtf_filter* filt,\
	                     const void* restrict in, void* restrict out,\
	                     int nsamples)


// Normal version
DECLARE_FILTFUNC(filter_f_noop);
DECLARE_FILTFUNC(filter_d_noop);
DECLARE_FILTFUNC(filter_fcf_noop);
DECLARE_FILTFUNC(filter_cf_noop);
DECLARE_FILTFUNC(filter_dcd_noop);
DECLARE_FILTFUNC(filter_cd_noop);

// SSE version
DECLARE_FILTFUNC(filter_f_sse);

// SSE2 version
DECLARE_FILTFUNC(filter_d_sse2);

// SSE3 version
DECLARE_FILTFUNC(filter_fcf_sse3);
DECLARE_FILTFUNC(filter_cf_sse3);
DECLARE_FILTFUNC(filter_dcd_sse3);
DECLARE_FILTFUNC(filter_cd_sse3);

#endif //FILTER_FUNCS_H

