#include <check.h>
#include <stdlib.h>

#include "testcases.h"


static Suite*
api_test_suite(void)
{
	Suite * s = suite_create("api");

	suite_add_tcase(s, create_rtfilter_tcase());

	return s;
}

int main(void)
{
	Suite * s;
	SRunner * sr;
	int exitcode = EXIT_SUCCESS;

	s = api_test_suite();
	sr = srunner_create(s);
#ifdef CHECK_SUPPORT_TAP
	srunner_set_tap(sr, "-");
#endif

	srunner_run_all(sr, CK_ENV);

	if (srunner_ntests_failed(sr) != 0)
		exitcode = EXIT_FAILURE;

	srunner_free(sr);

	return exitcode;
}
