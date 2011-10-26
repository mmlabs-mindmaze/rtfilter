#!/bin/sh

LOGFILE=testf.log

if {
     cmpwmatlab* -d 0 -p 0 -c 16 \
  && cmpwmatlab* -d 0 -p 0 -c 15
} > $LOGFILE
then
	rm $LOGFILE
	stty echo
	exit 0
else
	stty echo
	exit 1
fi
