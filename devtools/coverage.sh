#!/bin/sh

set -e

COVERAGE_CFLAGS="-O0 -fprofile-arcs -ftest-coverage"

test_coverage()
{
	TESTING_COVERAGE=false make clean

	mkdir -p $covdir

	lcov --directory . --zerocounters -q
	make check CFLAGS="$COVERAGE_CFLAGS"
	lcov --compat-libtool --directory . --capture --output-file $covdir/app.info

	TESTING_COVERAGE=true make clean

	genhtml -o $covdir/ $covdir/app.info
	set -x
	firefox $covdir/index.html &
	{ set +x; } 2> /dev/null
}


print_usage()
{
	echo "Usage:"
	echo "\t$0 run [outdir]" 2>&1
	echo "\t$0 clean [outdir]" 2>&1
}


if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
	echo "Illegal number of parameters" 2>&1
	print_usage
	exit 1
fi

action=$1
covdir=${2:-coverage-results}

case $action in
	"run")
		test_coverage
		;;

	"clean")
		test "x$TESTING_COVERAGE" = "xtrue" || rm -rf $covdir
		;;

	*)
		echo "Illegal action: $action" 2>&1
		print_usage
		exit 1
esac

