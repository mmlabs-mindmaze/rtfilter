#!/bin/sh

set -ex

includedir=$1
shift 1

randomize_header_list()
{
	for header in $@ ; do
		echo $header
	done | sort -R
}
all_headers=$(randomize_header_list $@)

all_included()
{
	for header in $all_headers ; do
		echo "#include \"$(basename $header)\""
	done

	echo "int main(void) { return 0; }"
}

# dump compilers versions
gcc --version

# dump generated file
all_included

# test headers for compliance with most warnings
all_included | gcc -x c - -I"$includedir" -Werror -Wall -pedantic

# also test with clang if available
if [ -x "$(which clang)" ] ; then
	clang --version
	all_included | clang -x c - -I"$includedir" -Weverything
fi

# test headers for C++ compatibility
all_included | gcc -x c++ - -I"$includedir" -Werror -Wall -pedantic
