#!/bin/sh

if ! cmpwmatlab -d 0 -p 0 -c 16 ; then
	return 1
fi

if ! cmpwmatlab -d 0 -p 0 -c 15 ; then
	return 1
fi
