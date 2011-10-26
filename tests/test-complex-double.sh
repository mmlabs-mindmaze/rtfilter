#!/bin/sh

LOGFILE=testcd.log

if {
    cmpwmatlab* -d 1 -p 3 -c 15 \
 && cmpwmatlab* -d 1 -p 3 -c 16 \
 && cmpwmatlab* -d 3 -p 1 -c 15 \
 && cmpwmatlab* -d 3 -p 1 -c 16 \
 && cmpwmatlab* -d 3 -p 3 -c 15 \
 && cmpwmatlab* -d 3 -p 3 -c 16
} > $LOGFILE 
then
	rm $LOGFILE
	stty echo
	exit 0
else
	stty echo
	exit 1
fi
