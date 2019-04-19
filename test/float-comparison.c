/*
 * @mindmaze_header@
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

