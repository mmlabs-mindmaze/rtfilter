#!/bin/sh

LOGFILE=testcf.log

if { 
    cmpwmatlab* -d 0 -p 2 -c 15 \
 && cmpwmatlab* -d 0 -p 2 -c 16 \
 && cmpwmatlab* -d 2 -p 0 -c 16 \
 && cmpwmatlab* -d 2 -p 0 -c 15 \
 && cmpwmatlab* -d 2 -p 2 -c 16 \
 && cmpwmatlab* -d 2 -p 2 -c 15
} > $LOGFILE
then
	rm $LOGFILE
	stty echo
	exit 0
else
	stty echo
	exit 1
fi
