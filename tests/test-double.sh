#!/bin/sh

LOGFILE=testd.log

if {
    cmpwmatlab* -d 1 -p 1 -c 16 \
 && cmpwmatlab* -d 1 -p 1 -c 15
} > $LOGFILE
then
	rm $LOGFILE
	exit 0
else
	exit 1
fi
