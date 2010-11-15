#!/bin/sh

if ! downsample* -t 0 -c 16 >test.log; then
	return 1
fi

if ! downsample* -t 1 -c 16 >test.log; then
	return 1
fi

rm test.log
