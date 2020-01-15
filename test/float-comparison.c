/*
 *  Copyright (C) 2018-2020 Mindmaze Holdings SA
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

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "float-comparison.h"

#define NB_BITS_THOLD_MANTISSA 10

typedef union {
	float f;
	struct {
		uint32_t mantisa_exp : 31;
		uint32_t sign : 1;
	} parts;
} parse_float;

/**
 * compare_floats() - Compare the distance between two floats
 * @infloat1:  first input float to compare
 * @infloat2:  second input float to compare
 *
 * Return: 0 if the distance is lower than thold, -1 otherwise.
 */
int compare_floats(float infloat1, float infloat2)
{
	int diff;
	int thold = (2 << NB_BITS_THOLD_MANTISSA) -1;

	parse_float parse_input1;
	parse_float parse_input2;

	parse_input1.f = infloat1;
	parse_input2.f = infloat2;

	/* If the input is negative apply 2's complement */
	if (parse_input1.parts.sign)
		parse_input1.parts.mantisa_exp =
			~parse_input1.parts.mantisa_exp;

	if (parse_input2.parts.sign)
		parse_input2.parts.mantisa_exp =
			~parse_input2.parts.mantisa_exp;

	diff = parse_input1.parts.mantisa_exp - parse_input2.parts.mantisa_exp;
	if (abs(diff) > thold)
		return -1;

	return 0;
}

