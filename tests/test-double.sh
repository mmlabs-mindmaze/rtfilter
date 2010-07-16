#!/bin/sh

if ! cmpwmatlab -d 1 -p 1 -c 16 ; then
	return 1
fi

if ! cmpwmatlab -d 1 -p 1 -c 15 ; then
	return 1
fi
