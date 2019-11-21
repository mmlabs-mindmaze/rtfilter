#!/bin/bash

set -e

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
for std in c99 c11 c17 gnu99 gnu11 gnu17
do
	echo "Test C language standard : $std"
	all_included | gcc -std=$std -x c - -I"$includedir" -Werror -Wall -pedantic
done

# also test with clang if available
if [ -x "$(which clang)" ] ; then
	clang --version
	all_included | clang -x c - -I"$includedir" -Weverything \
		-Wno-documentation-unknown-command  # silence doxygen-related warnings
fi

# test headers for C++ compatibility
for std in c++11 c++14 c++17 gnu++11 gnu++14 gnu++17
do
	echo "Test C++ language standard : $std"
	all_included | gcc -std=$std -x c++ - -I"$includedir" -Werror -Wall -pedantic
done
